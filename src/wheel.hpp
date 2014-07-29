#ifndef wheel_hpp
#define wheel_hpp

#include <vector>
#include <algorithm>

#include <iostream> // For testing purposes
#include <iterator>

using uint = unsigned int;

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

        //std::cout << "The wheel:" << std::endl;
        //for (uint j = 0; j < wheel_length; j++)
        //{
        //    std::cout << wheel[j];
        //}
        //std::cout << std::endl;
    }


    class iterator
    {
        const class wheel& wheel;
        uint offset;
        uint position;

        iterator(const class wheel& w, uint start):
            wheel (w)
        {
            offset = start - (start % (2*w.length)) + 1;

            uint i = 0;
            while(i < w.wheel_length)
            {
                uint test_offset = offset + 2*w.wheel[i];
                if (test_offset >= start)
                    break;
                offset = test_offset;
                i++;
            }

            position = i;
        }

        friend class wheel;

    public:

        uint next ()
        {
            uint next = wheel.wheel[position];

            position = (position + 1) % wheel.wheel_length;
            offset = offset + 2*next;
            return offset;
        }
    };

    iterator iterate_from (uint offset)
    {
        return iterator (*this, offset);
    }

};

#endif
