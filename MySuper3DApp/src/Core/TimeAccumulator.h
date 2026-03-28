#pragma once
#include <chrono>

// Отслеживает время между кадрами.
// Используется для фиксированного шага физики.
class TimeAccumulator
{
public:
    // Вызвать один раз в начале кадра.
    // Возвращает deltaTime в секундах.
    float Tick()
    {
        auto now   = std::chrono::steady_clock::now();
        delta_     = std::chrono::duration<float>(now - prev_).count();
        prev_      = now;
        accumulated_ += delta_;

        // Защита от спайков (например, при брейкпоинте в дебаггере)
        if (delta_ > 0.1f) delta_ = 0.1f;

        return delta_;
    }

    // Есть ли накопленное время для фиксированного шага?
    bool ShouldStep(float fixedDt)
    {
        if (accumulated_ >= fixedDt)
        {
            accumulated_ -= fixedDt;
            return true;
        }
        return false;
    }

    float GetDelta()       const { return delta_; }
    float GetAccumulated() const { return accumulated_; }

private:
    std::chrono::steady_clock::time_point prev_ = std::chrono::steady_clock::now();
    float delta_       = 0.0f;
    float accumulated_ = 0.0f;
};
