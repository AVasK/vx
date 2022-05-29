#pragma once

#include <type_traits>
#include <initializer_list>
#include <optional>


#ifndef __has_builtin         
#   define __has_builtin(x) 0
#endif

#ifndef __has_feature         
#   define __has_feature(x) 0
#endif

#ifndef __has_extension        
#   define __has_extension(x) 0
#endif

#ifndef __has_cpp_attribute 
#   define __has_cpp_attribute(x) 0
#endif

#define metafunc template<class...> class

namespace meta {

//===========[ REQUIRE ]==========
template <bool Cond>
using require = typename std::enable_if<Cond>::type;


//===========[ VALID : akin to void_t ]===========
struct check_valid {};

namespace detail {
    template <typename...>
    struct is_valid {
        using type = check_valid;
    };
}

template <typename... Args> 
using is_valid = typename detail::is_valid<Args...>::type;


//==========[ BASIC META TYPES ]========
template <typename T>
struct identity {
    using type = T;
};

struct nothing {};


template <class Base>
struct inherit_from : Base {
    using base = Base;
};

template <typename _, bool flag>
struct dependent_flag {
    static constexpr bool value = flag;
};

// Rationale:
// static_assert(assert_false<_>) will only fire @ instantiation time
// whereas the static_assert(false) will fire @ the first pass
template <typename _>
inline constexpr auto assert_false = dependent_flag<_, false>::value;


template <typename T>
using extract_type = typename T::type;

template <typename T>
struct always_true : std::true_type {};


// =====[ typelist ]=====
template <typename... Ts>
struct typelist;// {}; // We don't even need to actually define it

// =====[ HEAD & TAIL (from type packs) ]=====
namespace detail {

    template <typename... Ts>
    struct head_impl;

    template <typename T, typename... Ts>
    struct head_impl<T,Ts...> {
        using type = T;
    };

    template <>
    struct head_impl<> {
        using type = nothing;
    };


    template <typename... Ts>
    struct tail_impl;

    template <typename T, typename... Ts>
    struct tail_impl<T,Ts...> {
        using type = typelist<Ts...>;
    };

    template <>
    struct tail_impl<> {
        using type = typelist<>;
    };
}

template <typename... Ts>
using head = typename detail::head_impl<Ts...>::type;

template <typename... Ts>
using tail = typename detail::tail_impl<Ts...>::type;

//=====[ list_size ]=====
template <class List>
struct list_size;

template <template<typename...> class List, typename... Ts>
struct list_size< List<Ts...> > {
    static constexpr size_t value = sizeof...(Ts);
};

template <class List>
inline constexpr size_t list_size_v = list_size<List>::value;


// =====[ rename ]=====
namespace detail {
    template <class From, template<typename...> class To>
    struct rename_impl;

    template<
        template<typename...> class List, typename... Ts,
        template<typename...> class To
    >
    struct rename_impl< List<Ts...>, To > {
        using type = To<Ts...>;
    };
}

template <class From, template<typename...> class To>
using rename = typename detail::rename_impl<From, To>::type;


// =====[ select ]=====
namespace detail {
    template <bool Condition, class ThenType, class ElseType>
    struct select_impl {
        using type = ElseType;
    };

    template <class ThenType, class ElseType>
    struct select_impl<true, ThenType, ElseType> {
        using type = ThenType;
    };
}

template <bool Cond, typename T, typename E>
using select = typename detail::select_impl<Cond, T, E>::type;

template <class Cond, typename T, typename E>
using select_if = typename detail::select_impl<bool(Cond::value), T,E>::type;


// =====[ defer ]=====
namespace detail {
    template <metafunc F, typename... Args>
    struct defer_impl {
        using eval = F<Args...>;
        using type = F<Args...>;
    };
}

template <metafunc F, typename... Args>
using defer = detail::defer_impl<F, Args...>;

// =====[ type_at ]=====
namespace detail {
    ///TODO: Find a better fallback solution
    template <size_t Index, typename... Ts>
    struct type_at_impl;

    template <size_t Index, typename T, typename... Ts>
    struct type_at_impl<Index,T,Ts...> : type_at_impl<Index-1, Ts...> {};
    
