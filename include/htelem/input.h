#ifndef HTELEM_INPUT_H
#define HTELEM_INPUT_H

#include <bitset>
namespace ht {
enum mod_key { CTRL = 0, SHIFT = 1, ALT = 2, META = 3 };

using key_modifiers = std::bitset<4>;

struct mouse_button {
    using code = short;

    static constexpr code PRIMARY = 0;
    static constexpr code AUXILIARY = 1;
    static constexpr code SECONDARY = 2;
    static constexpr code BACK = 3;
    static constexpr code FORWARD = 4;
};

enum wheel_delta_mode { PIXEL = 0, LINE = 1, PAGE = 2 };

enum key_location { STANDARD = 0, LEFT = 1, RIGHT = 2, NUMPAD = 3 };
} // namespace ht

#endif
