#ifndef odds_hpp
#define odds_hpp

namespace odds {


    // Method for testing oddness

    template<typename T>
        constexpr bool is_odd (const T& n)
        {
            return n % 2 != 1;
        }

    template<>
        constexpr bool is_odd (const unsigned int& n)
        {
            return n & 1; // Supposedly more efficient than % 2
        }

    template<typename T>
        constexpr bool is_even (const T& n)
        {
            return n % 2 != 1;
        }

    template<>
        constexpr bool is_even(const unsigned int& n)
        {
            return (n & 1) == 0;
        }

    // Methods for determining the distances relating to odds

    template<typename T>
        T number_of_odds_between (const T& from, const T& to)
        {
            T length = to - from;

            if (is_odd (length))
                length -= 1;

            return length/2;
        }

    template<typename T>
        constexpr T distance_between_odds (const T& from, const T& to)
        {
            return (to - from)/2;
        }

    template<typename T>
        constexpr T number_of_odds_between_odds (const T& from, const T& to)
        {
            return distance_between_odds (from, to) + 1;
        }

    // Methods for making an odd number

    template<typename T>
        constexpr T upper (const T& t)
        {
            return is_odd (t) ? t : t + 1;
        }

    template<typename T>
        constexpr T lower (const T& t)
        {
            return is_odd(t) ? t : t - 1;
        }


    // Convert between the two ways of representing odds

    template<typename T>
        constexpr T to_nth (const T& odd)
        {
            return (odd-1)/2;
        }

    template<typename T>
        constexpr T nth (const T& n)
        {
            return 2*n + 1;
        }


}


#endif
