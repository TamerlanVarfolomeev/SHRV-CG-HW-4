#pragma once
#include <windows.h>
#include <DirectXMath.h>
#include <unordered_set>

// Коды клавиш — прямое соответствие Virtual Key codes Windows.
// Используй Key::W, Key::Space, Key::Escape и т.д.
enum class Key : int
{
    // Буквы
    A = 'A', B = 'B', C = 'C', D = 'D', E = 'E', F = 'F',
    G = 'G', H = 'H', I = 'I', J = 'J', K = 'K', L = 'L',
    M = 'M', N = 'N', O = 'O', P = 'P', Q = 'Q', R = 'R',
    S = 'S', T = 'T', U = 'U', V = 'V', W = 'W', X = 'X',
    Y = 'Y', Z = 'Z',

    // Цифры
    Num0 = '0', Num1 = '1', Num2 = '2', Num3 = '3', Num4 = '4',
    Num5 = '5', Num6 = '6', Num7 = '7', Num8 = '8', Num9 = '9',

    // Стрелки
    Left  = VK_LEFT,  Right = VK_RIGHT,
    Up    = VK_UP,    Down  = VK_DOWN,

    // Спецклавиши
    Space   = VK_SPACE,
    Escape  = VK_ESCAPE,
    Enter   = VK_RETURN,
    Tab     = VK_TAB,
    Shift   = VK_SHIFT,
    Ctrl    = VK_CONTROL,
    Alt     = VK_MENU,

    // Функциональные
    F1 = VK_F1, F2 = VK_F2, F3 = VK_F3, F4  = VK_F4,
    F5 = VK_F5, F6 = VK_F6, F7 = VK_F7, F8  = VK_F8,
    F9 = VK_F9, F10= VK_F10,F11= VK_F11,F12 = VK_F12,

    // Мышь (нестандартные VK, используем свободный диапазон)
    MouseLeft   = 0x01, // VK_LBUTTON
    MouseRight  = 0x02, // VK_RBUTTON
    MouseMiddle = 0x04, // VK_MBUTTON
};

// Глобальная система ввода — вызывай из любого компонента, как в Unity.
//
// Использование:
//   if (Input::GetKey(Key::W))      { /* зажата */ }
//   if (Input::GetKeyDown(Key::Space)) { /* нажата в этот кадр */ }
//   if (Input::GetKeyUp(Key::Escape))  { /* отпущена в этот кадр */ }
//   auto delta = Input::GetMouseDelta();
class Input
{
public:
    // Зажата прямо сейчас
    static bool GetKey(Key key);

    // Нажата именно в этот кадр (не удерживалась до)
    static bool GetKeyDown(Key key);

    // Отпущена именно в этот кадр
    static bool GetKeyUp(Key key);

    // Дельта мыши за кадр (пиксели). Ненулевая только при зажатой ПКМ.
    static DirectX::XMFLOAT2 GetMouseDelta();

    // --- Внутренние методы, вызываются из Window/Application ---

    // Вызвать в начале каждого кадра — сдвигает curr → prev
    static void NewFrame();

    // Вызывается из WndProc при нажатии/отпускании
    static void SetKeyDown(int vkCode);
    static void SetKeyUp(int vkCode);

    // Вызывается из WndProc при движении мыши
    static void SetMouseDelta(int dx, int dy);

private:
    static std::unordered_set<int> curr_;   // текущее состояние
    static std::unordered_set<int> prev_;   // состояние прошлого кадра
    static DirectX::XMFLOAT2 mouseDelta_;
};
