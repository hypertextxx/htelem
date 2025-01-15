#ifndef HTELEM_EVENT_H
#define HTELEM_EVENT_H

#include <chrono>
namespace ht {
enum event_phase { NONE = 0, CAPTURING, AT_TARGET, BUBBLING };

template <static_string Name> struct even #ifndef HTELEM_EVENT_H
#define HTELEM_EVENT_H

#endif
        t {
    static constexpr auto name = Name;

    event_phase phase;
    bool propagate;

    const std::chrono::milliseconds timestamp;
};
}

#endif

