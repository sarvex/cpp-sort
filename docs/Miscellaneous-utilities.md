The following utilities are available in the directory `cpp-sort/utility` and live in the namespace `cppsort::utility`. While not always directly related to sorting, they might be useful to solve common problems when writing and/or using sorters or adapters.

### `adapter_storage`

```cpp
#include <cpp-sort/utility/adapter_storage.h>
```

`adapter_storage` is a wrapper type meant to be used to store a *sorter* in the internals of [*sorter adapter*][sorter-adapters] which adapts a single [*sorter*][sorters]. One of its goal is to remain an empty type when possible in order to play nice with the parts of the libraries that allow to cast empty sorters to function pointers. It provides three different operations:
* *Construction:* it is constructed with a copy of the wrapped sorter which it stores in its internals. If `Sorter` is empty and default-constructible, then `adapter_storage<Sorter>` is also empty and default-constructible.
* *Sorter access:* a `get()` member function returns a reference to the wrapped sorter with `const` and reference qualifications matching those of the `adapter_storage` instance. If `Sorter` is empty and default-constructible, then `get()` returns a default-constructed instance of `Sorter` instead.
* *Invocation:* it has a `const`-qualified `operator()` which forwards its parameter to the corresponding operator in the wrapped `Sorter` (or in a default-constructed instance of `Sorter`) and returns its result.

```cpp
template<typename Sorter>
struct adapter_storage;
```

The usual way to use it when implementing a *sorter adapter* is to make said adapter inherit from `adapter_storage<Sorter>` and to feed it a copy of the *original sorter*. Then either `get()` or `operator()` can be used to correctly call the wrapped sorter.

*New in version 1.5.0*

### `apply_permutation`

```cpp
#include <cpp-sort/utility/apply_permutation.h>
```

`apply_permutation` is a function template accepting a random-access range of elements and a random-access range of [0, N) indices of the same size. The indices in the second range represent the positions of the elements in the first range that should be moved in the indices positions to bring the collection in sorted order.

The algorithm requires both the elements range and the indices range to be mutable and modifies both.

```cpp
template<typename RandomAccessIterator1, typename RandomAccessIterator2>
auto apply_permutation(RandomAccessIterator1 first, RandomAccessIterator1 last,
                        RandomAccessIterator2 indices_first, RandomAccessIterator2 indices_last)
    -> void;

template<typename RandomAccessIterable1, typename RandomAccessIterable2>
auto apply_permutation(RandomAccessIterable1&& iterable, RandomAccessIterable2&& indices)
    -> void;
```

*New in version 1.14.0*

### `as_comparison` and `as_projection`

```cpp
#include <cpp-sort/utility/functional.h>
```

When given a [*Callable*][callable] that satisfies both the comparison and projection concepts, every sorter in **cpp-sort** considers that it is a comparison function (unless it is a projection-only sorter, in which case the function is considered to be a projection). To make such ambiguous callables usable as projections, the utility function `as_projection` is provided, which wraps a function and exposes only its unary overload. A similar `as_comparison` function is also provided in case one needs to explicitly expose the binary overload of a function.

The result of `as_projection` also inherits from `projection_base`, which makes it usable to [compose projections][chainable-projections] with `operator|`.

```cpp
template<typename Function>
constexpr auto as_projection(Function&& func)
    -> /* implementation-defined */;

template<typename Function>
constexpr auto as_comparison(Function&& func)
    -> /* implementation-defined */;
```

When the object passed to `as_comparison` or `as_projection` is a [*transparent function object*][transparent-func], then the object returned by those functions will also be *transparent*.

*Changed in version 1.7.0:* `as_comparison` and `as_projection` accept any *Callable*.

*Changed in version 1.7.0:* the object returned by `as_projection` inherits from `projection_base`.

*Changed in version 1.13.0:* the objects returned by `as_comparison` and `as_projection` are now conditionally [*transparent*][transparent-func].

### `as_function`

```cpp
#include <cpp-sort/utility/as_function.h>
```

`as_function` is an utility function borrowed from Eric Niebler's [Range-v3][range-v3] library. This function takes a parameter and does what it can to return an object callable with the usual function call syntax. It is notably useful to transform a pointer to member data into a function taking an instance of the class and returning the member.

To be more specific, `as_function` returns the passed object as is if it is already callable with the usual function call syntax, and uses [`std::mem_fn`][std-mem-fn] to wrap the passed object otherwise. It is worth noting that when the original object is returned, its potential to be called in a `constexpr` context remains the same, which makes `as_function` superior to [`std::invoke`][std-invoke] in this regard (prior to C++20).

