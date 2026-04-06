#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "ObjLoader.h"
#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include <cstdio>

// ---------------------------------------------------------------------------
// Ключ для дедупликации вершин: тройка индексов tiny_obj_loader
// ---------------------------------------------------------------------------
struct VertexKey
{
    int pos, norm, uv;
    bool operator==(const VertexKey& o) const
    {
        return pos == o.pos && norm == o.norm && uv == o.uv;
    }
};

struct VertexKeyHash
{
    size_t operator()(const VertexKey& k) const
    {
        size_t h = std::hash<int>{}(k.pos);
        h ^= std::hash<int>{}(k.norm) + 0x9e3779b9u + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(k.uv)   + 0x9e3779b9u + (h << 6) + (h >> 2);
        return h;
    }
};

// ---------------------------------------------------------------------------

std::vector<ObjSubMesh> ObjLoader::Load(ID3D11Device* device, const std::string& objPath)
{
    namespace fs = std::filesystem;

    std::string baseDir = fs::path(objPath).parent_path().string();
    if (!baseDir.empty() && baseDir.back() != '/' && baseDir.back() != '\\')
        baseDir += '/';

    tinyobj::attrib_t                attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;
    std::string                      err;

    bool ok = tinyobj::LoadObj(
        &attrib, &shapes, &materials, &err,
        objPath.c_str(),
        baseDir.c_str(),
        true /*triangulate*/);

    if (!err.empty())
        printf("[ObjLoader] %s\n", err.c_str());

    if (!ok)
        throw std::runtime_error("ObjLoader: не удалось загрузить " + objPath);

    // -----------------------------------------------------------------------
    // Группируем индексы по material_id
    // -----------------------------------------------------------------------
    std::unordered_map<int, std::vector<tinyobj::index_t>> groups;

    for (const auto& shape : shapes)
    {
        size_t faceOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            int nv    = static_cast<int>(shape.mesh.num_face_vertices[f]);
            int matId = (f < shape.mesh.material_ids.size())
                        ? shape.mesh.material_ids[f]
                        : -1;

            for (int v = 0; v < nv; v++)
                groups[matId].push_back(shape.mesh.indices[faceOffset + v]);

            faceOffset += nv;
        }
    }

    // -----------------------------------------------------------------------
    // Для каждой группы строим меш и при необходимости загружаем текстуру
    // -----------------------------------------------------------------------
    std::vector<ObjSubMesh> result;
    result.reserve(groups.size());

    for (auto& kv : groups)
    {
        int                          matId      = kv.first;
        std::vector<tinyobj::index_t>& rawIndices = kv.second;

        // --- дедупликация вершин ---
        std::unordered_map<VertexKey, uint32_t, VertexKeyHash> seen;
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
        vertices.reserve(rawIndices.size());
        indices.reserve(rawIndices.size());

        for (size_t i = 0; i < rawIndices.size(); i++)
        {
            const tinyobj::index_t& idx = rawIndices[i];
            VertexKey key{ idx.vertex_index, idx.normal_index, idx.texcoord_index };

            auto it = seen.find(key);
            if (it != seen.end())
            {
                indices.push_back(it->second);
            }
            else
            {
                Vertex v = {};

                // позиция
                const float* p = attrib.vertices.data() + 3 * idx.vertex_index;
                v.position = { p[0], p[1], p[2] };

                // нормаль
                if (idx.normal_index >= 0)
                {
                    const float* n = attrib.normals.data() + 3 * idx.normal_index;
                    v.normal = { n[0], n[1], n[2] };
                }

                // UV (D3D: ось V перевёрнута относительно OBJ)
                if (idx.texcoord_index >= 0)
                {
                    const float* t = attrib.texcoords.data() + 2 * idx.texcoord_index;
                    v.uv = { t[0], 1.0f - t[1] };
                }

                uint32_t newIdx = static_cast<uint32_t>(vertices.size());
                seen[key] = newIdx;
                vertices.push_back(v);
                indices.push_back(newIdx);
            }
        }

        ObjSubMesh sm;

        // --- материал ---
        if (matId >= 0 && matId < static_cast<int>(materials.size()))
        {
            const tinyobj::material_t& mat = materials[matId];
            sm.name = mat.name;

            // Kd: диффузный цвет. Если нулевой — белый.
            float r = mat.diffuse[0], g = mat.diffuse[1], b = mat.diffuse[2];
            if (r + g + b < 1e-4f) { r = g = b = 1.0f; }
            sm.albedoColor = { r, g, b, 1.0f };

            // Диффузная текстура
            if (!mat.diffuse_texname.empty())
            {
                std::string texPath = mat.diffuse_texname;

                // Если путь абсолютный — берём только имя файла и ищем в папке текстур
                if (texPath.find(':') != std::string::npos || texPath[0] == '/' || texPath[0] == '\\')
                {
                    std::string fileName = fs::path(texPath).filename().string();
                    texPath = baseDir + "textures/" + fileName;

                    // Если в textures/ нет — пробуем рядом с .obj
                    if (!fs::exists(texPath))
                        texPath = baseDir + fileName;
                }
                else
                {
                    texPath = baseDir + texPath;
                }

                try
                {
                    sm.texture = std::make_unique<Texture>(device, texPath);
                }
                catch (const std::exception& e)
                {
                    printf("[ObjLoader] не удалось загрузить текстуру '%s': %s\n",
                           texPath.c_str(), e.what());
                }
            }
        }
        else
        {
            sm.name = "default";
        }

        // Сохраняем позиции и индексы для MeshCollider (ConcaveMeshShape)
        sm.meshPositions.reserve(vertices.size());
        for (const auto& v : vertices)
            sm.meshPositions.push_back(v.position);

        sm.meshIndices = indices;

        sm.mesh = std::make_unique<Mesh>(device, vertices, indices);
        result.push_back(std::move(sm));
    }

    return result;
}
