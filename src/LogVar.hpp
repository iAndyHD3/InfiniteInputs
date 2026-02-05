// 1. Helper to count arguments (Supports up to 10)
#define GET_11TH_ARG(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ...) a11
#define COUNT_ARGS(...) GET_11TH_ARG(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// 2. Stringification logic for each count (1 to 10)
#define LOG_V1(v1) #v1 ": {}", v1
#define LOG_V2(v1, v2) #v1 ": {}, " #v2 ": {}", v1, v2
#define LOG_V3(v1, v2, v3) #v1 ": {}, " #v2 ": {}, " #v3 ": {}", v1, v2, v3
#define LOG_V4(v1, v2, v3, v4) #v1 ": {}, " #v2 ": {}, " #v3 ": {}, " #v4 ": {}", v1, v2, v3, v4
#define LOG_V5(v1, v2, v3, v4, v5) #v1 ": {}, " #v2 ": {}, " #v3 ": {}, " #v4 ": {}, " #v5 ": {}", v1, v2, v3, v4, v5
#define LOG_V6(v1, v2, v3, v4, v5, v6)                                                                                 \
    #v1 ": {}, " #v2 ": {}, " #v3 ": {}, " #v4 ": {}, " #v5 ": {}, " #v6 ": {}", v1, v2, v3, v4, v5, v6
#define LOG_V7(v1, v2, v3, v4, v5, v6, v7)                                                                             \
    #v1 ": {}, " #v2 ": {}, " #v3 ": {}, " #v4 ": {}, " #v5 ": {}, " #v6 ": {}, " #v7 ": {}", v1, v2, v3, v4, v5, v6, v7
#define LOG_V8(v1, v2, v3, v4, v5, v6, v7, v8)                                                                         \
    #v1 ": {}, " #v2 ": {}, " #v3 ": {}, " #v4 ": {}, " #v5 ": {}, " #v6 ": {}, " #v7 ": {}, " #v8 ": {}", v1, v2, v3, \
            v4, v5, v6, v7, v8
#define LOG_V9(v1, v2, v3, v4, v5, v6, v7, v8, v9)                                                                     \
    #v1 ": {}, " #v2 ": {}, " #v3 ": {}, " #v4 ": {}, " #v5 ": {}, " #v6 ": {}, " #v7 ": {}, " #v8 ": {}, " #v9        \
        ": {}",                                                                                                        \
            v1, v2, v3, v4, v5, v6, v7, v8, v9
#define LOG_V10(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)                                                               \
    #v1 ": {}, " #v2 ": {}, " #v3 ": {}, " #v4 ": {}, " #v5 ": {}, " #v6 ": {}, " #v7 ": {}, " #v8 ": {}, " #v9        \
        ": {}, " #v10 ": {}",                                                                                          \
            v1, v2, v3, v4, v5, v6, v7, v8, v9, v10

// 3. Dispatcher Glue
#define LOG_CONCAT(a, b) a##b
#define LOG_DISPATCH(func, count, ...) LOG_CONCAT(func, count)(__VA_ARGS__)

// 4. Public API
#define LOGI(...) log::info(LOG_DISPATCH(LOG_V, COUNT_ARGS(__VA_ARGS__), __VA_ARGS__))
#define LOGE(...) log::error(LOG_DISPATCH(LOG_V, COUNT_ARGS(__VA_ARGS__), __VA_ARGS__))
#define LOGW(...) log::warn(LOG_DISPATCH(LOG_V, COUNT_ARGS(__VA_ARGS__), __VA_ARGS__))
#define LOGD(...) log::debug(LOG_DISPATCH(LOG_V, COUNT_ARGS(__VA_ARGS__), __VA_ARGS__))
