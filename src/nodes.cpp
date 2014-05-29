#include "nodes.hpp"


namespace yakkai
{
    auto is_nil( node const* const n )
        -> bool
    {
        if ( n == nullptr ) {
            assert( false );

        } else if ( n->type == node_type::e_list ) {
            auto&& c = static_cast<cons const* const>( n );
            return c->car == nullptr && c->cdr == nullptr;

        } else {
            return false;
        }
    }

    auto is_atom( node const* const n )
        -> bool
    {
        if ( is_nil( n ) ) {
            return true;

        } else if ( n->type != node_type::e_list ) {
            return true;

        } else {
            return false;
        }
    }

    auto is_list( node const* const n )
        -> bool
    {
        if ( is_nil( n ) ) {
            return true;

        } else if ( n->type == node_type::e_list ) {
            return true;

        } else {
            return false;
        }
    }

    auto is_symbol( node const* const n )
        -> bool
    {
        if ( is_nil( n ) ) {
            return false;

        } else if ( n->type == node_type::e_symbol ) {
            return true;

        } else {
            return false;
        }
    }

    auto is_keyword( node const* const n )
        -> bool
    {
        if ( is_nil( n ) ) {
            return false;

        } else if ( n->type == node_type::e_keyword ) {
            return true;

        } else {
            return false;
        }
    }

    auto is_native_function( node const* const n )
        -> bool
    {
        if ( is_nil( n ) ) {
            return false;

        } else if ( n->type == node_type::e_native_function ) {
            return true;

        } else {
            return false;
        }
    }

    auto is_integer( node const* const n )
        -> bool
    {
        if ( is_nil( n ) ) {
            return false;

        } else if ( n->type == node_type::e_integer ) {
            return true;

        } else {
            return false;
        }
    }

    auto is_float( node const* const n )
        -> bool
    {
        if ( is_nil( n ) ) {
            return false;

        } else if ( n->type == node_type::e_float ) {
            return true;

        } else {
            return false;
        }
    }


    auto is_callable( node const* const n )
        -> bool
    {
        if ( is_nil( n ) ) return false;

        return n->attr == node_attribute::e_callable;
    }

} // namespace yakkai
