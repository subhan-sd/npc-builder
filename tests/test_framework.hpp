#pragma once
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace npc::test {

inline int& failure_count() {
    static int count = 0;
    return count;
}

inline void report_failure(const char* expr, const char* file, int line, const std::string& msg = "") {
    std::cerr << "[FAIL] " << file << ":" << line << ": assertion failed (" << expr << ")";
    if (!msg.empty()) {
        std::cerr << " -> " << msg;
    }
    std::cerr << "\n";
    failure_count()++;
}

#define TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            ::npc::test::report_failure(#cond, __FILE__, __LINE__); \
        } \
    } while (0)

#define TEST_ASSERT_MSG(cond, msg) \
    do { \
        if (!(cond)) { \
            ::npc::test::report_failure(#cond, __FILE__, __LINE__, (msg)); \
        } \
    } while (0)

#define TEST_ASSERT_EQ(actual, expected) \
    do { \
        auto _act = (actual); \
        auto _exp = (expected); \
        if (!(_act == _exp)) { \
            std::ostringstream _ss; \
            _ss << "expected " << _exp << ", got " << _act; \
            ::npc::test::report_failure(#actual " == " #expected, __FILE__, __LINE__, _ss.str()); \
        } \
    } while (0)

#define TEST_ASSERT_LT(val1, val2) \
    do { \
        auto _v1 = (val1); \
        auto _v2 = (val2); \
        if (!(_v1 < _v2)) { \
            std::ostringstream _ss; \
            _ss << "expected " << _v1 << " < " << _v2; \
            ::npc::test::report_failure(#val1 " < " #val2, __FILE__, __LINE__, _ss.str()); \
        } \
    } while (0)

#define TEST_ASSERT_GT(val1, val2) \
    do { \
        auto _v1 = (val1); \
        auto _v2 = (val2); \
        if (!(_v1 > _v2)) { \
            std::ostringstream _ss; \
            _ss << "expected " << _v1 << " > " << _v2; \
            ::npc::test::report_failure(#val1 " > " #val2, __FILE__, __LINE__, _ss.str()); \
        } \
    } while (0)

inline uint64_t fnv1a_64(const uint8_t* data, size_t length) {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < length; ++i) {
        hash ^= static_cast<uint64_t>(data[i]);
        hash *= 1099511628211ULL;
    }
    return hash;
}

} // namespace npc::test
