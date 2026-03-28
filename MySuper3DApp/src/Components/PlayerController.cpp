#include "PlayerController.h"
#include "../Scene/GameObject.h"
#include "../Core/Input.h"

void PlayerController::Update(float dt)
{
    if (Input::GetKey(Key::W)) { gameObject->transform.position.y += speed_ * dt; }
    if (Input::GetKey(Key::S)) { gameObject->transform.position.y -= speed_ * dt; }
    if (Input::GetKey(Key::D)) { gameObject->transform.position.x += speed_ * dt; }
    if (Input::GetKey(Key::A)) { gameObject->transform.position.x -= speed_ * dt; }

}