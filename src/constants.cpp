#include <cmath>
#include <boost/math/tools/roots.hpp>
#include <boost/program_options.hpp>
#include <tuple>
#include <iostream>
#include "debug.hpp"

namespace po = boost::program_options;


// The global variables we define

uint number_of_primes,
     sieving_threads,
     splitting_threads,
     chunk_length,
     output_chunk_length,
     number_of_factors,
     number_of_odds_to_find_factors,
     nth_prime;

// Functions for our estimations

inline uint estimate_upper_bound_nth_prime (uint n)
{
    return ceil ( n * (log(n) + log(log(n))) );
}

std::tuple<float,float> f (float x, float p)
{
    return std::make_tuple ( x * (log(x) + log(log(x)) - 1) - p,
                             log(x) + log(log(x)) + 1/log(x) );
}


void estimate_factors (uint n)
{
    float est = estimate_upper_bound_nth_prime (n);

    nth_prime = ceil (est);

    // The largest prime we need to fully sieve is no larger than:
    float largest_factor = sqrt (est);

    // We estimate the last odd number we need to sieve to find the factors
    uint tmp = ceil(largest_factor);
    if (tmp % 2 == 0)
        number_of_odds_to_find_factors = (tmp - 4) / 2;
    else
        number_of_odds_to_find_factors = (tmp - 1) / 2;

    // Hence we estimate from below what prime that would be

    float guess_m = sqrt (largest_factor);

    using namespace std::placeholders;
    float itr = boost::math::tools::newton_raphson_iterate (
            std::bind (f, _1, largest_factor),
            guess_m,
            float(10),
            std::numeric_limits<float>::max (),
            std::numeric_limits<float>::digits / 2);

    number_of_factors = ceil (itr);

    trace (("Estimation: nth prime=%f, largest factor=%f which we think is the %d-th prime.", est, largest_factor, number_of_factors));
}


void initialize_constants (uint n)
{
    time_function ();

    estimate_factors (n);

    uint number_of_digits = ceil (log10 (estimate_upper_bound_nth_prime (n)));

    output_chunk_length = (number_of_digits + 1)*chunk_length;
}


bool parse_options(int ac, char* av[])
{
    bool help = false;
    po::options_description options("Options");

    options.add_options()
        ("sieving,s", po::value<uint>(&sieving_threads)->default_value (1),
         "number of threads sieving")
        ("splitting,p", po::value<uint>(&splitting_threads)->default_value (1),
         "number of threads converting uints to output chunks.")
        ("primes,n", po::value<uint>(&number_of_primes)->default_value (2000000),
         "the number of primes we seek")
        ("chunk,c", po::value<uint>(&chunk_length)->default_value (100000),
         "the number of odds we sieve per chunk in the sieving threads")
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

    trace (("Configuration: primes=%d, sieving=%d, splitting=%d, chunk=%d",
            number_of_primes,
            sieving_threads,
            splitting_threads,
            chunk_length));

      // We estimate the rest of the constants

    initialize_constants (number_of_primes);

    return true;
}

