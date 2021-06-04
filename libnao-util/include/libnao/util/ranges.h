#pragma once

#include "traits.h"

#include <ranges>

namespace nao {
    namespace impl {
        template <bool Const, std::ranges::input_range R, member_function_pointer Fn>
        class member_iterator {
            public:
            using base_iterator = std::ranges::iterator_t<std::conditional_t<Const, const R, R>>;
            using iterator_concept = typename base_iterator::iterator_concept;
            using iterator_category = typename base_iterator::iterator_category;
            using value_type = decltype(std::invoke(Fn{}, *base_iterator{}));
            using difference_type = typename base_iterator::difference_type;
            using pointer = std::remove_reference_t<value_type>*;
            using reference = value_type&;

            private:
            base_iterator _it{};
            Fn _fun{};

            public:
            constexpr member_iterator() = default;
            constexpr member_iterator(base_iterator pos, Fn fun) : _it{ pos }, _fun{ fun } { }

            constexpr member_iterator(member_iterator&& other) noexcept
                : _it{ std::move(other._it) }, _fun{ std::move(other._fun) } { }

            constexpr member_iterator& operator=(member_iterator&& other) noexcept {
                _it = std::move(other._it);
                _fun = std::move(other._fun);
                return *this;
            }


            constexpr member_iterator(const member_iterator& other)
                : _it{ other._it }, _fun{ other._fun } { }

            constexpr member_iterator& operator=(const member_iterator& other) {
                _it = other._it;
                _fun = other._fun;
                return *this;
            }

            /**
             * Dereferences the iterator, calling the member function
             */
            constexpr decltype(auto) operator*() const {
                return std::invoke(_fun, *_it);
            }


            // Basic bidirectional iterator
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

            // Ordering
            [[nodiscard]] friend constexpr std::strong_ordering operator<=>(
                const member_iterator& l, const member_iterator& r) {

                return l._it <=> r._it;
            }

            [[nodiscard]] friend constexpr bool operator==(
                const member_iterator& left, const member_iterator& right) {

                return left._it == right._it;
            }
        };


        /**
         * Dereferences to a struct containing the value and the index, which increments and
         * decrements with the iterator itself.
         */
        template <bool Const, std::ranges::input_range R>
        class with_index_iterator {
            public:
            using base_iterator = std::ranges::iterator_t<std::conditional_t<Const, const R, R>>;
            using iterator_concept = typename base_iterator::iterator_concept;
            using iterator_category = typename base_iterator::iterator_category;
            using size_type = size_t;
            using value_type = std::pair<typename base_iterator::value_type&, size_type>;
            using difference_type = typename base_iterator::difference_type;
            using pointer = value_type*;
            using reference = value_type&;

            private:
            base_iterator _it{};
            size_type _i{};

            public:
            constexpr with_index_iterator() = default;
            constexpr with_index_iterator(base_iterator it, size_type i)
                : _it { std::move(it) }, _i { i } { }

            constexpr with_index_iterator(with_index_iterator&& other) noexcept
                : _it { std::move(other._it) }, _i { other._i } { }

            constexpr with_index_iterator& operator=(with_index_iterator&& other) noexcept {
                _it = std::move(other._it);
                _i = other._i;
                return *this;
            }


            constexpr with_index_iterator(const with_index_iterator& other)
                : _it { other._it }, _i { other._i } { }

            constexpr with_index_iterator& operator=(const with_index_iterator& other) {
                _it = other._it;
                _i = other._i;
                return *this;
            }

            
            constexpr decltype(auto) operator*() {
                return value_type{ *_it, _i };
            }


            constexpr decltype(auto) operator++() {
                ++_it;
                ++_i;
                return *this;
            }

            constexpr decltype(auto) operator++(int) {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr decltype(auto) operator--() {
                --_it;
                --_i;
                return *this;
            }

            constexpr decltype(auto) operator--(int) {
                auto tmp = *this;
                --(*this);
                return tmp;
            }


            // Ordering
            [[nodiscard]] friend constexpr std::strong_ordering operator<=>(
                const with_index_iterator& l, const with_index_iterator& r) {

                return l._it <=> r._it;
            }

            [[nodiscard]] friend constexpr bool operator==(
                const with_index_iterator& left, const with_index_iterator& right) {

                return left._it == right._it && left._i == right._i;
            }
        };
    }

    /**
     * A range view that calls a member function.
     */
    template <std::ranges::input_range R, member_function_pointer Fn>
    class member_transform_view : public std::ranges::view_interface<member_transform_view<R, Fn>> {
        R _range;
        Fn _fun;
        public:
        constexpr member_transform_view(R range, Fn fun)
            : _range{ std::move(range) }, _fun{ fun } { }

        
        // Basic iterators that use the range's iterators
        constexpr auto begin() {
            return impl::member_iterator<false, R, Fn>{ std::ranges::begin(_range), _fun };
        }

