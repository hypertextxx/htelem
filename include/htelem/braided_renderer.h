#ifndef HTELEM_BRAIDED_RENDERER_H
#define HTELEM_BRAIDED_RENDERER_H

#include "attribute_list.h"
#include "braid.h"
#include "element.h"
#include "htelem/html_event.h"
#include "interface_spec.h"
#include "render_common.h"
#include <tuple>

namespace ht {
using namespace std::string_view_literals;

struct braided_renderer {
    template <std::size_t Depth = 0, std::convertible_to<std::string_view> T>
    constexpr sv_braid auto render(const T& sv, const render_decoration&& dec = {}) const {
        return braid{std::move(dec.pre), std::move(static_cast<std::string_view>(sv))} + std::move(dec.post);
    }

    template <std::size_t Depth = 0, class T>
    requires(sv_braid<std::invoke_result_t<custom_renderer<T>, const T&, const render_decoration &&>>)
    constexpr sv_braid auto render(const T& t, const render_decoration&& dec = {}) const {
        return std::move(custom_renderer<T>{}(t, std::move(dec)));
    }

    template <std::size_t Depth = 0, static_string AttrName, class AttrType>
    constexpr sv_braid auto render(this auto&& self, const attribute<AttrName, AttrType>& attr,
            const render_decoration&& dec = {}) {
        auto b1 = braid{std::move(dec.pre), std::move(self.render(AttrName))};
        auto b2 = std::move(b1) + "=\""sv;
        auto b3 = std::move(b2) + std::move(self.template render<Depth, AttrType>(*attr));
        auto b4 = std::move(b3) + "\""sv;
        auto b5 = std::move(b4) + std::move(dec.post);
        return b5;
    }

    template <std::size_t Depth = 0, static_string Tag, static_string In, class... Parents, auto... Attrs,
            class... Aspects>
    constexpr sv_braid auto render(this auto&& self,
            const element<Tag, interface_spec<In, std::tuple<Parents...>, Attrs...>, Aspects...>& el,
            const render_decoration&& dec = {}) {
        using element_type = const element<Tag, interface_spec<In, std::tuple<Parents...>, Attrs...>, Aspects...>;
        using interface = element_type::interface;

        auto attributes = [&]<class... SetAttrs>(std::tuple<SetAttrs...> tp) {
            if constexpr (sizeof...(SetAttrs) > 0) {
                return (std::move(self.render(el.*(std::get<attr_to_mem_ptr_t<SetAttrs, interface>>(
                                                     combined_interface_attrs_v<interface>)),
                                {.pre = " "sv})) +
                        ...);
            } else {
                return braid{""sv};
            }
        }(typename element_type::set_attrs_tuple{});

        auto tag_in_1 = braid{indent<0>(), "<"sv};
        auto tag_in_2 = std::move(tag_in_1) + static_cast<std::string_view>(Tag);
        auto tag_in_3 = std::move(tag_in_2) + std::move(attributes);
        auto tag_in = std::move(tag_in_3) + ">"sv;

        auto tag_out_1 = braid{"</"sv, static_cast<std::string_view>(Tag)};
        auto tag_out = std::move(tag_out_1) + ">"sv;

        if constexpr (std::tuple_size_v<typename element_type::children_tuple> == 0) {
            return std::move(dec.pre) + std::move(tag_in) + std::move(tag_out) + dec.post;
        } else if constexpr (std::tuple_size_v<typename element_type::children_tuple> == 1 &&
                             std::is_convertible_v<std::tuple_element_t<0, typename element_type::children_tuple>,
                                     std::string_view>) {
            auto b1 = std::move(tag_in) + std::move(self.render(std::get<0>(el.children)));
            return std::move(dec.pre) + std::move(b1) + std::move(tag_out) + dec.post;
        } else {
            auto ch = "\n"sv + std::move(std::apply([&]<class... C>(const C&... c) {
                return (std::move(self.template render<Depth + 1>(c, {.pre = indent<Depth + 1>(), .post = "\n"sv})) + ...);
            }, el.children));
            return std::move(dec.pre) + std::move(tag_in) + std::move(ch) + indent<Depth>() + std::move(tag_out) +
                   std::move(dec.post);
        }
    }

    template <std::size_t Depth = 0, static_string Trigger, class F>
    constexpr sv_braid auto render(const filtered_event_receiver<Trigger, F>& t,
        const render_decoration&& dec = {}) const {
        auto b1 = braid{std::move(dec.pre), "<!-- "sv};
        return std::move(b1) + braid{static_cast<std::string_view>(Trigger), " -->"sv} + std::move(dec.post);
    }

    template <class Element> static constexpr auto render_element(const Element& el) {
        return braided_renderer{}.render(el);
    }

    template <const auto& Element> struct make_static_html {
        static constexpr auto braid = render_element(Element);
        static constexpr auto sv = braid_literal_view<braid>;
    };

    template <const auto& Element> static constexpr auto static_html = make_static_html<Element>::sv;
};
} // namespace ht

#endif
