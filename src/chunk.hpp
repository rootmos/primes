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
        chunk_impl (uint f, uint t, std::vector<bool>&& odds);

        void fill_offset (uint p);
        void do_count ();
        void prepare_for_output ();

        void resize (uint n);
    };

    std::shared_ptr<chunk_impl> impl;

public:

    chunk ();
    chunk (uint f, uint t);
    chunk (uint f, uint t, std::vector<bool>&& odds);

    bool operator<(const chunk& rhs) const;
    bool operator==(const chunk& rhs) const;
    bool operator!=(const chunk& rhs) const;

    uint size () const;

    void resize (uint n) { impl->resize (n); };

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
    return impl->primes;
}

// The ordering of our chunks

inline bool
chunk::operator<(const chunk& rhs) const
{
    return impl->from > rhs.impl->from;
}

// The equality and inequality operators

inline bool
chunk::operator==(const chunk& rhs) const
{
    return impl == rhs.impl;
}

inline bool
chunk::operator!=(const chunk& rhs) const
{
    return !operator==(rhs);
}


#endif