```cpp
struct wrapper { int foo; };
// func is a function taking an instance of wrapper
// and returning the value of its member foo
auto&& func = cppsort::utility::as_function(&wrapper::foo);
```

### Branchless traits

```cpp
#include <cpp-sort/utility/branchless_traits.h>
```

Some of the library's algorithms ([`pdq_sorter`][pdq-sorter] and everything that relies on it, which means many things since it's the most common fallback for other algorithms) may use a different logic depending on whether the use comparison and projection functions are branchful and branchless. Unfortunately, it isn't something that can be deterministically determined; in order to provide the information, the following traits are available (they always inherit from either `std::true_type` or `std::false_type`):

```cpp
template<typename Compare, typename T>
struct is_probably_branchless_comparison;

template<typename Compare, typename T>
constexpr bool is_probably_branchless_comparison_v
    = is_probably_branchless_comparison<Compare, T>::value;
```

This trait tells whether the comparison function `Compare` is likely to generate branchless code when comparing two instances of `T`. By default it considers that the following comparison functions are likely to be branchless:
* [`std::less<>`][std-less-void], [`std::ranges::less`][std-ranges-less] and [`std::less<T>`][std-less] for any `T` satisfying [`std::is_arithmetic`][std-is-arithmetic]
* [`std::greater<>`][std-greater-void], [`std::ranges::greater`][std-ranges-greater] and [`std::greater<T>`][std-greater] for any `T` satisfying [`std::is_arithmetic`][std-is-arithmetic]

```cpp
template<typename Projection, typename T>
struct is_probably_branchless_projection;

template<typename Projection, typename T>
constexpr bool is_probably_branchless_projection_v
    = is_probably_branchless_projection<Projection, T>::value;
```

This trait tells whether the projection function `Projection` is likely to generate branchless code when called with an instance of `T`. By default it considers that the following projection functions are likely to be branchless:
* `cppsort::utility::identity` for any type
* [`std::identity`][std-identity] for any type (when available)
* Any type that satisfies [`std::is_member_function_pointer`][std-is-member-function-pointer] provided it is called with an instance of the appropriate class

These traits can be specialized for user-defined types. If one of the traits is specialized to consider that a user-defined type is likely to be branchless with a comparison/projection function, cv-qualified and reference-qualified versions of the same user-defined type will also be considered to produce branchless code when compared/projected with the same function.

*Changed in version 1.9.0:* conditional support for [`std::ranges::less`][std-ranges-less] and [`std::ranges::greater`][std-ranges-greater].

*Changed in version 1.9.0:* conditional support for [`std::identity`][std-identity].

### Buffer providers

```cpp
#include <cpp-sort/utility/buffer.h>
```

Buffer providers are classes designed to be passed to dedicated sorters; they describe how a the sorter should allocate a temporary buffer. Every buffer provider shall have a `buffer` inner class constructible with an `std::size_t`, and providing the methods `begin`, `end`, `cbegin`, `cend` and `size`.

```cpp
template<std::size_t N>
struct fixed_buffer;
```

This buffer provider allocates `N` elements on the stack. It uses [`std::array`][std-array] behind the scenes, so it also allows buffers of size 0. The runtime size passed to the buffer at construction is discarded.

```cpp
template<typename SizePolicy>
struct dynamic_buffer;
```

This buffer provider allocates on the heap a number of elements depending on a given *size policy* (a class whose `operator()` takes the size of the collection and returns another size). You can use the function objects from `utility/functional.h` as basic size policies. The buffer construction may throw an instance of [`std::bad_alloc`][std-bad-alloc] if it fails to allocate the required memory.

### Miscellaneous function objects

```cpp
#include <cpp-sort/utility/functional.h>
```

***WARNING:** `utility::identity` is removed in version 2.0.0, use `std::identity` instead.*

This header provides the class `projection_base` and the mechanism used to compose projections with `operator|`. See [Chainable projections][chainable-projections] for more information.

Also available in this header, the struct `identity` is a function object that can type any value of any movable type and return it as is. It is used as a default for every projection parameter in the library so that sorters view the values as they are by default, without a modification.

```cpp
struct identity:
    projection_base
{
    template<typename T>
    constexpr auto operator()(T&& t) const noexcept
        -> T&&;

    using is_transparent = /* implementation-defined */;
};
```

