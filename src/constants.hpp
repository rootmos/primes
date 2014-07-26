#ifndef constants_hpp
#define constants_hpp

using uint = unsigned int;

extern uint number_of_primes,
       sieving_threads,
       splitting_threads,
       chunk_length,
       output_chunk_length,
       number_of_factors,
       number_of_odds_to_find_factors,
       nth_prime;

bool parse_options(int ac, char* av[]);

#endif
