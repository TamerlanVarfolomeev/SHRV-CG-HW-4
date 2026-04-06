#define NOMINMAX
#include "src/Core/Application.h"
#include "src/Graphics/ObjLoader.h"
#include "src/Graphics/Shader.h"
#include "src/Graphics/Mesh.h"
#include "src/Components/MeshRenderer.h"
#include "src/Components/RigidBody.h"
#include "src/Components/BoxCollider.h"
#include "src/Components/MeshCollider.h"
#include "src/Components/PlayerController.h"
#include "src/Components/SphereCollider.h"
#include "src/Graphics/Texture.h"
#include <vector>
#include <memory>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

// ---------------------------------------------------------------------------
// Список спавнящихся объектов — добавляй новые строки сюда
// ---------------------------------------------------------------------------
struct SpawnDef
{
    const char*   objPath;         // путь к .obj файлу (относительно рабочей директории)
    rp3d::Vector3 boxHalfExtents;  // полу-размеры box-коллайдера
    float         mass;            // масса объекта
    float         eatVolume;       // вклад в объём катамари при поглощении
};

static const SpawnDef SPAWNABLE[] =
{
    //  objPath                                        halfExtents (x,y,z)           mass   eatVolume
    { "Assets/bazooka.obj",                          { 0.13f, 0.22f, 0.82f },       2.0f,  0.03f },
    { "Assets/barrel_large.obj",                     { 0.75f, 0.85f, 0.75f },       8.0f,  0.15f },
    { "Assets/barrel_small_stack.obj",               { 0.75f, 0.72f, 0.40f },       5.0f,  0.10f },
    { "Assets/bed_decorated.obj",                    { 1.05f, 0.68f, 1.22f },      20.0f,  0.50f },
    { "Assets/box_large.obj",                        { 0.60f, 0.60f, 0.60f },       6.0f,  0.12f },
    { "Assets/box_small_decorated.obj",              { 0.60f, 0.60f, 0.60f },       4.0f,  0.10f },
    { "Assets/chest_gold_lid.obj",                   { 0.68f, 0.38f, 0.58f },       7.0f,  0.13f },
    { "Assets/column.obj",                           { 0.28f, 0.56f, 0.28f },       4.0f,  0.05f },
    { "Assets/keg_decorated.obj",                    { 1.40f, 0.82f, 0.80f },      15.0f,  0.35f },
    { "Assets/pillar_decorated.obj",                 { 0.89f, 1.60f, 0.68f },      18.0f,  0.45f },
    { "Assets/table_long_decorated_C.obj",           { 0.80f, 1.10f, 1.60f },      15.0f,  0.45f },
    { "Assets/table_long_tablecloth_decorated_A.obj",{ 0.82f, 0.75f, 1.60f },      12.0f,  0.40f },
    { "Assets/table_medium_tablecloth.obj",          { 0.80f, 0.40f, 0.80f },       8.0f,  0.20f },
    { "Assets/table_small_decorated_A.obj",          { 0.42f, 0.65f, 0.49f },       5.0f,  0.12f },
    { "Assets/table_small_decorated_B.obj",          { 0.41f, 0.87f, 0.40f },       5.0f,  0.12f },
    { "Assets/trunk_large_B.obj",                    { 0.60f, 0.40f, 0.52f },       8.0f,  0.15f },
    // Добавляй сюда новые объекты:
    // { "Assets/myobject.obj",                      { 0.2f, 0.2f, 0.2f },          1.5f,  0.05f },
};
static const int SPAWNABLE_COUNT = (int)(sizeof(SPAWNABLE) / sizeof(SPAWNABLE[0]));

// ---------------------------------------------------------------------------

struct AttachedObject
{
    GameObject*              go;
    DirectX::XMFLOAT4X4     localMatrix; // трансформ объекта в локальном пространстве сферы
};

// ---------------------------------------------------------------------------
// Слушатель контактов RP3D — собирает тела, коснувшиеся игрока
// ---------------------------------------------------------------------------
class KatamariContactListener : public rp3d::EventListener
{
public:
    rp3d::RigidBody*                     playerBody = nullptr;
    std::vector<rp3d::RigidBody*>        pendingEats;
    std::unordered_set<rp3d::RigidBody*> eatenBodies; // уже съеденные (не трогать повторно)

