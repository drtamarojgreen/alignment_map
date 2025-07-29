#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>

// --- Test Case Management ---
struct TestCase {
    std::string name;
    std::function<void()> test_function;
};

inline std::vector<TestCase>& get_test_cases() {
    static std::vector<TestCase> test_cases;
    return test_cases;
}

// Auto-registration of test cases
struct TestRegister {
    TestRegister(const std::string& name, std::function<void()> func) {
        get_test_cases().push_back({name, func});
    }
};

#define TEST_CASE(name) \
    void test_case_##name(); \
    TestRegister register_##name(#name, test_case_##name); \
    void test_case_##name()

// --- Assertion Macros ---
#include <sstream>

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::stringstream ss; \
        ss << "Assertion failed: " #condition " is not true at " << __FILE__ << ":" << __LINE__; \
        throw std::runtime_error(ss.str()); \
    }

#define ASSERT_FALSE(condition) \
    if ((condition)) { \
        std::stringstream ss; \
        ss << "Assertion failed: " #condition " is not false at " << __FILE__ << ":" << __LINE__; \
        throw std::runtime_error(ss.str()); \
    }

#define ASSERT_EQUAL(actual, expected) \
    if ((actual) != (expected)) { \
        std::stringstream ss; \
        ss << "Assertion failed: " #actual " (" << (actual) << ") != " #expected " (" << (expected) << ") at " << __FILE__ << ":" << __LINE__; \
        throw std::runtime_error(ss.str()); \
    }

#define ASSERT_NOT_EQUAL(actual, expected) \
    if ((actual) == (expected)) { \
        std::stringstream ss; \
        ss << "Assertion failed: " #actual " (" << (actual) << ") == " #expected " (" << (expected) << ") at " << __FILE__ << ":" << __LINE__; \
        throw std::runtime_error(ss.str()); \
    }

#define ASSERT_THROWS(expression) \
    try { \
        (expression); \
        std::stringstream ss; \
        ss << "Assertion failed: " #expression " did not throw at " << __FILE__ << ":" << __LINE__; \
        throw std::runtime_error(ss.str()); \
    } catch (...) {}

// --- Test Runner ---
int run_all_tests();

#endif // TEST_RUNNER_H
