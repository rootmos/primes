#include <cmath>
#include <boost/math/tools/roots.hpp>
#include <tuple>
#include <iostream>

#include "debug.hpp"


inline uint estimate_upper_bound_nth_prime (uint n)
{
    return ceil ( n * (log(n) + log(log(n))) );
}

std::tuple<float,float> f (float x, float p)
{
    return std::make_tuple ( x * (log(x) + log(log(x)) - 1) - p, 
                             log(x) + log(log(x)) + 1/log(x) );
}


uint estimate_number_of_factors_needed (uint n)
{
    time_function ();
    // The largest prime we need to fully sieve is no larger than:
    float largest_factor = sqrt (estimate_upper_bound_nth_prime (n));

    // Hence we estimate from below what prime that would be

    float guess_m = sqrt (largest_factor); 
    
    using namespace std::placeholders;
    float itr = boost::math::tools::newton_raphson_iterate (
            std::bind (f, _1, largest_factor),
            guess_m,
            float(10),
            std::numeric_limits<float>::max (),
            std::numeric_limits<float>::digits / 2);

    return ceil (itr);
}


void initialize_constants (uint n)
{
    
    estimate_number_of_factors_needed (n);

}