    template <typename T, typename... Ts>
    struct type_at_impl<0, T, Ts...> {
        using type = T;
    };
}

#if __has_builtin(__type_pack_element)
#   ifdef CORE_DEBUG_TYPE_PACK_ELEMENT
#       warning <!> "type_at" uses builtin __type_pack_element
#   endif
template <size_t Index, typename... Ts>
using type_at = __type_pack_element<Index, Ts...>;
#else 
#   ifdef CORE_DEBUG_TYPE_PACK_ELEMENT
#       warning <!> builtin __type_pack_element unavailable
#   endif
template <size_t Index, typename... Ts>
using type_at = typename detail::type_at_impl<Index, Ts...>::type;
#endif


// ===== [ find ] =====
namespace detail {
        
    template <typename X, typename... Ts>
    struct find_impl {};

    template <typename X, typename T, typename... Ts>
    struct find_impl<X, T,Ts...> {
        static constexpr size_t find(size_t index=0) noexcept {
            return find_impl<X, Ts...>::find(index+1);
        }
    };

    template <typename T, typename... Ts>
    struct find_impl<T, T,Ts...> {
        static constexpr size_t find(size_t index=0) noexcept {
            return index;
        }
    };

    template <typename X>
    struct find_impl<X> {
        static_assert(dependent_flag<X, false>::value, "Type not found!");
    };
}//detail

template <typename X, typename... Ts>
constexpr size_t find(size_t index=0) {
    return detail::find_impl<X, Ts...>::find(index);
}


// ===== [ try_find ] =====
namespace detail {
    template <typename X, typename... Ts>
    struct try_find_impl {};

    template <typename X, typename T, typename... Ts>
    struct try_find_impl<X, T,Ts...> {
        static constexpr std::optional<size_t> try_find(size_t index=0) noexcept {
            return try_find_impl<X, Ts...>::try_find(index+1);
        }
    };

    template <typename T, typename... Ts>
    struct try_find_impl<T, T,Ts...> {
        static constexpr std::optional<size_t> try_find(size_t index=0) noexcept {
            return {index};
        }
    };

    template <typename X>
    struct try_find_impl<X> {
        static constexpr std::optional<size_t> try_find(size_t=0) noexcept {
            return {};
        }
    };
}//detail

template <typename X, typename... Ts>
constexpr std::optional<size_t> try_find(size_t index=0) {
    return detail::try_find_impl<X, Ts...>::try_find(index);
}

// ===== [ try_find_meta (metafunc) ] =====
namespace detail {
    template <metafunc X, metafunc... Ts>
    struct try_find_meta_impl {};

    template <metafunc X, metafunc T, metafunc... Ts>
    struct try_find_meta_impl<X, T,Ts...> {
        static constexpr std::optional<size_t> try_find(size_t index=0) noexcept {
            return try_find_meta_impl<X, Ts...>::try_find(index+1);
        }
    };

    template <metafunc T, metafunc... Ts>
    struct try_find_meta_impl<T, T,Ts...> {
        static constexpr std::optional<size_t> try_find(size_t index=0) noexcept {
            return {index};
        }
    };

    template <metafunc X>
    struct try_find_meta_impl<X> {
        static constexpr std::optional<size_t> try_find(size_t=0) noexcept {
            return {};
        }
    };
}//detail

template <metafunc X, metafunc... Ts>
constexpr std::optional<size_t> try_find_meta(size_t index=0) {
    return detail::try_find_meta_impl<X, Ts...>::try_find(index);
}

// ===== [ list_at ] =====
namespace detail {
    template <class L, size_t Index>
    struct list_at_impl;

    template <size_t Index, metafunc List, typename... Ts>
    struct list_at_impl<List<Ts...>, Index> {
        using type = type_at<Index, Ts...>;
    };
}

template <class L, size_t Index>
using list_at = typename detail::list_at_impl<L, Index>::type;

// =====[ quote ]=====
template <metafunc F>
struct quote {
    template <typename... Args>
    using fn = F<Args...>;
};

// =====[ invoke ]=====
template <class Q, typename... Args>
using invoke = typename Q::template fn<Args...>;


// =====[ bind ]=====
template <metafunc F, typename... Ts>
struct bind_first {
    template <typename... Us>
    using with = F<Ts..., Us...>;

