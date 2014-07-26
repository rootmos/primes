#include <cmath>
#include <boost/math/tools/roots.hpp>
#include <boost/program_options.hpp>
#include <tuple>
#include <iostream>
#include "debug.hpp"

namespace po = boost::program_options;


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
    uint number_of_digits = ceil (log10 (estimate_upper_bound_nth_prime (n)));
    
    estimate_number_of_factors_needed (n);

}


bool parse_options(int ac, char* av[])
{
    bool help = false;
    po::options_description options("Options");

    uint sieving, splitting, primes;
    
    options.add_options()
        ("sieving,s", po::value<uint>(&sieving)->default_value (1),
         "number of threads sieving")
        ("splitting,p", po::value<uint>(&splitting)->default_value (1),
         "number of threads converting uints to output chunks.")
        ("primes,n", po::value<uint>(&primes)->default_value (2000000),
         "the number of primes we seek")
        ("help,h", "show this help message"); 

    po::positional_options_description number_options;

    number_options.add ("primes", -1);

    po::variables_map vm;
    try {
        po::store(
                po::command_line_parser(ac, av).options(options)
                .positional(number_options).run(),
                vm);
        po::notify(vm);
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << "\n";
        help = true;
    }    

    if (help || vm.count("primes") != 1 || vm.count("help") > 0)
    {
        std::cout << "Usage: " << av[0] << " [options] [number-of-primes]\n";
        std::cout << options;
        return false;
    }

    trace (("Configuration: primes=%d, sieving=%d, splitting=%d.",
            primes,
            sieving,
            splitting));

    return true;
}

