/*
 * Copyright (c) 2018-2021 Morwenn
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
#include <cpp-sort/sorter_facade.h>
#include <cpp-sort/sorter_traits.h>
#include <cpp-sort/utility/functional.h>
#include <cpp-sort/utility/size.h>
#include <cpp-sort/utility/static_const.h>
#include "../detail/iterator_traits.h"
#include "../detail/melsort.h"
#include "../detail/type_traits.h"

namespace cppsort
{
    ////////////////////////////////////////////////////////////
    // Sorter

    namespace detail
    {
        struct mel_sorter_impl
        {
            template<
                typename ForwardIterable,
                typename Compare = std::less<>,
                typename Projection = utility::identity,
                typename = detail::enable_if_t<
                    is_projection_v<Projection, ForwardIterable, Compare>
                >
            >
            auto operator()(ForwardIterable&& iterable,
                            Compare compare={}, Projection projection={}) const
                -> void
            {
                static_assert(
                    std::is_base_of<
                        std::forward_iterator_tag,
                        iterator_category_t<decltype(std::begin(iterable))>
                    >::value,
                    "mel_sorter requires at least forward iterators"
                );

                melsort(std::begin(iterable), std::end(iterable),
                        utility::size(iterable),
                        std::move(compare), std::move(projection));
            }

            template<
                typename ForwardIterator,
                typename Compare = std::less<>,
                typename Projection = utility::identity,
                typename = detail::enable_if_t<
                    is_projection_iterator_v<Projection, ForwardIterator, Compare>
                >
            >
            auto operator()(ForwardIterator first, ForwardIterator last,
                            Compare compare={}, Projection projection={}) const
                -> void
            {
                static_assert(
                    std::is_base_of<
                        std::forward_iterator_tag,
                        iterator_category_t<ForwardIterator>
                    >::value,
                    "mel_sorter requires at least forward iterators"
                );

                auto dist = std::distance(first, last);
                melsort(std::move(first), std::move(last), dist,
                        std::move(compare), std::move(projection));
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

    namespace
    {
        constexpr auto&& mel_sort
            = utility::static_const<mel_sorter>::value;
    }
}

#ifdef CPPSORT_ADAPTERS_CONTAINER_AWARE_ADAPTER_DONE_
#include "../detail/container_aware/mel_sort.h"
#endif

#endif // CPPSORT_SORTERS_MEL_SORTER_H_
