#include "test_runner.h"

int run_all_tests() {
    int passed = 0;
    int failed = 0;
    const auto& test_cases = get_test_cases();

    std::cout << "Running " << test_cases.size() << " test case(s)..." << std::endl;
    std::cout << std::string(40, '-') << std::endl;

    for (const auto& test_case : test_cases) {
        try {
            std::cout << "TEST: " << test_case.name << "...";
            test_case.test_function();
            std::cout << " PASSED" << std::endl;
            passed++;
        } catch (const std::exception& e) {
            std::cout << " FAILED" << std::endl;
            std::cerr << "  Reason: " << e.what() << std::endl;
            failed++;
        } catch (...) {
            std::cout << " FAILED" << std::endl;
            std::cerr << "  Reason: Unknown exception." << std::endl;
            failed++;
        }
    }

    std::cout << std::string(40, '-') << std::endl;
    std::cout << "Test Summary: " << passed << " passed, " << failed << " failed." << std::endl;

    return failed > 0 ? 1 : 0;
}
