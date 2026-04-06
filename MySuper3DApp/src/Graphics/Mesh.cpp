#include "Mesh.h"
#include <stdexcept>

Mesh::Mesh(ID3D11Device* device,
           const std::vector<Vertex>& vertices,
           const std::vector<uint32_t>& indices)
    : indexCount_(static_cast<uint32_t>(indices.size()))
{
    // Vertex buffer
    D3D11_BUFFER_DESC vbd  = {};
    vbd.Usage              = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth          = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    vbd.BindFlags          = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vd = {};
    vd.pSysMem = vertices.data();

    if (FAILED(device->CreateBuffer(&vbd, &vd, &vb_)))
        throw std::runtime_error("Failed to create vertex buffer");

    // Index buffer
    D3D11_BUFFER_DESC ibd  = {};
    ibd.Usage              = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth          = static_cast<UINT>(sizeof(uint32_t) * indices.size());
    ibd.BindFlags          = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA id_ = {};
    id_.pSysMem = indices.data();

    if (FAILED(device->CreateBuffer(&ibd, &id_, &ib_)))
        throw std::runtime_error("Failed to create index buffer");
}

void Mesh::Draw(ID3D11DeviceContext* ctx) const
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, vb_.GetAddressOf(), &stride, &offset);
    ctx->IASetIndexBuffer(ib_.Get(), DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->DrawIndexed(indexCount_, 0, 0);
}

Mesh Mesh::CreateQuad(ID3D11Device* device)
{
    std::vector<Vertex> verts = {
        { {-0.5f,  0.5f, 0.0f}, {0,0,-1}, {0,0} },
        { { 0.5f,  0.5f, 0.0f}, {0,0,-1}, {1,0} },
        { { 0.5f, -0.5f, 0.0f}, {0,0,-1}, {1,1} },
        { {-0.5f, -0.5f, 0.0f}, {0,0,-1}, {0,1} },
    };
    std::vector<uint32_t> idx = { 0,1,2, 0,2,3 };
    return Mesh(device, verts, idx);
}


Mesh Mesh::CreateCircle(ID3D11Device* device, int vertexCount, float radius)
{
    using namespace DirectX;
    const float PI = acos(-1.0f);
    // Цетнральная вершина
    std::vector<Vertex> verts = {
        { {0.0f,  0.0f, 0.0f}, {0,0,-1}, {0,0} }
    };
    std::vector<uint32_t> idx;

    for (int i = 0; i < vertexCount; i++)
    {
        float angle = 2.0f * PI * i / vertexCount;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        verts.push_back({ {x, y, 0.0f}, {0,0,0}, {0,0} });
    }

    for (int i = 0; i < vertexCount; i++)
    {
        int a = i + 1;
        int b = (i + 1) % vertexCount + 1;

        idx.push_back(0);
        idx.push_back(b);
        idx.push_back(a);
    }

    return Mesh(device, verts, idx);
}

Mesh Mesh::CreateSphere(ID3D11Device* device, int stacks, int sectors)
{
    const float PI = acosf(-1.0f);
    std::vector<Vertex> verts;
    std::vector<uint32_t> idx;

    for (int i = 0; i <= stacks; i++)
    {
        float phi = PI * (-0.5f + static_cast<float>(i) / stacks);
        float y   = sinf(phi);
        float r   = cosf(phi);

        for (int j = 0; j <= sectors; j++)
        {
            float theta = 2.0f * PI * static_cast<float>(j) / sectors;
            float x = r * cosf(theta);
            float z = r * sinf(theta);

            Vertex v;
            v.position = { x, y, z };
            v.normal   = { x, y, z };
            v.uv       = { static_cast<float>(j) / sectors,
                           1.0f - static_cast<float>(i) / stacks };
            verts.push_back(v);
        }
    }

    for (int i = 0; i < stacks; i++)
    {
        for (int j = 0; j < sectors; j++)
        {
            uint32_t a = i * (sectors + 1) + j;
            uint32_t b = a + sectors + 1;

            // CW winding (D3D11 CULL_BACK)
            idx.push_back(a);     idx.push_back(a + 1); idx.push_back(b);
            idx.push_back(a + 1); idx.push_back(b + 1); idx.push_back(b);
        }
    }

    return Mesh(device, verts, idx);
}

