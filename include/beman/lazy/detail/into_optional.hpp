// include/beman/lazy/detail/into_optional.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_INTO_OPTIONAL
#define INCLUDED_BEMAN_LAZY_DETAIL_INTO_OPTIONAL

#include <optional>
#include <tuple>
#include <beman/execution26/execution.hpp>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {

inline constexpr struct into_optional_t
    : beman::execution26::sender_adaptor_closure<into_optional_t>
{
    template <::beman::execution26::sender Upstream>
    struct sender
    {
        using upstream_t = std::remove_cvref_t<Upstream>;
        using sender_concept = ::beman::execution26::sender_t;
        upstream_t upstream;

        template <typename...> struct type_list {};

        template <typename T>
        static auto find_type(type_list<type_list<T>>) { return std::optional<T>{}; }
        template <typename T>
        static auto find_type(type_list<type_list<T>, type_list<>>) { return std::optional<T>{}; }
        template <typename T>
        static auto find_type(type_list<type_list<>, type_list<T>>) { return std::optional<T>{}; }
        template <typename T0, typename T1, typename... T>
        static auto find_type(type_list<type_list<T0, T1, T...>>) { return std::optional<std::tuple<T0, T1, T...>>{}; }
        template <typename T0, typename T1, typename... T>
        static auto find_type(type_list<type_list<T0, T1, T...>, type_list<>>) { return std::optional<std::tuple<T0, T1, T...>>{}; }
        template <typename T0, typename T1, typename... T>
        static auto find_type(type_list<type_list<>, type_list<T0, T1, T...>>) { return std::optional<std::tuple<T0, T1, T...>>{}; }

        template <typename Env>
        auto get_type(Env&&) const {
            return find_type(::beman::execution26::value_types_of_t<Upstream, std::remove_cvref_t<Env>, type_list, type_list>());
        }

        template <typename... E, typename... S>
        constexpr auto make_signatures(auto&& env, type_list<E...>, type_list<S...>) const {
            return ::beman::execution26::completion_signatures<
                ::beman::execution26::set_value_t(decltype(this->get_type(env))),
                ::beman::execution26::set_error_t(E)...,
                S...
                >();
        }
        template<typename Env>
        auto get_completion_signatures(Env&& env) const {
            return make_signatures(env,
                                   ::beman::execution26::error_types_of_t<Upstream, std::remove_cvref_t<Env>, type_list>{},
                                   std::conditional_t<
                                        ::beman::execution26::sends_stopped<Upstream, std::remove_cvref_t<Env>>,
                                        type_list<::beman::execution26::set_stopped_t()>,
                                        type_list<>>{}
                                   );
        }

        template <typename Receiver>
        auto connect(Receiver&& receiver) && {
            return ::beman::execution26::connect(
                ::beman::execution26::then(std::move(this->upstream),
                    []<typename...A>(A&&... a)->decltype(get_type(::beman::execution26::get_env(receiver))) {
                        if constexpr (sizeof...(A) == 0u)
                            return {};
                        else
                            return {std::forward<A>(a)...};
                }),
                std::forward<Receiver>(receiver)
            );
        }
    };

    template <typename Upstream>
    sender<Upstream> operator()(Upstream&& upstream) const { return {std::forward<Upstream>(upstream)}; }
} into_optional{};
}

// ----------------------------------------------------------------------------

#endif