It is equivalent to the C++20 [`std::identity`][std-identity]. Wherever the documentation mentions special handling of `utility::identity`, the same support is provided for `std::identity` when it is available.

Another simple yet very handy projection available in the header is `indirect`: its instances can be called on any dereferenceable value `it`, in which case it returns `*it`. It is meant to be used as a projection with standard algorithms when iterating over a collection of iterators.

```cpp
struct indirect:
    projection_base
{
    template<typename T>
    constexpr auto operator()(T&& indirect_value)
        -> decltype(*std::forward<T>(indirect_value));

    template<typename T>
    constexpr auto operator()(T&& indirect_value) const
        -> decltype(*std::forward<T>(indirect_value));
};
```

This header also provides additional function objects implementing basic unary operations. These functions objects are designed to be used as *size policies* with `dynamic_buffer` and similar classes. The following function objects are available:
* `half`: returns the passed value divided by 2.
* `log`: returns the base 10 logarithm of the passed value.
* `sqrt`: returns the square root of the passed value.

All of those function objects inherit from `projection_base` and are [*transparent function objects*][transparent-func].

Since C++17, the following utility is also available when some level of micro-optimization is needed:

```cpp
template<auto Function>
struct function_constant
{
    using value_type = decltype(Function);

    static constexpr value_type value = Function;

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
        noexcept(noexcept(as_function(Function)(std::forward<Args>(args)...)))
        -> decltype(as_function(Function)(std::forward<Args>(args)...));

    constexpr operator value_type() const noexcept;
};
```

This utility is modeled after [`std::integral_constant`][std-integral-constant], but is different in that it takes its parameter as `template<auto>`, and `operator()` calls the wrapped value instead of returning it. The goal is to store function pointers and pointer to members "for free": they are only "stored" as a template parameter, which allows `function_constant` to be an empty class. This has two main advantages: `function_constant` can benefit from [*Empty Base Optimization*][ebo] since it weighs virtually nothing, and it won't need to be pushed on the stack when passed to a function, while the wrapped pointer would have been if passed unwrapped. Unless you are micro-optimizing some specific piece of code, you shouldn't need this class.

`is_probably_branchless_comparison` and `is_probably_branchless_projection` will correspond to `std::true_type` if the wrapped `Function` also gives `std::true_type`. Moreover, you can even specialize these traits for specific `function_constant` instanciations if you need even more performance.

*Warning: `function_constant` is only available since C++17.*

*New in version 1.7.0:* `projection_base` and chainable projections.

*Changed in version 1.9.0:* `std::identity` is now also supported wherever the library has special behavior for `utility::identity`.

*Changed in version 1.13.0:* `half`, `log` and `sqrt` are now [*transparent function objects*][transparent-func].

*New in version 1.14.0:* `indirect`.

### `iter_move` and `iter_swap`

```cpp
#include <cpp-sort/utility/iter_move.h>
```

***WARNING:** this header is removed in version 2.0.0, use `mstd::iter_move` and `mstd::iter_swap` instead.*

The functions `iter_move` and `iter_swap` are equivalent to the same functions as proposed by [P0022][p0022]: utility functions intended to be used with ADL to handle proxy iterators among other things. An algorithm can use them instead of `std::move` and possibly ADL-found `swap` to handle tricky classes such as `std::vector<bool>`.

The default implementation of `iter_move` simply move-returns the dereferenced iterator. `iter_swap` is a bit more tricky: if the iterators to be swapped have a custom `iter_move`, then `iter_swap` will use it, otherwise it will call an ADL-found `swap` or `std::swap` on the dereferenced iterators.

```cpp
template<typename Iterator>
constexpr auto iter_move(Iterator it)
    -> decltype(std::move(*it));

template<typename Iterator>
constexpr auto iter_swap(Iterator lhs, Iterator rhs)
    -> void;
```

*NOTE:* while both overloads are marked as `constexpr`, the generic version of `iter_swap` might use `std::swap`, which is not `constexpr` before C++20.

*Changed in version 1.10.0:* generic `iter_move` and `iter_swap` overloads are now marked as `constexpr`.

### `make_integer_range`

```cpp
#include <cpp-sort/utility/make_integer_range.h>
```

***WARNING:** `make_integer_range` and `make_index_range` are deprecated in version 1.8.0 and removed in version 2.0.0.*

The class template `make_integer_range` can be used wherever an [`std::integer_sequence`][std-integer-sequence] can be used. An `integer_range` takes a type template parameter that shall be an integer type, then three integer template parameters which correspond to the beginning of the range, the end of the range and the « step ».