Mesh Mesh::CreateColorSphere(ID3D11Device* device, float radius, int stacks, int sectors)
{
    const float PI = acosf(-1.0f);
    std::vector<Vertex> verts;
    std::vector<uint32_t> idx;

    // Генерируем вершины: каждая вершина уникальна (без дедупликации),
    // чтобы каждый треугольник мог иметь свой уникальный UV.
    // UV = (треугольник_x / всего_треугольников, треугольник_y)
    int triCount = stacks * sectors * 2;
    int triPerRow = sectors * 2;

    for (int i = 0; i < stacks; i++)
    {
        float phi0 = PI * (-0.5f + static_cast<float>(i) / stacks);
        float phi1 = PI * (-0.5f + static_cast<float>(i + 1) / stacks);

        float y0 = sinf(phi0);
        float y1 = sinf(phi1);
        float r0 = cosf(phi0);
        float r1 = cosf(phi1);

        for (int j = 0; j < sectors; j++)
        {
            float theta0 = 2.0f * PI * static_cast<float>(j) / sectors;
            float theta1 = 2.0f * PI * static_cast<float>(j + 1) / sectors;

            // 4 вершины квада
            float x00 = r0 * cosf(theta0), z00 = r0 * sinf(theta0);
            float x01 = r0 * cosf(theta1), z01 = r0 * sinf(theta1);
            float x10 = r1 * cosf(theta0), z10 = r1 * sinf(theta0);
            float x11 = r1 * cosf(theta1), z11 = r1 * sinf(theta1);

            uint32_t triIdx = i * triPerRow + j * 2;
            float uv0x = static_cast<float>(triIdx % triPerRow) / triPerRow;
            float uv0y = static_cast<float>(triIdx / triPerRow) / stacks;
            float uv1x = static_cast<float>((triIdx + 1) % triPerRow) / triPerRow;
            float uv1y = uv0y;

            // Triangle 1
            float base0 = static_cast<float>(verts.size());
            verts.push_back({ {x00 * radius, y0 * radius, z00 * radius}, {x00, y0, z00}, {uv0x, uv0y} });
            verts.push_back({ {x01 * radius, y0 * radius, z01 * radius}, {x01, y0, z01}, {uv1x, uv0y} });
            verts.push_back({ {x10 * radius, y1 * radius, z10 * radius}, {x10, y1, z10}, {(uv0x + uv1x) * 0.5f, uv1y} });
            idx.push_back((uint32_t)base0);
            idx.push_back((uint32_t)base0 + 2);
            idx.push_back((uint32_t)base0 + 1);

            // Triangle 2
            float base1 = static_cast<float>(verts.size());
            verts.push_back({ {x01 * radius, y0 * radius, z01 * radius}, {x01, y0, z01}, {uv1x, uv0y} });
            verts.push_back({ {x11 * radius, y1 * radius, z11 * radius}, {x11, y1, z11}, {uv1x, uv1y} });
            verts.push_back({ {x10 * radius, y1 * radius, z10 * radius}, {x10, y1, z10}, {uv0x, uv1y} });
            idx.push_back((uint32_t)base1);
            idx.push_back((uint32_t)base1 + 2);
            idx.push_back((uint32_t)base1 + 1);
        }
    }

    return Mesh(device, verts, idx);
}

Mesh Mesh::CreateCube(ID3D11Device* device)
{
    // 6 граней, по 4 вершины на каждую (уникальные нормали)
    using namespace DirectX;
    const XMFLOAT3 n[6] = {
        {0,0,-1}, {0,0,1},  // Front, Back
        {-1,0,0}, {1,0,0},  // Left,  Right
        {0,1,0},  {0,-1,0}, // Top,   Bottom
    };

    std::vector<Vertex> verts;
    std::vector<uint32_t> idx;

    // Каждая грань: 4 вершины + 2 треугольника
    auto addFace = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d, XMFLOAT3 norm)
    {
        uint32_t base = static_cast<uint32_t>(verts.size());
        verts.push_back({ a, norm, {0,0} });
        verts.push_back({ b, norm, {1,0} });
        verts.push_back({ c, norm, {1,1} });
        verts.push_back({ d, norm, {0,1} });
        idx.insert(idx.end(), { base,base+1,base+2, base,base+2,base+3 });
    };

    addFace({-0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, n[0]); // Front
    addFace({ 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, n[1]); // Back
    addFace({-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, n[2]); // Left
    addFace({ 0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f,-0.5f}, n[3]); // Right
    addFace({-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, n[4]); // Top
    addFace({-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f}, n[5]); // Bottom

    return Mesh(device, verts, idx);
}