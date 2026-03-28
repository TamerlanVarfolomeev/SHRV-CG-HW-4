#include "FoodManager.h"
#include "MeshRenderer.h"
#include "../Scene/GameObject.h"
#include "../Scene/Scene.h"



FoodManager::FoodManager(Scene* scene, ID3D11Device* device, Shader* shader, GameObject* player)
{
	mesh_Circle = std::make_unique<Mesh>(Mesh::CreateCircle(device, 32, 1.0f));
	player_ = player;
	for (int i = 0; i < maxFoodCount_; i++)
	{
		auto* food = scene->CreateObject("Food");
		auto* mr = food->AddComponent<MeshRenderer>(device, mesh_Circle.get(), shader);
		mr->material.albedoColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // Белый
		food->transform.position = { static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 4))-2.0f, static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 4))-2.0f, 0.1f};
		food->transform.scale = { 0.033,  0.033,  0.033 };
		foodObjects_.push_back(food);
	}
}

void FoodManager::Update(float dt)
{
	for (int i = 0; i < foodObjects_.size(); i++)
	{
		float dx = foodObjects_[i]->transform.position.x - player_->transform.position.x;
		float dy = foodObjects_[i]->transform.position.y - player_->transform.position.y;
		float distSq = dx * dx + dy * dy;
		if (distSq < player_->transform.scale.x * player_->transform.scale.x)
		{
			foodObjects_[i]->transform.position = { static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 4)) - 2.0f, static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 4)) - 2.0f, 0.1f };
			player_->transform.scale = { player_->transform.scale.x * 1.1f, player_->transform.scale.y * 1.1f, player_->transform.scale.z * 1.1f };
			break;
		}
	}

}