#include "testing.h"

std::vector<testing::controller> testing::all_tests;

int main(int argc, char* argv[]) {
    auto n_tests = testing::all_tests.size();
    std::size_t n_passed = 0;
    std::for_each(testing::all_tests.begin(), testing::all_tests.end(), [&, i = 0](auto& test) mutable {
        std::cout << "[" << ++i << "/" << n_tests <<  "] " << test.name << std::endl;
        std::size_t failed_expects = std::invoke(test);
        if (failed_expects == 0) {
            n_passed++;
            std::cout << "[" << i << "/" << n_tests << "] " << test.name << " passed" << std::endl;
        } else {
            std::cout << "[" << i << "/" << n_tests << "] " << test.name << " failed (" << failed_expects << " items)" << std::endl;
        }
    });
    std::cout << n_passed << "/" << n_tests << " tests passed." << std::endl;
    return static_cast<int>(n_tests - n_passed);
}
