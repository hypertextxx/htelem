#include "testing.h"
#include "../lib/static_string.h"

using namespace ht;
using namespace std::string_view_literals;

TEST(SvBraidSize) {
    constexpr std::size_t size = [](){
        sv_braid b { "i"sv };
        auto b1 = b + "j"sv;
        auto b2 = "123"sv + b1;
        return b2.size();
    }();
};
