#ifndef HTELEM_HTML_EVENT_H
#define HTELEM_HTML_EVENT_H

#include "_utilities/static_string.h"
#include "element.h"
#include "input.h"
#include <chrono>
#include <concepts>
#include <functional>

namespace ht {

enum event_phase { NONE = 0, CAPTURING, AT_TARGET, BUBBLING };

/// Base type for built-in HTML events.
template <static_string Name> struct html_event {
    static constexpr std::string_view name = Name;

    event_phase phase;
    bool propagate;
    const bool bubbles;
    const bool cancelable;
    const bool trusted;
    const std::chrono::milliseconds timestamp;

    constexpr html_event(): bubbles{false}, cancelable{false}, trusted{true}, timestamp{0} { };
    constexpr html_event(const html_event&) = default;
    constexpr html_event(html_event&&) = default;
};

/// Retrieves the name of an event, e.g. `onclick`. User-defined events should provide  their own specialization of
/// `html_event_name`.
template <class Event> struct html_event_name;
template <static_string Name> struct html_event_name<html_event<Name>> {
    static constexpr std::string_view value = Name;
};

/// Specifies that there is a specialization of \ref ht::html_event_name that can be used to retrieve the name of `T`.
template <class T> concept named_event = requires {
    { html_event_name<T>::value } -> std::equality_comparable_with<std::string_view>;
};

/// Specifies that `T` models \ref ht::named_event and that the name of `T` is `Name`.
template <class T, static_string Name> concept event_named = named_event<T> and html_event_name<T>::value == Name;

/// Base type for HTML UI events.
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

/// Emits an event to a partiuclar element or receiver. If `T` models \ref std::invocable<Event&&>, then \p to is
/// invoked with \p event as a forwarded argument. If `T` models \ref ht::element_type, then any child of \p to which
/// models `std::invocable<Event&&>` is invoked likewise.
template <class T, class Event, element_type El> constexpr Event&& emit_event(T& to, Event&& event, El& element) {
    if constexpr (element_type<T>) {
        std::apply([&to, &event]<class... Children>(Children&... children) {
            (emit_event(children, std::forward<Event>(event), to), ...);
        }, to.children);
    }

    if constexpr (requires { to(event); }) {
        std::invoke(to, event);
    } else if constexpr (requires { to(event, element); }) {
        std::invoke(to, event, element);
    }
    return std::forward<Event>(event);
}

template <class Event> constexpr Event&& emit_event(element_type auto& to, Event&& event) {
    return std::forward<Event>(emit_event(to, std::forward<Event>(event), to));
}

/// A type that is invocable with argument type `Ev&&` if and only if `Func` is and `Ev` satisfies
/// `ht::event_named<Trigger>`.
template <static_string Trigger, class Func> struct filtered_event_receiver {
    static constexpr auto trigger_name = Trigger;

    Func func;

    template <event_named<Trigger> Event, class... Args>
    constexpr std::invoke_result_t<Func, Event&, Args...> operator()(this auto& self, Event& event, Args&&... args) {
        return std::invoke(self.func, event, std::forward<Args>(args)...);
    }
};

struct event_handler_attr_type {
    constexpr bool operator==(const event_handler_attr_type&) const { return false; }
};

template <static_string At, attribute_value_type<At>... T> struct attribute_spec;

/**
 * \brief Explicit specialization for event handler attributes.
 *
 * Attributes in an element definition of the form `_onclick = [](auto&& event){ }` will be converted to children of the
 * element with the type \ref ht::filtered_event_receiver "ht::filtered_event_receiver<\"onclick\", LAMBDA>".
 */
template <static_string At> struct attribute_spec<At, event_handler_attr_type> {
    template <class Func> constexpr auto operator=(Func&& func) const {
        return filtered_event_receiver<At, Func>{std::forward<Func>(func)};
    }
};

} // namespace ht

#endif
