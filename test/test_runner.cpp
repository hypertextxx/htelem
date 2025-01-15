#include "testing.h"

std::vector<testing::controller> testing::all_tests;

int main(int argc, char* argv[]) {
    auto n_tests = testing::all_tests.size();
    std::size_t n_passed = 0;
    std::for_each(testing::all_tests.begin(), testing::all_tests.end(), [&, i = 0](auto& test) mutable {
        std::print("[{}/{}] {}\n", ++i, n_tests, test.name);
        std::size_t failed_expects = std::invoke(test);
        if (failed_expects == 0) {
            n_passed++;
            std::print("[{}/{}] {} passed\n", i, n_tests, test.name);
        } else {
            std::print("[{}/{}] {} failed ({} items)", i, n_tests, test.name, failed_expects);
        }
    });
    std::print("{}/{} tests passed.\n", n_passed, n_tests);
    return static_cast<int>(n_tests - n_passed);
}
