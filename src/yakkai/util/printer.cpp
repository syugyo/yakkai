#include "printer.hpp"


namespace yakkai
{
    namespace detail
    {
        auto print_node_to_stream_with_type( std::ostream& os, node const* const n )
            -> std::ostream&
        {
            if ( n == nullptr ) {
                return os;
            }

            if ( is_nil( n ) ) {
                os << "(): unit";

            } else if ( n->type == node_type::e_list ) {

                os << "( ";
                auto c = static_cast<cons const* const>( n );
                for(;;) {
                    print_node_to_stream_with_type( os, c->car );
                    os << " ";
                    if ( is_nil( c->cdr ) ) {
                        break;
                    }

                    assert( c->cdr->type == node_type::e_list );
                    c = static_cast<cons const* const>( c->cdr );
                }
                os << "): e_list";

            } else if ( n->type == node_type::e_symbol ) {
                auto s = static_cast<symbol const* const>( n );
                os << s->value << ": symbol";

            } else if ( n->type == node_type::e_integer ) {
                auto s = static_cast<integer_value const* const>( n );
                os << s->value << ": int";

            } else if ( n->type == node_type::e_float ) {
                auto s = static_cast<float_value const* const>( n );
                os << s->number << "e" << s->exp << ": float";

            } else {
                os << debug_string( n->type ) << " : !!Unknown!!";
            }

            return os;
        }
    }


    auto print_node( node const* const n )
        -> void
    {
        detail::print_node_to_stream_with_type( std::cout, n );
        std::cout << std::endl;
    }

} // namespace yakkai