    template <typename... Us>
    using fn = F<Ts..., Us...>; // to support interoperability with other quoted metafunctions
};

template <metafunc F, typename T>
using bind = bind_first<F,T>;


// =====[ prepend ]=====
namespace detail {
    template <class List, typename... T>
    struct prepend_impl;

    template <
        template<typename...> class List, typename... Ts,
        typename... T
    >
    struct prepend_impl<List<Ts...>, T...> {
        using type = List<T...,Ts...>;
    };
}

template <class List, typename... T>
using prepend = typename detail::prepend_impl<List,T...>::type;


// =====[ append ]=====
namespace detail {
    template <class List, typename... T>
    struct append_impl;

    template <
        template<typename...> class List, typename... Ts,
        typename... T
    >
    struct append_impl<List<Ts...>, T...> {
        using type = List<Ts..., T...>;
    };
}

template <class List, typename... T>
using append = typename detail::append_impl<List,T...>::type;


// =====[ map ]=====
namespace detail {
    template <metafunc F, class T>
    struct apply_impl {
        using type = F<T>;
    };

    template <metafunc F, metafunc List, typename... Ts>
    struct apply_impl<F, List<Ts...>> {
        using type = F<Ts...>;
    };
}

template <metafunc F, class List>
using apply = typename detail::apply_impl<F, List>::type;


// =====[ filter_p (filter with predicate) ]=====
namespace detail {
    template <class List, class Predicate>
    struct filter_p_impl;

    template<metafunc List, typename... Ts, class Predicate>
    struct filter_p_impl<List<Ts...>, Predicate> {
        using type = select< 
            Predicate::template eval< head<Ts...> >(), 
            prepend< typename filter_p_impl<apply<List, tail<Ts...>>, Predicate>::type, head<Ts...> >,
            typename filter_p_impl<apply<List, tail<Ts...>>, Predicate>::type
        >;
    };

    template<metafunc List, class Predicate>
    struct filter_p_impl<List<>, Predicate> {
        using type = List<>;
    };
}

template<class List, class Predicate>
using filter_p = typename detail::filter_p_impl<List, Predicate>::type;


// =====[ transform ]=====
namespace detail {
    template <metafunc F, class List>
    struct transform_impl;

    template <metafunc F, metafunc List, typename... Ts>
    struct transform_impl< F, List<Ts...> >{
        using type = List< F<Ts>... >;
    };
}

template <metafunc F, class List>
using transform = typename detail::transform_impl<F,List>::type;

// =====[ transform_q (Quoted) ]=====
namespace detail {
    template <class Q, class List>
    struct transform_q_impl;

    template <class Q, metafunc List, typename... Ts>
    struct transform_q_impl< Q, List<Ts...> >{
        using type = List< invoke<Q, Ts...> >;
    };
}
template <class Q, class List>
using transform_q = typename detail::transform_q_impl<Q, List>::type;



// =====[ apply_transforms ]=====
namespace detail {
    template <class Args, metafunc... Fs>
    struct apply_transforms_impl;

    template <
        metafunc List, typename... Ts,
        metafunc... Fs
    >
    struct apply_transforms_impl<List<Ts...>, Fs...> {
        using type = List< Fs<Ts>... >;
    };
}

template <class Args, metafunc... Fs>
using apply_transforms = typename detail::apply_transforms_impl<Args,Fs...>::type;



// =====[ take ]=====
namespace detail {
    template <class List, size_t N, class Result>
    struct take_impl;

    template <
        metafunc List, typename T, typename... Ts, 
        metafunc Res, typename... Xs
    >
    struct take_impl<List<T, Ts...>, 0, Res<Xs...>> {
        using type = Res<Xs...>;
    };

    template <
        metafunc List, 
        size_t N, 
        metafunc Res, typename... Xs
    >
    struct take_impl<List<>, N, Res<Xs...>> {
        using type = Res<Xs...>;
    };

    template <
        metafunc List, typename T, typename... Ts,
        size_t N,
        metafunc Res, typename... Xs
    >
    struct take_impl<List<T, Ts...>, N, Res<Xs...>> : take_impl<List<Ts...>, N-1, Res<Xs..., T>> {};

}

template <size_t N, typename List, metafunc Res=typelist>
using take = typename detail::take_impl<List, N, Res<>>::type;

// =====[ drop ]=====
namespace detail {
    template <class List, size_t N>
    struct drop_impl;

