#pragma once

#include <utility>


namespace yakkai
{
    template<typename I>
    auto gcd( I m, I n )
        -> I
    {
        if ( n < m ) std::swap( m, n );

        I r;
        while( n != 0 ) {
            r = m % n;
            m = n;
            n = r;
        }
        return m;
    }

    template<typename I>
    auto lcm( I m, I n )
        -> I
    {
        return ( m * n ) / gcd( m, n );
    }
} // namespace yakkai
