#include <iostream>
#include <string>
#include <cassert>
#include <cctype>


namespace yakkai
{
    template<typename It>
    auto is_eof( It const& it, It const& end )
        -> bool
    {
        return it == end;
    }

    class reached_to_eof : std::exception
    {
    };

    template<typename It>
    auto expect_not_eof( It const& it, It const& end )
        -> void
    {
        if ( is_eof( it, end ) ) {
            throw reached_to_eof();
        }
    }

    template<typename It>
    auto step_iterator( It& it, It const& end, std::size_t step = 1 )
        -> void
    {
        expect_not_eof( it, end );

        for( std::size_t i=0; i<step; ++i ) {
            ++it;

            expect_not_eof( it, end );
        }
    };


    enum struct node_type
    {
        none,

        // list
            list,

        // atom
            symbol
            };







    struct node
    {
        node( node_type t )
            : type( t )
            {}

        node_type type;
    };





    struct cons : public node
    {
        cons()
            : node( node_type::list )
            {}

        node* car = nullptr;
        node* cdr = nullptr;
    };


    struct symbol : public node
    {
        symbol( std::string const& v )
            : node( node_type::symbol )
            , value( v )
            {}

        std::string value;
    };




    auto is_nil( node const* const n )
        -> bool
    {
        if ( n == nullptr ) {
            return true;

        } else if ( n->type == node_type::list ) {
            return is_nil( static_cast<cons const* const>( n )->car );

        } else {
            return false;
        }
    }



    auto print( node const* const n, std::size_t indent = 0 )
        -> void
    {
        auto const space = std::string( indent * 2, ' ' );

        if ( is_nil( n ) ) {
            std::cout << space << "nil" << std::endl;
            return;

        } else if ( n->type == node_type::list ) {
            auto c = static_cast<cons const* const>( n );
            std::cout << space << "( " << std::endl;
            print( c->car, indent + 1 );
            std::cout << space << "." << std::endl;
            print( c->cdr, indent + 1 );
            std::cout << space << ")" << std::endl;

        } else if ( n->type == node_type::symbol ) {
            auto s = static_cast<symbol const* const>( n );
            std::cout << space << s->value << std::endl;

        } else {
            std::cout << space << "!!Unknown!!" << std::endl;
        }
    }






    auto print_atom( node const* const n, std::size_t indent = 0 )
        -> void
    {
        auto const space = std::string( indent * 2, ' ' );

        if ( is_nil( n ) ) {
            std::cout << space << "nil" << std::endl;
            return;

        } else if ( n->type == node_type::symbol ) {
            auto s = static_cast<symbol const* const>( n );
            std::cout << space << s->value << std::endl;

        }
    }


    auto print2( node const* const n, std::size_t indent = 0 )
        -> void
    {
        auto const space = std::string( indent * 2, ' ' );

        if ( is_nil( n ) ) {
            std::cout << space << "nil" << std::endl;
            return;

        } else if ( n->type == node_type::list ) {

            std::cout << "( ";
            auto c = static_cast<cons const* const>( n );
            for(;;) {
                print2( c->car, indent + 1 );
                std::cout << " ";
                if ( is_nil( c->cdr ) ) {
                    break;
                }

                assert( c->cdr->type == node_type::list );
                c = static_cast<cons const* const>( c->cdr );
            }
            std::cout << ")";
            if ( indent == 0 ) {
                std::cout << std::endl;
            }

            return;

        } else if ( n->type == node_type::symbol ) {
            auto s = static_cast<symbol const* const>( n );
            std::cout << s->value;

        } else {
            std::cout << "!!Unknown!!";
        }
    }







    template<typename T, typename... Args>
    auto make_node( Args&&... args )
        -> T*
    {
        return new T( std::forward<Args>( args )... );
    }


    template<typename It>
    auto skip_space( It& it, It const& end )
        -> void
    {
        expect_not_eof( it, end );
        while( std::isspace( *it ) ) {
            step_iterator( it, end );
        }
    }




