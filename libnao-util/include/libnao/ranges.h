#pragma once

#include <ranges>

namespace nao {
    namespace impl {
        template <bool Const, std::ranges::input_range R, std::copy_constructible Fn>
        class transform_iterator {
            public:
            using base_iterator = std::ranges::iterator_t<std::conditional_t<Const, const R, R>>;
            using iterator_concept = typename base_iterator::iterator_concept;
            using iterator_category = typename base_iterator::iterator_category;
            using value_type = decltype(Fn{}(*base_iterator{}));
            using difference_type = typename base_iterator::difference_type;
            using pointer = value_type*;
            using reference = value_type&;

            private:
            base_iterator _it{};
            Fn _fun;
            public:
            constexpr transform_iterator() = default;
            constexpr transform_iterator(base_iterator pos, Fn fun) : _it{ pos }, _fun{ fun } { }


            constexpr transform_iterator(transform_iterator&& other) noexcept {
                *this = std::forward<transform_iterator>(other);
            }

            constexpr transform_iterator& operator=(transform_iterator&& other) noexcept {
                _it = std::move(other._it);
                _fun = std::move(other._fun);
                return *this;
            }


            constexpr transform_iterator(const transform_iterator& other)
                : _it{ other._it }, _fun{ other._fun } { }

            constexpr transform_iterator& operator=(const transform_iterator& other) {
                _it = other._it;
                _fun = other._fun;
                return *this;
            }

            constexpr decltype(auto) operator*() const {
                return _fun(*_it);
            }


            constexpr decltype(auto) operator++() {
                ++_it;

                return *this;
            }


            constexpr decltype(auto) operator++(int) {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }


            constexpr decltype(auto) operator--() {
                --_it;

                return *this;
            }


            constexpr decltype(auto) operator--(int) {
                auto tmp = *this;
                --(*this);
                return tmp;
            }


            [[nodiscard]] friend constexpr std::strong_ordering operator<=>(
                const transform_iterator& l, const transform_iterator& r) {
                return l._it <=> r._it;
            }

            [[nodiscard]] friend constexpr bool operator==(const transform_iterator& left, const transform_iterator& right) {
                return left._it == right._it;
            }
        };
    }

    template <std::ranges::input_range R, std::copy_constructible Fn>
    class the_transform : public std::ranges::view_interface<the_transform<R, Fn>> {
        R _range{};
        Fn _fun;
        public:
        constexpr the_transform(R range, Fn fun) : _range{ std::move(range) }, _fun{ std::move(fun) } {

        }


        constexpr auto begin() const requires std::ranges::range<const R>
            && std::regular_invocable<const Fn&, std::ranges::range_reference_t<const R>> {
            return impl::transform_iterator<true, R, Fn>{ std::ranges::begin(_range), _fun };
        }


        constexpr auto begin() {
            return impl::transform_iterator<false, R, Fn>{ std::ranges::begin(_range), _fun };
        }


        constexpr auto end() const requires std::ranges::range<const R>
            && std::regular_invocable<const Fn&, std::ranges::range_reference_t<const R>> {
            return impl::transform_iterator<true, R, Fn>{ std::ranges::end(_range), _fun };
        }


        constexpr auto end() {
            return impl::transform_iterator<false, R, Fn>{ std::ranges::end(_range), _fun };
        }
    };

    class transform_fun {
        public:
        template <std::copy_constructible Fn>
        struct _partial {
            Fn fun;

            template <typename R>
            [[nodiscard]] friend constexpr auto operator|(R&& r, const _partial& t) {
                return the_transform{ std::forward<R>(r), std::move(t.fun) };
            }
        };

        public:
        template <std::ranges::viewable_range R, std::copy_constructible Fn>
        [[nodiscard]] constexpr auto operator()(R&& range, Fn fun) {
            return the_transform{ std::forward<R>(range), std::move(fun) };
        }

        template <std::copy_constructible Fn>
        [[nodiscard]] constexpr auto operator()(Fn fun) const {
            return _partial<Fn>{ .fun = fun };
        }
    };

    inline transform_fun my_transform;
}

namespace std {
    template <bool Const, ranges::input_range R, std::copy_constructible Fn>
    struct iterator_traits<nao::impl::transform_iterator<Const, R, Fn>> {
        using difference_type = typename nao::impl::transform_iterator<Const, R, Fn>::difference_type;
        using value_type = typename nao::impl::transform_iterator<Const, R, Fn>::value_type;
        using pointer = typename nao::impl::transform_iterator<Const, R, Fn>::pointer;
        using reference = typename nao::impl::transform_iterator<Const, R, Fn>::reference;
        using iterator_category = typename nao::impl::transform_iterator<Const, R, Fn>::iterator_category;
    };

    namespace ranges {
        template <input_range R, copy_constructible Fn>
        inline constexpr bool enable_borrowed_range<nao::the_transform<R, Fn>> = true;
    }
}