```cpp
template<
    typename Integer,
    Integer Begin,
    Integer End,
    Integer Step = 1
>
using make_integer_range = /* implementation-defined */;
```

`make_index_range` is a specialization of `integer_range` for `std::size_t`.

```cpp
template<
    std::size_t Begin,
    std::size_t End,
    std::size_t Step = 1u
>
using make_index_range = make_integer_range<std::size_t, Begin, End, Step>;
```

### `size`

```cpp
#include <cpp-sort/utility/size.h>
```

***WARNING:** this header is removed in version 2.0.0, use `mstd::distance` instead.*

`size` is a function that can be used to get the size of an iterable. It is equivalent to the C++17 function [`std::size`][std-size] but has an additional tweak so that, if the iterable is not a fixed-size C array and doesn't have a `size` method, it calls `std::distance(std::begin(iter), std::end(iter))` on the iterable. Therefore, this function can also be used for `std::forward_list` as well as some implementations of ranges.

*Changed in version 1.12.1:* `utility::size()` now also works for collections that only provide non-`const` `begin()` and `end()`.

### `sorted_indices`

```cpp
#include <cpp-sort/utility/sorted_indices.h>
```

`utility::sorted_indices` is a function object that takes a sorter and returns a new function object that follows the *unified sorting interface*. This new function object accepts a random-access collection and returns an `std::vector` containing the indices that would sort that collection (similarly to [`numpy.argsort`][numpy-argsort]). The resulting indices can be passed [`apply_permutation`][apply-permutation] to sort the original collection.

```cpp
std::vector<int> vec = { 6, 4, 2, 1, 8, 7, 0, 9, 5, 3 };
auto get_sorted_indices_for = cppsort::utility::sorted_indices<cppsort::heap_sorter>{};
auto indices = get_sorted_indices_for(vec);
// indices == [6, 3, 2, 9, 1, 8, 0, 5, 4, 7]
```

When the collection contains *equivalent elements*, the order of their indices in the result depends on the sorter being used. However that order should be consistent across all stable sorters. `sorted_indices` follows the [`is_stable` protocol][is-stable], so the trait can be used to check whether the indices of *equivalent elements* appear in a stable order in the result.

*New in version 1.14.0*

### `sorted_iterators`

```cpp
#include <cpp-sort/utility/sorted_iterators.h>
```

`utility::sorted_iterators` is a function object that takes a sorter and returns a new function object that follows the *unified sorting interface*. This new function object accepts a collection and returns an `std::vector` containing iterators to the passed collection in a sorted order.

```cpp
std::list<int> li = { 6, 4, 2, 1, 8, 7, 0, 9, 5, 3 };
auto get_sorted_iterators_for = cppsort::utility::sorted_iterators<cppsort::heap_sorter>{};
const auto iterators = get_sorted_iterators_for(li);

// Displays 0 1 2 3 4 5 6 7 8 9
for (auto it: iterators) {
    std::cout << *it << ' ';
}
```

It can be thought of as a kind of sorted view of the passed collection - as long as said collection does not change. It can be useful when the order of the original collection must be preserved, but operations have to be performed on the sorted collection.

When the collection contains *equivalent elements*, the order of the corresponding iterators in the result depends on the sorter being used. However that order should be consistent across all stable sorters. `sorted_iterators` follows the [`is_stable` protocol][is-stable], so the trait can be used to check whether the iterators to *equivalent elements* appear in a stable order in the result.

*New in version 1.14.0*

### Sorting network tools

```cpp
#include <cpp-sort/utility/sorting_network.h>
```

Some of the library's [*fixed-size sorters*][fixed-size-sorters] implement [sorting networks][sorting-network]. It is a subdomain of sorting that has seen extensive research and there is no way all of the interesting bits could be provided as mere sorters; therefore the following tools are provided specifically to experiment with sorting networks, and with comparator networks more generally.

All comparator networks execute a fixed sequence of compare-exchanges, operations that compare two elements and exchange them if they are out-of-order. The following `index_pair` class template represents a pair of indices meant to contain the indices used to perform a compare-exchange operation:

```cpp
template<typename IndexType>
struct index_pair
{
    IndexType first, second;
};
```

This pretty rough template acts as the base vocabulary type for comparator networks. *Fixed-size sorters* that happen to be sorting networks should provide an `index_pairs()` static methods returning a sequence of `index_pair` sufficient to sort a collection of a given size.

