# vx - the much needed syntactic sugar and unification for std::variant, std::optional, std::any, (parts of it work with std::tuple, std::array & more...)

 - `match` for std::variant 
 - generic `as` / `is` / `at` ops for variant/any/tuple/array

 > Supports C++17, but uses C++20 concepts when available.

## match
Let's be honest, std::visit doesn't look particularly good, even with overloaded{} pattern the visitation leaves *a lot* to be desired. The most common case is trying to visit (for some it'll look like matching...) a single variant to ~~/with/whatever you call it~~ multiple functions, i.e. this:
```C++
std::visit(overloaded {
    [](int i) {...},
    [](float f) {...}
}, v); // <--- v comes last,
// not to mention that the "std::visit(overloaded {" part could definitely be better...
```
And it also seems to be the usecase with the greatest optimisation potential 
> e.g. using jump-tables and making it possible for the compiler to successfully inline functions vs. using a table of function pointers which are hard for the compiler to see through.

Regardless, we are visiting a single variant, so why pay (in code clarity) for what we don't use?
It's just a simple operation on the variant type and a pretty common one. It's the preferred way over `is` and `as` (or if you still prefer `std::holds_alternative` and `std::get`) for variants with sufficiently many alternatives that it makes manually handling cases with `if`s monotonous and error-prone...
So why make it more complex than it needs to be? Won't it be *a lot* more readable were it written like that instead?
```C++
w | match {
    [](int i){ ... }, 
    [](float f){ ... }
};
```

## `match`ing optionals:
matching construct can be used with `optional` values to force **exhaustiveness**

> matching std::optional
```C++
    std::optional<int> o;
    o | match {
        []()      { std::cout << "nothing\n"; }, 
        [](int v) { std::cout << "int "<< v << "\n"; }
        // []     { std::cout << "nothing\n"; } // another way to say "empty"
    };
```

The match{...} won't compile if the cases are non-exhaustive.

## is<> and as<> (and at<>)
### Why?
To get an alternative from some type there's a std::get<> that works great, but then all of a sudden there's no std::get<> for std::any and instead what we get is std::any_cast<>...

To query what type does our variable hold we should use `std::holds_alternative` for `std::variant` and `.type()` for `std::any`, both of which try to answer the same question: does this variant/any/e.t.c. hold a type T? in other words, `is` it *(or rather the value inside it)* of type T?

`as<>` makes it easier to get what you want from the variant/any in a generic way. 
> For the types that don't specialise `as` it behaves as a `static_cast`. 

`at` and `as` can also be used where you'd otherwise write   `std::in_place_index<>` and `std::in_place_type<>` respectively, so if you, like me, had trouble remembering all those names and disliked the way those monstrosities devoured the free space on the line of code they appeared, behold, there's a sexy new `as<>` (and `at<>`, too). Or at least as sexy as it gets unless we get a language-level keyword for `as` and `is` some day... ~~pretty sure we ain't getting one for `at` though~~

## Examples
> ## extending variant

Assuming we have some variant, e.g.
```C++
std::variant<int, float> v = 3.14f;
```
to do something if it holds a `float` we'll need
>  Standard way:
> ```C++
> if (std::holds_alternative<float>(v)) {
>     std::get<float>(v) = 7.77f; // (1)
>     std::get<1>(v) = 7.77f;     // (2)
> }
> ```

> The proposed way:
> ```C++
> if (v |is <float>) {
>     v |as <float> = 7.77f;      // (1)
>     v |at <1> = 7.77f;          // (2)
> }
> ```

When constructing in place:
> Standard approach:
> ```C++
> std::variant<...> v {std::in_place_type<X>, args...};
> std::variant<...> v {std::in_place_index<I>, args...};
> ```

> Proposed approach:
> ```C++
> std::variant<...> v {as<X>, args...};
> std::variant<...> v {at<1>, args...};
> ```

> ## extending any

```C++
std::any v = 3.14f;
```

```C++
if (v.type() == typeid(std::string)) {
    std::cout << (std::any_cast<std::string>(v));
}
```

```C++
if (v |is <float>) {
    v |as <float> = 7.77f;      // (1)
}
```
Starting to see the similarity?)
