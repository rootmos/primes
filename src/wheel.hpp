#ifndef wheel_hpp
#define wheel_hpp

#include <vector>
#include <algorithm>

#include <iostream> // For testing purposes
#include <iterator>

using uint = unsigned int;

static constexpr uint wheel_buffer = 20;

template<uint n>
class wheel
{
    uint wheel[n];
    uint wheel_length;
    uint length;
    uint start;

public:

    void invent (std::vector<uint>& factors)
    {
        // Find the number m of factors such that the product of them are
        // smaller than n. That product will be the max length of the wheel.

        uint m = 0;
        length = 1;

        auto itr = factors.begin ();

        start = 1;// *itr; // The wheel will start at the first

        while (itr != factors.end ())
        {
            uint next_prod = length * (*itr);

            if (next_prod > n)
                break;

            length = next_prod;
            ++m;
            ++itr;
        }

        std::vector<int> counters;
        counters.reserve (m);

        std::transform(factors.begin(), itr,
                       std::inserter(counters, counters.begin()),
                       [this] (int p) { return (start+2) % p; });

        uint i = 0;
        wheel[i] = 1;

        for (uint j = 0; j < length; j++)
        {
            if ( std::any_of (counters.begin (), counters.end (),
                std::bind(std::equal_to<int>(), std::placeholders::_1, 0)) )
            {
                wheel[i] += 1;
            }
            else
            {
                wheel[++i] = 1;
            }

            std::transform(counters.begin (), counters.end (),
                           factors.begin (), counters.begin (),
                           [] (int c, int p) { return (c+2)%p; });

            //std::copy(counters.begin(), counters.end(),
            //          std::ostream_iterator<int>(std::cout, "\n"));
        }

        wheel_length = i;

        std::cout << "The wheel:" << std::endl;
        for (uint j = 0; j < wheel_length; j++)
        {
            std::cout << wheel[j];
        }
        std::cout << std::endl;
    }


    class iterator
    {
        const class wheel& wheel;


        uint their_start; // Their buffer starts at this odd number
        uint our_start; // ... which corresponds to this for us
        int offset; // ... +- thin number of odds

        uint their_position;
        int our_position;

        iterator(const class wheel& w, uint from):
            wheel (w)
        {
            their_start = (from - 1)/2;

            offset = their_start % w.length;

            our_start = their_start - offset;

            std::cout << "1 their_start=" << their_start << std::endl;
            std::cout << "1 our_start=" << our_start << std::endl;
            std::cout << "1 offset=" << offset << std::endl;
            

            uint i = 0;

            while (our_start < their_start)
            {
                our_start += w.wheel[i];
                i = (i + 1) % w.wheel_length;
            }

            offset = our_start - their_start;
            our_start = i;

            std::cout << "2 their_start=" << their_start << std::endl;
            std::cout << "2 our_start=" << our_start << std::endl;
            std::cout << "2 offset=" << offset << std::endl;

            reset ();

        }

        friend class wheel;

    public:

        void reset ()
        {
            their_position = offset;
            our_position = our_start;
        }

        uint next ()
        {
            their_position += wheel.wheel[our_position];
            our_position = (our_position + 1) % wheel.wheel_length;
            return their_position;
        }

        uint position ()
        {
            return their_position;
        }
    };

    iterator iterate_from (uint offset)
    {
        return iterator (*this, offset);
    }

};


extern wheel<wheel_buffer> the_wheel;

#endif
