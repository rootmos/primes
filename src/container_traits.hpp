#ifndef container_traits_hpp
#define container_traits_hpp

#include <functional>


namespace container_traits
{
    namespace utils
    {
        struct failure { };

        template <typename T>
            constexpr bool failed()
            {
                return std::is_same<T, utils::failure>::value;
            }

        template <typename T>
            constexpr bool succeeded()
            {
                return !std::is_same<T, utils::failure>::value;
            }
    }


    template <typename T>
        struct has_front
        {
        private:
            template <typename X>
                static auto check(const X& x) -> decltype(x.front ());

            static utils::failure check(...);

        public:
            static constexpr bool value =
                utils::succeeded<decltype(check (std::declval<const T&>()) )>();
        };

    template <typename T>
        struct has_top
        {
        private:
            template <typename X>
                static auto check(const X& x) -> decltype(x.top ());

            static utils::failure check(...);

        public:
            static constexpr bool value =
                utils::succeeded<decltype(check (std::declval<const T&>()) )>();
        };

    template <typename T>
        struct has_pop
        {
        private:
            template <typename X>
                static auto check(X& x) -> decltype(x.pop ());

            static utils::failure check(...);

        public:
            static constexpr bool value =
                utils::succeeded<decltype(check (std::declval<T&>()) )>();
        };


    template <typename T>
        struct has_pop_front
        {
        private:
            template <typename X>
                static auto check(X& x) -> decltype(x.pop_front ());

            static utils::failure check(...);

        public:
            static constexpr bool value =
                utils::succeeded<decltype(check (std::declval<T&>()) )>();
        };


    template <typename T>
        struct has_push_back
        {
        private:
            template <typename X>
                static auto check(X& x)
                    -> decltype(x.push_back (std::declval<typename T::value_type>()));

            static utils::failure check(...);

        public:
            static constexpr bool value =
                utils::succeeded<decltype(check (std::declval<T&>()) )>();
        };

    template <typename T>
        struct has_push
        {
        private:
            template <typename X>
                static auto check(X& x)
                    -> decltype(x.push (std::declval<typename T::value_type>()));

            static utils::failure check(...);

        public:
            static constexpr bool value =
                utils::succeeded<decltype(check (std::declval<T&>()) )>();
        };

}


#endif
