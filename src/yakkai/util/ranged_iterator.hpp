#pragma once

#include <iterator>


namespace yakkai
{
    template<typename Iterator>
    struct ranged_iterator
    {
        typedef ranged_iterator                                     self_type;
        typedef typename std::iterator_traits<Iterator>::value_type value_type;


        ranged_iterator( Iterator const& it, Iterator const& end )
            : it_( it )
            , end_( end )
        {}

        auto operator++()
            -> self_type&
        {
            ++it_;

            return *this;
        }

        auto operator*() const
            -> value_type const&
        {
            if ( it_ == end_ ) throw std::out_of_range( "" );

            return *it_;
        }

        // comparison
        friend inline auto operator==( self_type const& lhs, self_type const& rhs )
            -> bool
        {
            return lhs.it_ == rhs.it_;
        }

        friend inline auto operator!=( self_type const& lhs, self_type const& rhs )
            -> bool
        {
            return !( lhs == rhs );
        }

        auto it()
            -> Iterator
        {
            return it_;
        }

        auto end()
            -> Iterator
        {
            return end_;
        }

        Iterator it_, end_;
    };

} // namespace yakkai