        constexpr auto end() {
            return impl::member_iterator<false, R, Fn>{ std::ranges::end(_range), _fun };
        }

        // Const versions only make sense if a const range makes sense
        constexpr auto begin() const requires std::ranges::range<const R>
            && std::invocable<Fn, std::ranges::range_reference_t<const R>> {
            return impl::member_iterator<true, R, Fn>{ std::ranges::begin(_range), _fun };
        }

        constexpr auto end() const requires std::ranges::range<const R>
            && std::invocable<Fn, std::ranges::range_reference_t<const R>> {
            return impl::member_iterator<true, R, Fn>{ std::ranges::end(_range), _fun };
        }


        constexpr auto size() const requires
            requires { std::ranges::size(_range); }
        {
            return std::ranges::size(_range);
        }
    };


    class member_transform_fun {
        // Used for pipe operator support
        template <member_function_pointer Fn>
        struct partial {
            Fn fun;

            /**
             * Constructs a range view that applies the member_transform_view.
             */
            template <typename R>
            [[nodiscard]] friend constexpr auto operator|(R&& r, const partial& t) {
                return member_transform_view{ std::forward<R>(r), t.fun };
            }
        };

        public:
        /**
         * Constructs a range view that applies the member_transform_view.
         */
        template <std::ranges::viewable_range R, member_function_pointer Fn>
        [[nodiscard]] constexpr auto operator()(R&& range, Fn fun) {
            return member_transform_view{ std::forward<R>(range), fun };
        }

        /**
         * Constructs an object that can be used when combining views.
         */
        template <member_function_pointer Fn>
        [[nodiscard]] constexpr auto operator()(Fn fun) const {
            return partial<Fn>{ .fun = fun };
        }
    };

    // Range view that calls a specific member function on all objects
    inline constexpr member_transform_fun member_transform;

    template <std::ranges::input_range R>
    class with_index_view : public std::ranges::view_interface<with_index_view<R>> {
        R _range{};
        public:
        constexpr explicit with_index_view(R range) : _range{ std::move(range) } { }


        constexpr auto begin() {
            return impl::with_index_iterator<false, R>{ std::ranges::begin(_range), 0 };
        }

        constexpr auto end() {
            return impl::with_index_iterator<false, R>{
                std::ranges::end(_range), std::ranges::size(_range) };
        }


        [[nodiscard]] constexpr auto begin() const requires std::ranges::range<const R> {
            return impl::with_index_iterator<true, R>{ std::ranges::begin(_range), 0 };
        }

        [[nodiscard]] constexpr auto end() const requires std::ranges::range<const R> {
            return impl::with_index_iterator<true, R>{
                std::ranges::end(_range), std::ranges::size(_range) };
        }


        [[nodiscard]] constexpr auto size() const requires requires { std::ranges::size(_range); } {
            return std::ranges::size(_range);
        }
    };

    // Implements a callable and a direct pipe operator
    class with_index_fun {
        public:
        template <std::ranges::viewable_range R>
        [[nodiscard]] constexpr auto operator()(R&& range) const {
            return with_index_view{ std::views::all(std::forward<R>(range)) };
        }

        template <typename R>
        [[nodiscard]] friend constexpr auto operator|(R&& range, const with_index_fun& fun) {
            return fun(std::forward<R>(range));
        }
    };

    // Range view that returns a pair, which contains the dereferenced value and the index
    inline constexpr with_index_fun with_index;
}

namespace std {
    template <bool Const, ranges::input_range R, nao::member_function_pointer Fn>
    struct iterator_traits<nao::impl::member_iterator<Const, R, Fn>> {
        using iterator = nao::impl::member_iterator<Const, R, Fn>;

        using difference_type = typename iterator::difference_type;
        using value_type = typename iterator::value_type;
        using pointer = typename iterator::pointer;
        using reference = typename iterator::reference;
        using iterator_category = typename iterator::iterator_category;
    };

    template <bool Const, ranges::input_range R>
    struct iterator_traits<nao::impl::with_index_iterator<Const, R>> {
        using iterator = nao::impl::with_index_iterator<Const, R>;

        using difference_type = typename iterator::difference_type;
        using value_type = typename iterator::value_type;
        using pointer = typename iterator::pointer;
        using reference = typename iterator::reference;
        using iterator_category = typename iterator::iterator_category;
    };

    namespace ranges {
        template <input_range R, nao::member_function_pointer Fn>
        inline constexpr bool enable_borrowed_range<nao::member_transform_view<R, Fn>> = true;

        template <input_range R>
        inline constexpr bool enable_borrowed_range<nao::with_index_view<R>> = true;
    }
}
