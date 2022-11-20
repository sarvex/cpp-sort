/*
 * Copyright (c) 2018-2022 Morwenn
 * SPDX-License-Identifier: MIT
 */
#ifndef CPPSORT_SORTERS_MEL_SORTER_H_
#define CPPSORT_SORTERS_MEL_SORTER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>
#include <cpp-sort/mstd/iterator.h>
#include <cpp-sort/mstd/ranges.h>
#include <cpp-sort/sorter_facade.h>
#include <cpp-sort/sorter_traits.h>
#include "../detail/iterator_traits.h"
#include "../detail/melsort.h"

namespace cppsort
{
    ////////////////////////////////////////////////////////////
    // Sorter

    namespace detail
    {
        struct mel_sorter_impl
        {
            template<
                mstd::forward_range Range,
                typename Compare = std::less<>,
                typename Projection = std::identity
            >
                requires is_projection_v<Projection, Range, Compare>
            auto operator()(Range&& range, Compare compare={}, Projection projection={}) const
                -> mstd::iterator_t<Range>
            {
                auto first = mstd::begin(range);
                auto last = mstd::end(range);
                auto last_it = mstd::next(first, std::move(last));
                melsort(std::move(first), last_it,
                        mstd::distance(range),
                        std::move(compare), std::move(projection));
                return last_it;
            }

            template<
                mstd::forward_iterator Iterator,
                mstd::sentinel_for<Iterator> Sentinel,
                typename Compare = std::less<>,
                typename Projection = std::identity
            >
                requires is_projection_iterator_v<Projection, Iterator, Compare>
            auto operator()(Iterator first, Sentinel last,
                            Compare compare={}, Projection projection={}) const
                -> Iterator
            {
                auto dist = mstd::distance(first, last);
                auto last_it = mstd::next(first, std::move(last));
                melsort(std::move(first), last_it, dist,
                        std::move(compare), std::move(projection));
                return last_it;
            }

            ////////////////////////////////////////////////////////////
            // Sorter traits

            using iterator_category = std::forward_iterator_tag;
            using is_always_stable = std::false_type;
        };
    }

    struct mel_sorter:
        sorter_facade<detail::mel_sorter_impl>
    {};

    ////////////////////////////////////////////////////////////
    // Sort function

    inline constexpr mel_sorter mel_sort{};
}

#ifdef CPPSORT_ADAPTERS_CONTAINER_AWARE_ADAPTER_DONE_
#include "../detail/container_aware/mel_sort.h"
#endif

#endif // CPPSORT_SORTERS_MEL_SORTER_H_
