/*
 * Copyright (c) 2015-2022 Morwenn
 * SPDX-License-Identifier: MIT
 */
#ifndef CPPSORT_SORTERS_QUICK_SORTER_H_
#define CPPSORT_SORTERS_QUICK_SORTER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>
#include <cpp-sort/mstd/iterator.h>
#include <cpp-sort/mstd/ranges.h>
#include <cpp-sort/mstd/type_traits.h>
#include <cpp-sort/sorter_facade.h>
#include <cpp-sort/sorter_traits.h>
#include "../detail/quicksort.h"

namespace cppsort
{
    ////////////////////////////////////////////////////////////
    // Sorter

    namespace detail
    {
        struct quick_sorter_impl
        {
            template<
                mstd::forward_range Range,
                typename Compare = std::less<>,
                typename Projection = std::identity,
                typename = mstd::enable_if_t<
                    is_projection_v<Projection, Range, Compare>
                >
            >
            constexpr auto operator()(Range&& range, Compare compare={}, Projection projection={}) const
                -> void
            {
                auto first = mstd::begin(range);
                auto last = mstd::end(range);
                auto last_it = mstd::next(first, std::move(last));
                quicksort(std::move(first), std::move(last_it),
                          mstd::distance(range),
                          std::move(compare), std::move(projection));
            }

            template<
                mstd::forward_iterator Iterator,
                mstd::sentinel_for<Iterator> Sentinel,
                typename Compare = std::less<>,
                typename Projection = std::identity,
                typename = mstd::enable_if_t<
                    is_projection_iterator_v<Projection, Iterator, Compare>
                >
            >
            constexpr auto operator()(Iterator first, Sentinel last,
                                      Compare compare={}, Projection projection={}) const
                -> void
            {
                auto last_it = mstd::next(first, std::move(last));
                auto dist = mstd::distance(first, last);
                quicksort(std::move(first), std::move(last_it), dist,
                          std::move(compare), std::move(projection));
            }

            ////////////////////////////////////////////////////////////
            // Sorter traits

            using iterator_category = std::forward_iterator_tag;
            using is_always_stable = std::false_type;
        };
    }

    struct quick_sorter:
        sorter_facade<detail::quick_sorter_impl>
    {};

    ////////////////////////////////////////////////////////////
    // Sort function

    inline constexpr quick_sorter quick_sort{};
}

#endif // CPPSORT_SORTERS_QUICK_SORTER_H_