The following functions accept a sequence of `index_pair` and swap the elements of a given random-access collection (represented by an iterator to its first element) according to the index pairs:

```cpp
template<
    typename RandomAccessIterator,
    typename IndexType,
    std::size_t N,
    typename Compare = std::less<>,
    typename Projection = utility::identity
>
auto swap_index_pairs(RandomAccessIterator first, const std::array<index_pair<IndexType>, N>& index_pairs,
                      Compare compare={}, Projection projection={})
    -> void;

template<
    typename RandomAccessIterator,
    typename IndexType,
    std::size_t N,
    typename Compare = std::less<>,
    typename Projection = utility::identity
>
auto swap_index_pairs_force_unroll(RandomAccessIterator first,
                                   const std::array<index_pair<IndexType>, N>& index_pairs,
                                   Compare compare={}, Projection projection={})
    -> void;
```

`swap_index_pairs` loops over the index pairs in the simplest fashion and calls the compare-exchange operations in the simplest possible way. `swap_index_pairs_force_unroll` is a best effort function trying to achieve the same job by unrolling the loop over indices the best it can - a perfect unrolling is thus attempted, but never guaranteed, which might or might result in faster runtime and/or increased binary size.

*New in version 1.11.0*

### `static_const`

```cpp
#include <cpp-sort/utility/static_const.h>
```

***WARNING:** `utility::static_const` is removed in version 2.0.0, use [`inline` variables][inline-variables] instead.*

`static_const` is a tiny utility used to instantiate function objects (for example sorters) and expose a single global instance to users of the library while avoiding ODR problems. It is taken straight from [Range-v3][range-v3]. The general pattern to instantiate function objects is as follows:

```cpp
namespace
{
    constexpr auto&& awesome_sort
        = cppsort::utility::static_const<awesome_sorter>::value;
}
```

You can read more about this instantiation pattern in [this article][eric-niebler-static-const] by Eric Niebler.


  [apply-permutation]: Miscellaneous-utilities.md#apply_permutation
  [chainable-projections]: Chainable-projections.md
  [callable]: https://en.cppreference.com/w/cpp/named_req/Callable
  [ebo]: https://en.cppreference.com/w/cpp/language/ebo
  [eric-niebler-static-const]: https://ericniebler.com/2014/10/21/customization-point-design-in-c11-and-beyond/
  [fixed-size-sorters]: Fixed-size-sorters.md
  [inline-variables]: https://en.cppreference.com/w/cpp/language/inline
  [is-stable]: Sorter-traits.md#is_stable
  [numpy-argsort]: https://numpy.org/doc/stable/reference/generated/numpy.argsort.html
  [p0022]: https://wg21.link/P0022
  [pdq-sorter]: Sorters.md#pdq_sorter
  [range-v3]: https://github.com/ericniebler/range-v3
  [sorter-adapters]: Sorter-adapters.md
  [sorters]: Sorters.md
  [sorting-network]: https://en.wikipedia.org/wiki/Sorting_network
  [std-array]: https://en.cppreference.com/w/cpp/container/array
  [std-bad-alloc]: https://en.cppreference.com/w/cpp/memory/new/bad_alloc
  [std-greater]: https://en.cppreference.com/w/cpp/utility/functional/greater
  [std-greater-void]: https://en.cppreference.com/w/cpp/utility/functional/greater_void
  [std-identity]: https://en.cppreference.com/w/cpp/utility/functional/identity
  [std-integer-sequence]: https://en.cppreference.com/w/cpp/utility/integer_sequence
  [std-integral-constant]: https://en.cppreference.com/w/cpp/types/integral_constant
  [std-invoke]: https://en.cppreference.com/w/cpp/utility/functional/invoke
  [std-is-arithmetic]: https://en.cppreference.com/w/cpp/types/is_arithmetic
  [std-is-member-function-pointer]: https://en.cppreference.com/w/cpp/types/is_member_function_pointer
  [std-less]: https://en.cppreference.com/w/cpp/utility/functional/less
  [std-less-void]: https://en.cppreference.com/w/cpp/utility/functional/less_void
  [std-mem-fn]: https://en.cppreference.com/w/cpp/utility/functional/mem_fn
  [std-ranges-greater]: https://en.cppreference.com/w/cpp/utility/functional/ranges/greater
  [std-ranges-less]: https://en.cppreference.com/w/cpp/utility/functional/ranges/less
  [std-size]: https://en.cppreference.com/w/cpp/iterator/size
  [transparent-func]: Comparators-and-projections.md#Transparent-function-objects
