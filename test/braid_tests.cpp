#include "testing.h"
#include "../lib/braid.h"
#include <algorithm>
#include <ranges>
#include <string_view>

using namespace ht;
using namespace std::string_view_literals;

constexpr auto make_non_uniform_braid() {
    auto b1 = braid { "left"sv };
    auto b2 = std::move(b1) + 0.5f;
    return b2;
}

TEST(NonUniformBraid) {
    static constexpr auto br = make_non_uniform_braid(); 
    static_assert(!br.is_uniform);
    EXPECT_TRUE(!br.is_uniform);
};

constexpr auto make_sv_braid_1(){
    auto b1 = braid{ "center"sv };
    auto b2 = "left"sv + std::move(b1);
    auto b3 = std::move(b2) + "right"sv;
    return b3;
}

TEST(StringViewBraid1) {
    static constexpr auto br = make_sv_braid_1();

    static_assert(br.count() == 3);
    EXPECT(br.count(), std::equal_to, 3);

    static_assert(std::is_same_v<decltype(br.left), const std::string_view>);

    static_assert(br.is_uniform);
    EXPECT_TRUE(br.is_uniform);
    
    static constexpr auto br_size = br.reduce([](std::size_t n, const std::string_view& sv){ return n + sv.size(); }, 0);
    static_assert(br_size == 15);
    EXPECT(br_size, std::equal_to, 15);
};

constexpr auto make_sv_braid_2() {
    auto b1 = braid{ "a"sv };
    auto b2 = "there's "sv + std::move(b1);
    auto b3 = braid{ "place "sv};
    auto b4 = std::move(b3) + "downtown"sv;
    auto b5 = std::move(b2) + " "sv + std::move(b4);
    return b5;
}

TEST(StringViewBraid2){
    static constexpr auto br = make_sv_braid_2();
    EXPECT(br.count(), std::equal_to, 5);
   
    using ty = braid_common_type<std::decay_t<decltype(br)>>::type; 
    EXPECT_TRUE(br.is_uniform); 

    static constexpr auto array = sv_braid_to_str<sv_braid_size(br)>(br);

    static constexpr std::string_view as_sv{ array };
    EXPECT(as_sv.size(), std::equal_to, array.size());
    static_assert(as_sv == "there's a place downtown"sv); // where the freaks all come around
    EXPECT(as_sv, std::equal_to, "there's a place downtown"sv);
};