    template <
        metafunc List, typename T, typename... Ts
    >
    struct drop_impl<List<T, Ts...>, 0> {
        using type = List<T,Ts...>;
    };

    template <metafunc List, size_t N>
    struct drop_impl<List<>, N> {
        using type = List<>;
    };

    template <
        metafunc List, typename T, typename... Ts,
        size_t N
    >
    struct drop_impl<List<T, Ts...>, N> : drop_impl<List<Ts...>, N-1> {};

}

template <size_t N, typename List>
using drop = typename detail::drop_impl<List, N>::type;


// =====[ zip ]=====
namespace detail {
    template <class List1, class List2>
    struct zip_impl;

    template <metafunc List1, typename... T1s, metafunc List2, typename... T2s>
    struct zip_impl <List1<T1s...>, List2<T2s...>>{
        using type = typelist< typelist<T1s, T2s>... >;
    };
}

template <class List1, class List2>
using zip = typename detail::zip_impl<List1, List2>::type;


// =====[ zip_with ]=====
namespace detail {
    template <class List1, class List2, metafunc F>
    struct zip_with_impl;

    template <metafunc List1, typename... T1s, metafunc List2, typename... T2s, metafunc F>
    struct zip_with_impl <List1<T1s...>, List2<T2s...>, F>{
        using type = List1< F<T1s, T2s>... >;
    };
}

template <class List1, class List2, metafunc F>
using zip_with = typename detail::zip_with_impl<List1, List2, F>::type;


// =====[ concat ]=====
namespace detail {
    template <class... Lists>
    struct concat_impl;

    template <
        metafunc L1, typename... T1s,
        metafunc L2, typename... T2s,
        class... Lists
    >
    struct concat_impl< L1<T1s...>, L2<T2s...>, Lists... >{
        using type = typename concat_impl< L1<T1s..., T2s...>, Lists... >::type;
    };

    template <metafunc List, typename... Ts>
    struct concat_impl< List<Ts...> >{
        using type = List<Ts...>;
    };

    template <>
    struct concat_impl<> {
        using type = typelist<>;
    };
}

template <class... Lists>
using concat = typename detail::concat_impl<Lists...>::type;


// =====[ repeat ]=====
namespace detail {
    template <size_t N, typename... Ts>
    struct repeat_impl {
        using l = typename repeat_impl<N/2, Ts...>::type;
        using r = typename repeat_impl<(N-N/2), Ts...>::type;
        using type = concat<l, r>;
    };

    template <typename... Ts>
    struct repeat_impl<0,Ts...> {
        using type = typelist<>;
    };

    template <typename... Ts>
    struct repeat_impl<1,Ts...> {
        using type = typelist<Ts...>;
    };
}

template <size_t N, typename... Ts>
using repeat = typename detail::repeat_impl<N,Ts...>::type;



// =====[ set_contains ]=====
namespace detail {
    template <class List, class Value>
    struct set_contains_impl;

    template <
        template <class...> class List,
        typename... Ts,
        class Value
    >
    struct set_contains_impl< List<Ts...>, Value > {
        struct L : identity<Ts>... {};
        using type = std::is_base_of<identity<Value>, L>;
    };
}

template <class List, class Value>
using set_contains = typename detail::set_contains_impl<List, Value>::type;


// =====[ make_set ]=====
namespace detail {

    template <class List, class Set>
    struct make_set_impl;

    template <
        template <class...> class List,
        template <class...> class Set,
        typename... Ts
    >
    struct make_set_impl< List<>, Set<Ts...> > {
        using type = Set<Ts...>;
    };

    template <
        template <class...> class List,
        typename T, typename... Ts,
        template <class...> class Set,
        typename... Us
    >
    struct make_set_impl< List<T, Ts...>, Set<Us...> > {
        using type = typename select< 
            set_contains<Set<Us...>, T>::value,
            make_set_impl<List<Ts...>, Set<Us...>>,
            make_set_impl<List<Ts...>, Set<Us..., T>>
        >::type;
    };
}

template <typename... Ts>
using set = typename detail::make_set_impl<typelist<Ts...>, typelist<>>::type;

template <class List, metafunc Set=typelist>
using make_set = typename detail::make_set_impl< List, Set<> >::type;


// =====[ contains ]=====
namespace detail {
    template <class List, typename X>
    struct contains_impl;

