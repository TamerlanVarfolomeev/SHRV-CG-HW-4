#include "Input.h"

// Определения статических полей
std::unordered_set<int>  Input::curr_;
std::unordered_set<int>  Input::prev_;
DirectX::XMFLOAT2        Input::mouseDelta_ = { 0, 0 };
float                    Input::scrollDelta_ = 0.0f;

bool Input::GetKey(Key key)
{
    return curr_.count(static_cast<int>(key)) > 0;
}

bool Input::GetKeyDown(Key key)
{
    int k = static_cast<int>(key);
    return curr_.count(k) > 0 && prev_.count(k) == 0;
}

bool Input::GetKeyUp(Key key)
{
    int k = static_cast<int>(key);
    return curr_.count(k) == 0 && prev_.count(k) > 0;
}

DirectX::XMFLOAT2 Input::GetMouseDelta()
{
    return mouseDelta_;
}

void Input::NewFrame()
{
    prev_        = curr_;
    mouseDelta_  = { 0, 0 };
    scrollDelta_ = 0.0f;
}

float Input::GetScrollDelta() { return scrollDelta_; }
void  Input::SetScrollDelta(float delta) { scrollDelta_ += delta; }

void Input::SetKeyDown(int vkCode)
{
    curr_.insert(vkCode);
}

void Input::SetKeyUp(int vkCode)
{
    curr_.erase(vkCode);
}

void Input::SetMouseDelta(int dx, int dy)
{
    mouseDelta_.x += static_cast<float>(dx);
    mouseDelta_.y += static_cast<float>(dy);
}
