#ifndef odds_hpp
#define odds_hpp

namespace odds {

template<typename T>
T number_of_odds_between (T from, T to)
{
    T length = to - from;

    if (length % 2 == 1)
        length -= 1;

    return length/2;
}

template<typename T>
constexpr T number_of_odds_between_odds (const T& from, const T& to)
{
    return (to - from)/2 + 1;
}


template<typename T>
constexpr T upper (const T& t)
{
    return t % 2 == 0 ? t + 1 : t;
}

template<typename T>
constexpr T lower (const T& t)
{
    return t % 2 == 0 ? t - 1 : t;
}

}


#endif
