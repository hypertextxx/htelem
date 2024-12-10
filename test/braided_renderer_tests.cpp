#include "simple_elements.h"
#include "testing.h"
#include "../lib/braided_renderer.h"
#include "../lib/html.h"

using namespace ht;
using namespace ht::ml;

TEST(BraidedSimpleElement) {
    static constexpr auto c = child{ _aaa = "test"sv };
    static constexpr auto rendered_child = braided_renderer{ }.render(c);
    static constexpr auto rendered_text_size = sv_braid_size(rendered_child);
    static constexpr auto rendered_text = sv_braid_to_str<rendered_text_size>(rendered_child);
    static constexpr auto text_view = std::string_view { rendered_text };

    EXPECT(rendered_text_size, std::equal_to, 26);
    EXPECT(text_view, std::equal_to, R"(<child aaa="test"></child>)"sv);
    
    static constexpr auto x = braided_renderer::render_element(child{ _aaa="test"sv });
    static constexpr auto lit = HT_BRAID_LITERAL(x);
    EXPECT(text_view, std::equal_to, std::string_view{ lit });
};


static constexpr auto embedded_child = child2 { _aaa = "hello"sv, _bbb = "world"sv };
static constexpr HT_EXPORT_PAGE("test_braided_embedded_element.html") auto embedded_child_html = HT_HTML_BRAID_LITERAL(embedded_child);
TEST(BraidedEmbeddedElement) {
   EXPECT(std::string_view{ embedded_child_html }, std::equal_to, R"(<child aaa="hello" bbb="world"></child>)"sv); 
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

static constexpr HT_EXPORT_PAGE("test_braided_basic_full_page.html") auto basic_full_page_html = HT_HTML_BRAID_LITERAL(basic_full_page); 

TEST(BraidedBasicFullPage) {
    EXPECT(std::string_view{ basic_full_page_html }, std::equal_to, R"(<html>
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

