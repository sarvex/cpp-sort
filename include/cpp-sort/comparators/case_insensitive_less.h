/*
 * Copyright (c) 2016-2022 Morwenn
 * SPDX-License-Identifier: MIT
 */
#ifndef CPPSORT_COMPARATORS_CASE_INSENSITIVE_LESS_H_
#define CPPSORT_COMPARATORS_CASE_INSENSITIVE_LESS_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <algorithm>
#include <iterator>
#include <locale>
#include <type_traits>
#include <utility>
#include <cpp-sort/mstd/ranges.h>
#include "../detail/type_traits.h"

namespace cppsort
{
    namespace detail
    {
        ////////////////////////////////////////////////////////////
        // Case insensitive comparison for char sequences

        template<typename CharT>
        struct char_less
        {
            const std::ctype<CharT>& ct;

            char_less(const std::ctype<CharT>& ct):
                ct(ct)
            {}

            auto operator()(CharT lhs, CharT rhs) const
                -> bool
            {
              return ct.tolower(lhs) < ct.tolower(rhs);
            }
        };

        template<typename T>
        auto case_insensitive_less(const T& lhs, const T& rhs, const std::locale& loc)
            -> bool
        {
            using char_type = std::remove_cvref_t<mstd::range_reference_t<T>>;
            const auto& ct = std::use_facet<std::ctype<char_type>>(loc);

            return std::lexicographical_compare(mstd::begin(lhs), mstd::end(lhs),
                                                mstd::begin(rhs), mstd::end(rhs),
                                                char_less<char_type>(ct));
        }

        template<typename T>
        auto case_insensitive_less(const T& lhs, const T& rhs)
            -> bool
        {
            std::locale loc;
            return case_insensitive_less(lhs, rhs, loc);
        }

        ////////////////////////////////////////////////////////////
        // Customization point

        struct case_insensitive_less_fn;
        struct case_insensitive_less_locale_fn;
        namespace adl_barrier
        {
            template<typename T>
            struct refined_case_insensitive_less_fn;
            template<typename T>
            struct refined_case_insensitive_less_locale_fn;
        }

        template<typename T>
        inline constexpr bool can_be_refined_for
            = requires (T& range) {
                typename mstd::range_reference_t<T>;
            };

        struct case_insensitive_less_locale_fn
        {
            private:

                std::locale loc;

            public:

                explicit case_insensitive_less_locale_fn(const std::locale& loc):
                    loc(loc)
                {}

                template<typename T, typename U>
                constexpr auto operator()(T&& lhs, U&& rhs) const
                    noexcept(noexcept(case_insensitive_less(std::forward<T>(lhs), std::forward<U>(rhs), loc)))
                    -> decltype(case_insensitive_less(std::forward<T>(lhs), std::forward<U>(rhs), loc))
                {
                    return case_insensitive_less(std::forward<T>(lhs), std::forward<U>(rhs), loc);
                }

                template<typename T>
                    requires can_be_refined_for<T>
                auto refine() const
                    -> adl_barrier::refined_case_insensitive_less_locale_fn<T>
                {
                    return adl_barrier::refined_case_insensitive_less_locale_fn<T>(loc);
                }

                using is_transparent = void;
        };

        struct case_insensitive_less_fn
        {
            template<typename T, typename U>
            constexpr auto operator()(T&& lhs, U&& rhs) const
                noexcept(noexcept(case_insensitive_less(std::forward<T>(lhs), std::forward<U>(rhs))))
                -> decltype(case_insensitive_less(std::forward<T>(lhs), std::forward<U>(rhs)))
            {
                return case_insensitive_less(std::forward<T>(lhs), std::forward<U>(rhs));
            }

            template<typename T>
                requires can_be_refined_for<T>
            auto refine() const
                -> adl_barrier::refined_case_insensitive_less_fn<T>
            {
                return adl_barrier::refined_case_insensitive_less_fn<T>();
            }

            inline auto operator()(const std::locale& loc) const
                -> case_insensitive_less_locale_fn
            {
                return case_insensitive_less_locale_fn(loc);
            }

