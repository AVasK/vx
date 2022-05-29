#include <iostream>
#include "vx.hpp"

#include <complex>
#include <cassert>
#include <array>
#include <tuple>

int main() {
    using vx::as;
    using vx::is;
    using vx::at;
    using vx::match;

    // behaves like static_cast for other types if not overloaded
    // (?) may lead to a slight ambiguity from one perspective
    // (+) looks totally normal from the other perspective if you think of variant as a sum type.
    std::cout << (3.14 |as <int>);

    // testing & getting variant alternatives
    std::variant<int, float> v = 3.14f;
    if (v |is <float>) std::cout << (v |as <float>);
    v |as <float> = 7.77f;
    std::cout << (v |at <1>);


    constexpr std::variant<int, float> w = 3.14 | as<float>;
    if constexpr (w |is <float>) {
        std::cout << "\nw: " << (w |as <float>) << "\n";
    }

    w | match {
        [](int x){ std::cout << "int " << x << "\n"; },
        [](float x){ std::cout << "float " << x << "\n"; }
    };

    // as / at can be used instead of std::in_place_type / std::in_place_index
    std::variant<int, std::complex<double>> v2 {as<std::complex<double>>, 3, 3};
    assert( v2 |is <std::complex<double>>);

    std::variant<int, std::complex<double>> v3 {at<1>, 3, 3};
    assert( v3 |is <std::complex<double>>);

    // The syntax for std::any is similar!
    std::any any_v = std::string("abcd");
    if (any_v |is <std::string>) {
        std::cout << (any_v |as <std::string>);
    }

    // Can use at<> with tuples, arrays, e.t.c.
    std::tuple tup (1, "2", '3', 4.f);
    tup |at <0> = 7;
    std::cout << (tup |at <0>);

    std::array<int, 5> arr = {1,2,3,4,5};
    std::cout << "arr[4] = " << (arr |at <4>);

}