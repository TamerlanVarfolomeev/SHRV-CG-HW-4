#pragma once
#include "Mesh.h"
#include "Texture.h"
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>

// Одна «группа» OBJ-файла: один материал = один меш + одна текстура.
struct ObjSubMesh
{
    std::string              name;
    std::unique_ptr<Mesh>    mesh;
    std::unique_ptr<Texture> texture;     // nullptr если текстуры нет
    DirectX::XMFLOAT4        albedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

// Загружает .obj файл через tiny_obj_loader.
// Возвращает вектор SubMesh — по одному на каждый уникальный материал.
// Текстуры загружаются автоматически по пути из .mtl файла (если файл существует).
class ObjLoader
{
public:
    // objPath — абсолютный или относительный путь к .obj файлу
    static std::vector<ObjSubMesh> Load(ID3D11Device* device, const std::string& objPath);
};
