#pragma once
#include "../Scene/Component.h"

class PlayerController : public Component
{
    public:
        void Update(float dt) override;

    private:
        float speed_ = 1.0f;
};