    template<typename It>
    auto parse_symbol( It& it, It const& end )
        -> node*
    {
        It begin = it;

        if ( ( *it >= 'A' && *it <= 'Z' ) || ( *it >= 'a' && *it <= 'z' ) ) {
            step_iterator( it, end );

            while( ( *it >= 'A' && *it <= 'Z' )
                   || ( *it >= 'a' && *it <= 'z' )
                   || ( *it >= '0' && *it <= '1' ) ) {
                step_iterator( it, end );
            }

            return make_node<symbol>( std::string( begin, it ) );

        } else {
            it = begin;
            return nullptr;
        }
    }


    template<typename It>
    auto parse_closer_s_expression( It& it, It const& end, node* n = nullptr )
        -> node*
    {
        if ( *it != ')' ) {
            assert( false );
        }
        step_iterator( it, end );

        return n;
    }


    template<typename It>
    auto parse_s_expression( It& it, It const& end, bool = false )
        -> node*;
    template<typename It>
    auto parse_s_expression( It& it, It const& end, cons* outer_cell, bool = false )
        -> cons*;


    template<typename It>
    auto parse_s_expression_or_closer( It& it, It const& end )
        -> node*
    {
        return parse_s_expression( it, end, true );
    }
    template<typename It>
    auto parse_s_expression_or_closer( It& it, It const& end, cons* outer_cell )
        -> cons*
    {
        return parse_s_expression( it, end, outer_cell, true );
    }


    template<typename It>
    auto parse_s_expression( It& it, It const& end, cons* outer_cell, bool is_enable_closer )
        -> cons*
    {
        auto s = parse_s_expression( it, end, is_enable_closer );

        //
        if ( s == nullptr ) {
            outer_cell->cdr = nullptr;
            return nullptr;

        } else {
            auto inner_cell = make_node<cons>();
            inner_cell->car = s;

            assert( outer_cell != nullptr );
            outer_cell->cdr = inner_cell;

            return inner_cell;  // NOTE:
        }
    }


    template<typename It>
    auto parse_s_expression( It& it, It const& end, bool is_enable_closer )
        -> node*
    {
        skip_space( it, end );

        if ( *it == '(' ) {
            step_iterator( it, end );

            // car
            auto s = parse_s_expression_or_closer( it, end );
            if ( s == nullptr ) {
                return nullptr;
            }

            //
            auto cell = make_node<cons>();
            cell->car = s;

            skip_space( it, end );
            if ( *it == '.' ) {
                // cons cell
                step_iterator( it, end );

                auto s = parse_s_expression( it, end, cell );
                return parse_closer_s_expression( it, end, cell );

            } else {
                // list
                auto last_cell = cell;
                while( last_cell = parse_s_expression_or_closer( it, end, last_cell ) );

                return cell;
            }

        } else if ( is_enable_closer &&  *it == ')' ) {
            return parse_closer_s_expression( it, end );

        } else {
            // atom
            if ( auto&& s = parse_symbol( it, end ) ) {
                return s;

            } else {
                throw "parse error";
            }
        }
    }



    template<typename It>
    auto parse_program( It& it, It const& end )
        -> node*
    {
        parse_s_expression( it, end );
    }












    auto eval( std::string const& source )
        -> void
    {
        auto it = source.cbegin();
        auto const end = source.cend();

        try {
            for (;;) {
                auto s = parse_program( it, end );

                std::cout << "Result: " << std::endl;
                print2( s );
            }

        } catch( char const* const message ) {
            std::cout << "exception!: " << message << std::endl;
        } catch( reached_to_eof const& e ) {
        }

        std::cout << "rest: " << std::string( it, end ) << std::endl;


    }
} // namespace yakkai






int main()
{
    std::string const test_case = R"::(
(list (quote a) (quote b) abc)
(a . b)
()

)::";

    std::cout << test_case << std::endl;


    yakkai::eval( test_case );
}