    void onContact(const rp3d::CollisionCallback::CallbackData& data) override
    {
        for (uint32_t i = 0; i < data.getNbContactPairs(); i++)
        {
            auto pair = data.getContactPair(i);

            // Интересуют только новые контакты
            if (pair.getEventType() !=
                rp3d::CollisionCallback::ContactPair::EventType::ContactStart)
                continue;

            auto* b1 = static_cast<rp3d::RigidBody*>(pair.getBody1());
            auto* b2 = static_cast<rp3d::RigidBody*>(pair.getBody2());

            rp3d::RigidBody* hit = nullptr;
            if      (b1 == playerBody) hit = b2;
            else if (b2 == playerBody) hit = b1;

            if (!hit) continue;

            // Только динамические объекты — статический террейн игнорируем
            if (hit->getType() != rp3d::BodyType::DYNAMIC) continue;

            if (!eatenBodies.count(hit))
            {
                eatenBodies.insert(hit);
                pendingEats.push_back(hit);
            }
        }
    }
};

// ---------------------------------------------------------------------------

class MyApp : public Application
{
public:
    MyApp() : Application(1280, 720, L"Katamari Physics") {}

protected:
    void OnStart() override
    {
        ID3D11Device* dev = GetGfx().GetDevice();

        // --- Шейдер ---
        shader_ = std::make_unique<Shader>(dev,
            L"Shaders/Textured.hlsl",
            std::vector<InputElement>{
                { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
                { "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT },
                { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT    },
            });

        // --- Предзагрузка всех spawnable объектов ---
        spawnableSubmeshes_.resize(SPAWNABLE_COUNT);
        for (int i = 0; i < SPAWNABLE_COUNT; i++)
            spawnableSubmeshes_[i] = ObjLoader::Load(dev, SPAWNABLE[i].objPath);

        // --- Террейн ---
        terrainSubmeshes_ = ObjLoader::Load(dev, "Assets/terrain.obj");
        CreateTerrain(dev);

        // --- Игрок ---
        CreatePlayer(dev);

        // --- Регистрируем слушатель контактов ---
        contactListener_.playerBody = playerRb_->GetBody();
        GetPhysics().SetContactListener(&contactListener_);

        // --- Камера ---
        GetScene().camera->SetOrbitDistance(20.0f);

        spawnTimer_ = spawnInterval_;
    }

    void OnFixedUpdate(float) override
    {
        // 1. Камера следует за игроком
        if (playerController_)
            playerController_->FollowCamera();

        // 2. Обработка новых «поеданий» (накоплено во время physics_->Step)
        for (auto* body : contactListener_.pendingEats)
            ProcessEat(body);
        contactListener_.pendingEats.clear();

        // 3. Обновляем позиции прикреплённых объектов вместе со сферой
        DirectX::XMMATRIX sphereRT = GetSphereRT();
        for (auto& att : attachedObjects_)
        {
            DirectX::XMMATRIX worldMat =
                DirectX::XMLoadFloat4x4(&att.localMatrix) * sphereRT;
            att.go->transform.SetPhysicsMatrix(worldMat);
        }
    }

    void OnUpdate(float dt) override
    {
        spawnTimer_ -= dt;
        if (spawnTimer_ <= 0.0f)
        {
            spawnTimer_ = spawnInterval_;
            SpawnObject();
        }
    }

private:
    // -----------------------------------------------------------------------
    void CreateTerrain(ID3D11Device* dev)
    {
        if (terrainSubmeshes_.empty()) return;

        auto* go = GetScene().CreateObject("Terrain");
        go->transform.position = { 0.0f, 0.0f, 0.0f };
        go->transform.scale    = { 1.0f, 1.0f, 1.0f };
        go->transform.rotation = { 0.0f, 0.0f, 0.0f };

        for (auto& sm : terrainSubmeshes_)
        {
            auto* r = go->AddComponent<MeshRenderer>(
                dev, sm.mesh.get(), shader_.get(), sm.texture.get());
            r->material.albedoColor = sm.albedoColor;
        }

        go->AddComponent<RigidBody>(
            GetPhysics().GetWorld(),
            &GetPhysics().GetCommon(),
            PhysicsBodyType::Static);

        std::vector<DirectX::XMFLOAT3> allPositions;
        std::vector<uint32_t>          allIndices;
        uint32_t vertexOffset = 0;

        for (auto& sm : terrainSubmeshes_)
        {
            allPositions.insert(allPositions.end(),
                                sm.meshPositions.begin(), sm.meshPositions.end());
            for (uint32_t idx : sm.meshIndices)
                allIndices.push_back(idx + vertexOffset);
            vertexOffset += static_cast<uint32_t>(sm.meshPositions.size());
        }

        go->AddComponent<MeshCollider>(
            &GetPhysics().GetCommon(),
            std::move(allPositions),
            std::move(allIndices));
    }

    // -----------------------------------------------------------------------
    void CreatePlayer(ID3D11Device* dev)
    {
        gApp = this;

        auto* go = GetScene().CreateObject("Player");
        go->transform.position = { 0.0f, 2.0f, 0.0f };

        playerRb_ = go->AddComponent<RigidBody>(
            GetPhysics().GetWorld(),
            &GetPhysics().GetCommon(),
            PhysicsBodyType::Dynamic);
        playerRb_->mass           = 10.0f;
        playerRb_->linearDamping  = 0.5f;
        playerRb_->angularDamping = 0.3f;

        playerSc_ = go->AddComponent<SphereCollider>(
            &GetPhysics().GetCommon(), baseRadius_);

        // Процедурная сфера-меш
        playerMesh_ = std::make_unique<Mesh>(
            Mesh::CreateColorSphere(dev, baseRadius_, 8, 16));

        const int texSize = 64;
        std::vector<uint8_t> pixels(texSize * texSize * 4);
        for (int y = 0; y < texSize; y++)
            for (int x = 0; x < texSize; x++)
            {
                int idx = (y * texSize + x) * 4;
                int bx = x / 8, by = y / 8;
                pixels[idx + 0] = (bx * 41 + by * 17) % 256;
                pixels[idx + 1] = (bx * 23 + by * 59) % 256;
                pixels[idx + 2] = (bx * 67 + by * 31) % 256;
                pixels[idx + 3] = 255;
            }

        playerTexture_ = std::unique_ptr<Texture>(
            Texture::CreateFromPixels(dev, texSize, texSize, pixels.data()));

        go->AddComponent<MeshRenderer>(
            dev, playerMesh_.get(), shader_.get(), playerTexture_.get());

        playerController_ = go->AddComponent<PlayerController>(7.0f, baseRadius_);
        playerGo_ = go;
    }

    // -----------------------------------------------------------------------
    void SpawnObject()
    {
        if (SPAWNABLE_COUNT == 0) return;

        int defIdx = rand() % SPAWNABLE_COUNT;
        auto& def  = SPAWNABLE[defIdx];
        auto& subs = spawnableSubmeshes_[defIdx];
        if (subs.empty()) return;

        ID3D11Device* dev = GetGfx().GetDevice();

        float x  = ((float)(rand() % 480) - 240.0f) * 0.1f;
        float z  = ((float)(rand() % 480) - 240.0f) * 0.1f;
        float ry = (float)(rand() % 360);

        auto* go = GetScene().CreateObject("SpawnObj");
        go->transform.position = { x, 12.0f, z };
        go->transform.rotation = { 0.0f, ry, 0.0f };

        auto* rb = go->AddComponent<RigidBody>(
            GetPhysics().GetWorld(),
            &GetPhysics().GetCommon(),
            PhysicsBodyType::Dynamic);
        rb->mass = def.mass;

        go->AddComponent<BoxCollider>(
            &GetPhysics().GetCommon(), def.boxHalfExtents);

        for (auto& sm : subs)
        {
            auto* r = go->AddComponent<MeshRenderer>(
                dev, sm.mesh.get(), shader_.get(), sm.texture.get());
            r->material.albedoColor = sm.albedoColor;
        }

        // Сохраняем связь body → GO, объём и характерный размер объекта
        auto* body = rb->GetBody();
        bodyToGo_[body]     = go;
        bodyToVolume_[body] = def.eatVolume;

        // Характерный размер = наибольший из полу-экстентов коллайдера
        float sx = def.boxHalfExtents.x, sy = def.boxHalfExtents.y, sz = def.boxHalfExtents.z;
        bodyToSize_[body] = sx > sy ? (sx > sz ? sx : sz) : (sy > sz ? sy : sz);
    }

    // -----------------------------------------------------------------------
    // Вызывается ПОСЛЕ physics_->Step — безопасно изменять физику
    void ProcessEat(rp3d::RigidBody* body)
    {
        auto goIt = bodyToGo_.find(body);
        if (goIt == bodyToGo_.end()) return; // неизвестное тело (террейн и т.п.)

        // Объект можно съесть, только если игрок достаточно вырос.
        // Порог: характерный размер объекта не должен превышать 1.8× текущий радиус сферы.
        auto sizeIt = bodyToSize_.find(body);
        if (sizeIt != bodyToSize_.end() && sizeIt->second > currentRadius_ * 1.8f)
        {
            // Убираем из очереди «съеденных», чтобы при следующем касании попробовать снова
            contactListener_.eatenBodies.erase(body);
            return;
        }

        GameObject* go = goIt->second;

        // 1. Выключаем физику объекта
        auto* rb = go->GetComponent<RigidBody>();
        if (rb) rb->SetSimulated(false);

        // 2. Вычисляем localMatrix = objectWorld * invSphereRT
        //    Захватываем текущий мировой трансформ объекта (уже синхронизирован RigidBody::FixedUpdate)
        DirectX::XMMATRIX sphereRT    = GetSphereRT();
        DirectX::XMMATRIX invSphereRT = DirectX::XMMatrixInverse(nullptr, sphereRT);
        DirectX::XMMATRIX objWorld    = go->transform.GetWorldMatrix();

        AttachedObject att;
        att.go = go;
        DirectX::XMStoreFloat4x4(&att.localMatrix, objWorld * invSphereRT);
        attachedObjects_.push_back(att);

        // 3. Рост сферы
        auto volIt = bodyToVolume_.find(body);
        if (volIt != bodyToVolume_.end())
            eatenVolume_ += volIt->second;

        UpdateSphereSize();
    }

    // -----------------------------------------------------------------------
    void UpdateSphereSize()
    {
        // Радиус растёт пропорционально кубическому корню от суммарного объёма
        float newRadius = baseRadius_ * std::pow(1.0f + eatenVolume_, 1.0f / 3.0f);
        currentRadius_ = newRadius;

        // Визуальный масштаб (S * R * T в RigidBody::FixedUpdate считывает scale)
        float s = newRadius / baseRadius_;
        playerGo_->transform.scale = { s, s, s };

        // Физический коллайдер
        playerSc_->SetRadius(newRadius);

        // Скорость в PlayerController тоже масштабируется
        if (playerController_)
            playerController_->SetRadius(newRadius);
    }

    // -----------------------------------------------------------------------
    // Матрица поворота + трансляция сферы (без масштаба) из физики RP3D
    DirectX::XMMATRIX GetSphereRT()
    {
        using namespace DirectX;
        auto* body = playerRb_->GetBody();
        const auto& t   = body->getTransform();
        const auto& pos = t.getPosition();
        const auto& q   = t.getOrientation();

        XMVECTOR dxQ = XMVectorSet(q.x, q.y, q.z, q.w);
        XMMATRIX R   = XMMatrixRotationQuaternion(dxQ);
        XMMATRIX T   = XMMatrixTranslation(pos.x, pos.y, pos.z);
        return R * T;
    }

    // -----------------------------------------------------------------------
    std::unique_ptr<Shader>             shader_;
    std::vector<ObjSubMesh>             terrainSubmeshes_;
    std::vector<std::vector<ObjSubMesh>> spawnableSubmeshes_;

    // Игрок
    GameObject*       playerGo_         = nullptr;
    RigidBody*        playerRb_         = nullptr;
    SphereCollider*   playerSc_         = nullptr;
    PlayerController* playerController_ = nullptr;
    std::unique_ptr<Mesh>    playerMesh_;
    std::unique_ptr<Texture> playerTexture_;

    // Катамари-механика
    float                   baseRadius_    = 0.5f;
    float                   currentRadius_ = 0.5f; // обновляется в UpdateSphereSize
    float                   eatenVolume_   = 0.0f;
    std::vector<AttachedObject>              attachedObjects_;
    KatamariContactListener                  contactListener_;
    std::unordered_map<rp3d::RigidBody*, GameObject*> bodyToGo_;
    std::unordered_map<rp3d::RigidBody*, float>       bodyToVolume_;
    std::unordered_map<rp3d::RigidBody*, float>       bodyToSize_; // макс. half-extent объекта

    // Спавн
    float spawnTimer_    = 0.0f;
    float spawnInterval_ = 1.0f;
};

int main()
{
    MyApp app;
    app.Run();
}
