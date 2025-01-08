#ifndef HTELEM_HTML_EVENT_H
#define HTELEM_HTML_EVENT_H

#include "htelem/element.h"
#include "htelem/input.h"
#include "htelem/static_string.h"
#include <chrono>
#include <functional>

namespace ht {

enum event_phase { NONE = 0, CAPTURING_PHASE, AT_TARGET, BUBBLING_PHASE };

template <static_string Name> struct html_event {
    static constexpr std::string_view name = Name;

    event_phase phase;
    bool propagate;
    const bool bubbles;
    const bool cancelable;
    const bool trusted;
    const std::chrono::milliseconds timestamp;
};

template <class Event> struct html_event_name;
template <static_string Name> struct html_event_name<html_event<Name>> {
    static constexpr std::string_view value = Name;
};
template <class Event> constexpr auto html_event_name_v = html_event_name<std::decay_t<Event>>::value;

template <class T, static_string Name> concept event_named = requires {
    typename html_event_name<T>;
    html_event_name_v<T> == Name;
};

template <static_string Name, class Window> struct ui_event: html_event<Name> {
    Window& window;
};

#define UI_EVENT_DECL(name) template <static_string Name, class Window> struct name

UI_EVENT_DECL(mouse_event): ui_event<Name, Window> {
    using coord = Window::coordinate_type;

    const coord screen_x;
    const coord screen_y;
    const coord client_x;
    const coord client_y;
    const coord layer_x;
    const coord layer_y;
    const key_modifiers mods;
    const mouse_button::code mouse_buttons;
};

UI_EVENT_DECL(wheel_event): mouse_event<Name, Window> {
    const double delta_x;
    const double delta_y;
    const double delta_z;
    const wheel_delta_mode mode;
};

UI_EVENT_DECL(input_event): ui_event<Name, Window> {
    const std::string_view data;
    const std::string_view input_type;
};

UI_EVENT_DECL(keyboard_event): ui_event<Name, Window> {
    const std::string_view key;
    const std::string_view code;
    const key_location location;
    const key_modifiers mods;
    const bool repeat;
};

#undef UI_EVENT_DECL

template <class T, class Event> constexpr decltype(auto) propagate_event(const T& to, Event&& event) {
    if constexpr (is_element_type_v<T>) {
        std::apply([&event]<class... Children>(Children&... children) {
            ((propagate_event(children, std::forward<Event>(event))), ...);
        }, to.children());
    }

    if constexpr (std::is_invocable_v<T, Event&>) {
        std::invoke(to, std::forward<Event>(event));
    }
    return std::forward<Event>(event);
}

} // namespace ht

#endif
