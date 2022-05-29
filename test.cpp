#include <iostream>
#include "vx.hpp"
// Testing
#include <complex>
#include <cassert>
#include <array>
#include <tuple>

struct TestCopyEllision {
    int x;
    operator int() const { return x; }
    TestCopyEllision(int v=7) : x{v} {}
    TestCopyEllision(TestCopyEllision const&) = delete;
    TestCopyEllision& operator= (TestCopyEllision const&) = delete;
};

int main() {
    using vx::as;
    using vx::is;
    using vx::at;

    [[maybe_unused]] auto x = 3.14 |as <int>;
    std::cout << (3.14 |as <int>);

    auto y = TestCopyEllision{8} |as <int>;
    if constexpr (std::is_same_v<decltype(y), int>) {
        std::cout << "y is " << y << "\n";
    }

    std::variant<int, float> v = 3.14f;
    if (v |is <float>) std::cout << (v |as <float>);
    v |as <float> = 7.77;
    std::cout << (v |at <1>);


    std::variant<int, std::complex<double>> v2 {as<std::complex<double>>, 3, 3};
    assert( v2 |is <std::complex<double>>);

    std::variant<int, std::complex<double>> v3 {at<1>, 3, 3};
    assert( v3 |is <std::complex<double>>);

    std::any any_v = 1; //std::string("abcd");
    if (any_v |is <std::string>) {
        std::cout << (any_v |as <std::string>);
    }

    std::tuple tup (1, "2", '3', 4.f);
    tup |at <0> = 7;
    std::cout << (tup |at <0>);

}