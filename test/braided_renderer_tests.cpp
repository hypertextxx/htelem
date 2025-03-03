#include "simple_elements.h"
#include "testing.h"
#include <htelem/braided_renderer.h>
#include <htelem/html.h>

using namespace ht;
using namespace ht::ml;

TEST(BraidedSimpleElement) {
    static constexpr auto c = child{ _aaa = "test"sv };
    static constexpr auto rendered_child = braided_renderer::render_element(c);
    static constexpr auto rendered_text_size = sv_braid_size(rendered_child);
    static constexpr auto rendered_text = sv_braid_to_str<rendered_text_size>(rendered_child);
    static constexpr auto text_view = std::string_view { rendered_text };

    STATIC_EXPECT(rendered_text_size, std::equal_to, 26);
    STATIC_EXPECT(text_view, std::equal_to, R"(<child aaa="test"></child>)"sv);
    
    static constexpr auto x = braided_renderer::render_element(child{ _aaa="test"sv });
    static constexpr auto lit = braid_literal<x>;
    STATIC_EXPECT(text_view, std::equal_to, std::string_view{ lit });
};

static constexpr auto embedded_child = child2 { _aaa = "hello"sv, _bbb = "world"sv };
static constexpr auto embedded_child_html = braided_renderer::static_html<embedded_child>;
TEST(BraidedEmbeddedElement) {
    STATIC_EXPECT(embedded_child_html, std::equal_to, R"(<child aaa="hello" bbb="world"></child>)"sv); 
};

static constexpr auto basic_full_page = html{ 
    head{ title{ "test"sv } },
    body{ 
        h1{ "heading"sv },
        ml::div{ 
            "some random text here"sv,
            a{ _href = "#nothing"sv, " a link!"sv }
        }
    }
};
static constexpr auto basic_full_page_html = braided_renderer::static_html<basic_full_page>; 
TEST(BraidedBasicFullPage) {
    STATIC_EXPECT(basic_full_page_html, std::equal_to, R"(<html>
    <head>
        <title>test</title>
    </head>
    <body>
        <h1>heading</h1>
        <div>
            some random text here
            <a href="#nothing"> a link!</a>
        </div>
    </body>
</html>)"sv);
};

static constexpr auto div_with_events = ml::div{
    _onclick = [i=0] mutable { ++i; },
    "contents"sv
};
static constexpr auto div_with_events_html = braided_renderer::static_html<div_with_events>;
TEST(DivWithEvents) {
    STATIC_EXPECT(div_with_events_html, std::equal_to, R"(<div>
    <!-- onclick -->
    contents
</div>)"sv);
};

