#pragma once

class GraphicsDevice;
class Camera;

// Лёгкая структура, передаётся в Component::Render().
// Содержит всё необходимое для отрисовки одного компонента.
struct RenderContext
{
    GraphicsDevice* gfx    = nullptr;
    Camera*         camera = nullptr;
};
