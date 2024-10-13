#include <chrono>
#include <cassert>

using namespace std;
using namespace chrono;
// If parameter is not true, test fails
// This check function would be provided by the test framework
#define IS_TRUE(x) { if (!(x)) cout << __FUNCTION__ << " failed on line " << __LINE__ << endl; }


#define time_it(descr, a) \
    do { \
        auto start = high_resolution_clock::now(); \
        a; \
        auto stop = high_resolution_clock::now(); \
        auto duration = duration_cast<microseconds>(stop - start); \
        cout << descr << ": " << fixed << duration.count() / (1000.0) << "ms" << endl; \
    } while (0)

#define time_it_n(descr, a, N) \
    do { \
    auto start = high_resolution_clock::now(); \
    for(int i=0; i < N; i++){ \
        a; \
    } \
    auto stop = high_resolution_clock::now(); \
    auto duration = duration_cast<microseconds>(stop - start); \
    cout << descr << ": " << fixed << duration.count() / (1000.0 * N) << "ms" << endl; \
    } while (0)

#define EXPECT_THROW(expr) \
    try { \
        expr; \
        assert(false && "Expression should have thrown an exception"); \
    } catch (const std::exception& e) { \
        /* Exception caught, as expected */ \
    }
