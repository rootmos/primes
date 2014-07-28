#ifndef chunk_hpp
#define chunk_hpp

#include <vector>
#include <string>
#include <memory>
#include "constants.hpp"

using uint = unsigned int;

class chunk
{
    // Internal data structure

    struct chunk_impl
    {
        uint from;
        uint to;
        uint primes;

        std::vector<bool> odds;
        std::string output;

        chunk_impl (uint f, uint t);

        void fill_offset (uint p);
        void do_count ();
        void prepare_for_output ();
    };

    std::shared_ptr<chunk_impl> impl;

public:

    chunk ();
    chunk (uint f, uint t);

    bool operator<(const chunk& rhs) const;

    uint size () const;

    void sieve (std::vector<uint>& factors);

    void prepare_for_output () { impl->prepare_for_output (); };

    void c_str (const char*& buffer, size_t& length) const;

    uint from () const { return impl->from; };
    uint to () const { return impl->to; };
};


// The method for getting the number of primes in the chunk

inline uint
chunk::size () const
{
    if (impl->primes == 0)
        impl->do_count ();

    return impl->primes;
}


// The ordering of our chunks

inline bool
chunk::operator<(const chunk& rhs) const
{
    return impl->from < rhs.impl->from;
}


#endif