    template <metafunc List, typename T, typename... Ts,
              typename X>
    struct contains_impl<List<T,Ts...>, X> 
    : std::integral_constant<bool, std::is_same<T,X>::value || contains_impl<List<Ts...>,X>::value> {};


    template <metafunc List, typename X>
    struct contains_impl<List<>, X> : std::integral_constant<bool, false> {};
}

template <class List, typename X>
using contains = detail::contains_impl<List, X>;


// =====[ unpack ]=====
// unpack< Class<Types...> >::base|::arg<0>|...e.t.c
template <class C>
struct unpack;

template <template<typename...> class Base, typename... Ts>
struct unpack< Base<Ts...> >{

    template <typename... Us>
    using base = Base<Us...>;
    
    template <size_t I>
    using arg = type_at<I, Ts...>;
};

template <class C, typename... Ts>
using unpack_base = typename unpack<C>::template base<Ts...>;

template <class C, size_t I>
using unpack_arg = typename unpack<C>::template arg<I>;

// =====[ stl_rewire ]=====
/**
 * @brief rewires T parameter for most STL containers: enables doing so in a generic manner
 * @tparam C<T,...> STL Container
 * @tparam X New type to substitute into C, giving C<X,...>
 * @remark modifies all template-template-parameters like Allocator<> and Compare<> to take new type X
 * @returns -> C<X,...> also an STL Container
 */
template <class C, typename X>
struct stl_rewire {};

template <class C, typename X>
using stl_rewire_t = typename stl_rewire<C,X>::type;

namespace detail {
    template <class T, class>
    struct rewire_template : identity<T> {};

    template <template <typename> class C, typename T, typename NewT>
    struct rewire_template< C<T>, NewT >{
        using type = C<NewT>;
    };
}//namespace detail

template <template <typename...> class C, typename T, typename... Ts, typename To>
struct stl_rewire< C<T, Ts...>, To >{
    using type = C<To, typename detail::rewire_template<Ts, To>::type ...>;
};

template <template <typename, size_t> class C, typename T, size_t I, typename To>
struct stl_rewire< C<T, I>, To >{
    using type = C<To, I>;
};


// =====[ sum ]=====
constexpr size_t sum() noexcept { return 0; }

template <typename T, typename... Ts>
constexpr size_t sum(T v, Ts... vs) noexcept {
    return v + sum(vs...);
}


#if __cplusplus/100 >= 2014
// Any & All boolean funcs beta [bound to change]
// ALL
// template <bool... Vs>
// constexpr bool all() noexcept {
//     constexpr bool flag[] = {Vs...};
//     for (size_t i = 0; i < sizeof...(Vs); ++i) {
//         if (!flag[i]) { return false; }
//     }
//     return true;
// }

// =====[ ALL ]=====
constexpr bool all(std::initializer_list<bool> vs) noexcept {
    for (auto&& flag : vs) {
        if (!flag) { return false; }
    }
    return true;
}


namespace detail {
    template <typename T>
    struct all_impl;

    template <template<typename...> class C, typename... Ts>
    struct all_impl<C<Ts...>> {
        constexpr bool operator()() {
            constexpr bool values[] = {Ts::value...};
            for (auto const& v : values) {
                if (!v) return false;
            }
            return true;
        }
    };
}

template <typename List>
constexpr bool all() noexcept {
    return detail::all_impl<List>{}();
}


// =====[ ANY ]=====
template <bool... Vs>
constexpr bool any() noexcept {
    constexpr bool flag[] = {Vs...};
    for (size_t i = 0; i < sizeof...(Vs); ++i) {
        if (flag[i]) { return true; }
    }
    return false;
}

constexpr bool any(std::initializer_list<bool> vs) noexcept {
    for (auto&& flag : vs) {
        if (flag) { return true; }
    }
    return false;
}
#endif


#undef metafunc
} //namespace meta
