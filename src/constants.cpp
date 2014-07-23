#include <cmath>
#include <boost/math/tools/roots.hpp>
#include <tuple>

using uint = unsigned int;

inline uint estimate_upper_bount_nth_prime (uint n)
{
    return ceil ( n * (log(n) + log(log(n))) );
}

std::tuple<float,float> f_fderiv (float x)
{

}


uint estimate_number_of_factors_needed (uint n)
{
    uint largest_factor = sqrt (estimate_upper_bount_nth_prime (n));

}