            using is_transparent = void;
        };

        namespace adl_barrier
        {
            // Hide the generic case_insensitive_less from the enclosing namespace
            struct nope_type {};
            template<typename T>
            auto case_insensitive_less(const T& lhs, const T& rhs, const std::locale& loc)
                -> nope_type;
            template<typename T>
            auto case_insensitive_less(const T& lhs, const T& rhs)
                -> nope_type;

            // It makes is_invocable easier to work with
            struct caller
            {
                template<typename T>
                auto operator()(const T& lhs, const T& rhs, const std::locale& loc) const
                    -> decltype(case_insensitive_less(lhs, rhs, loc));

                template<typename T>
                auto operator()(const T& lhs, const T& rhs) const
                    -> decltype(case_insensitive_less(lhs, rhs));
            };

            template<typename T>
            struct refined_case_insensitive_less_locale_fn
            {
                private:

                    using char_type = std::remove_cvref_t<mstd::range_reference_t<T>>;

                    std::locale loc;
                    const std::ctype<char_type>& ct;

                public:

                    explicit refined_case_insensitive_less_locale_fn(const std::locale& loc):
                        loc(loc),
                        ct(std::use_facet<std::ctype<char_type>>(loc))
                    {}

                    template<typename U=T>
                        requires (not std::is_invocable_r_v<nope_type, caller, U, U, std::locale>)
                    auto operator()(const T& lhs, const T& rhs) const
                        -> decltype(case_insensitive_less(lhs, rhs, loc))
                    {
                        return case_insensitive_less(lhs, rhs, loc);
                    }

                    template<typename U=T>
                        requires std::is_invocable_r_v<nope_type, caller, U, U, std::locale>
                    auto operator()(const T& lhs, const T& rhs) const
                        -> bool
                    {
                        return std::lexicographical_compare(mstd::begin(lhs), mstd::end(lhs),
                                                            mstd::begin(rhs), mstd::end(rhs),
                                                            char_less<char_type>(ct));
                    }
            };

            template<typename T>
            struct refined_case_insensitive_less_fn
            {
                private:

                    using char_type = std::remove_cvref_t<mstd::range_reference_t<T>>;

                    std::locale loc;
                    const std::ctype<char_type>& ct;

                public:

                    refined_case_insensitive_less_fn():
                        loc(),
                        ct(std::use_facet<std::ctype<char_type>>(loc))
                    {}

                    template<typename U=T>
                        requires (not std::is_invocable_r_v<nope_type, caller, U, U>)
                    auto operator()(const T& lhs, const T& rhs) const
                        -> decltype(case_insensitive_less(lhs, rhs))
                    {
                        return case_insensitive_less(lhs, rhs);
                    }

                    template<typename U=T>
                        requires (
                            std::is_invocable_r_v<nope_type, caller, U, U> &&
                            not std::is_invocable_r_v<nope_type, caller, U, U, std::locale>
                        )
                    auto operator()(const T& lhs, const T& rhs) const
                        -> decltype(case_insensitive_less(lhs, rhs, loc))
                    {
                        return case_insensitive_less(lhs, rhs, loc);
                    }

                    template<typename U=T>
                        requires (
                            std::is_invocable_r_v<nope_type, caller, U, U> &&
                            std::is_invocable_r_v<nope_type, caller, U, U, std::locale>
                        )
                    auto operator()(const T& lhs, const T& rhs) const
                        -> bool
                    {
                        return std::lexicographical_compare(mstd::begin(lhs), mstd::end(lhs),
                                                            mstd::begin(rhs), mstd::end(rhs),
                                                            char_less<char_type>(ct));
                    }

                    auto operator()(const std::locale& loc) const
                        -> refined_case_insensitive_less_locale_fn<T>
                    {
                        return refined_case_insensitive_less_locale_fn<T>(loc);
                    }
            };
        }
    }

    using case_insensitive_less_t = detail::case_insensitive_less_fn;
    inline case_insensitive_less_t case_insensitive_less{};
}

#endif // CPPSORT_COMPARATORS_CASE_INSENSITIVE_LESS_H_
