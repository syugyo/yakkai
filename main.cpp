#include <iostream>
#include <string>
#include <cassert>
#include <cctype>
#include <iterator>
#include <stdexcept>


namespace yakkai
{


    class reached_to_eof : std::exception
    {
    };




    enum struct node_type
    {
        e_none,

        // list
            list,

        // atom
            symbol,
            string,
            e_integer,
            e_ratio,
            e_float,
            e_complex
            };






    //
    struct node
    {
        node( node_type t )
            : type( t )
            {}

        node_type type;
    };

    //
    struct cons : public node
    {
        cons()
            : node( node_type::list )
            {}

        node* car = nullptr;
        node* cdr = nullptr;
    };

    //
    struct symbol : public node
    {
        symbol( std::string const& v )
            : node( node_type::symbol )
            , value( v )
            {}

        std::string value;
    };

    //
    struct integer_value : public node
    {
        integer_value( std::string const& radix, std::string number )
            : node( node_type::e_integer )
            , radix( radix )
            , number( number )
            {}

        std::string radix, number;
    };


    //
    struct float_value : public node
    {
        float_value( std::string const& number, std::string exp = "" )
            : node( node_type::e_float )
            , number( number )
            , exp( exp )
            {}

        std::string number, exp;
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


#if 0
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
#endif





    auto print2( node const* const n, std::size_t indent = 0 )
        -> void
    {
        auto const space = std::string( indent * 2, ' ' );

        if ( is_nil( n ) ) {
            std::cout << space << "nil";

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
            std::cout << "): list";

        } else if ( n->type == node_type::symbol ) {
            auto s = static_cast<symbol const* const>( n );
            std::cout << s->value << ": symbol";

        } else if ( n->type == node_type::e_integer ) {
            auto s = static_cast<integer_value const* const>( n );
            std::cout << s->number << "(" << s->radix << "): int";

        } else if ( n->type == node_type::e_float ) {
            auto s = static_cast<float_value const* const>( n );
            std::cout << s->number << "e" << s->exp << ": float";

        } else {
            std::cout << "!!Unknown!!";
        }

            if ( indent == 0 ) {
                std::cout << std::endl;
            }
    }



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







    template<typename T, typename... Args>
    auto make_node( Args&&... args )
        -> T*
    {
        return new T( std::forward<Args>( args )... );
    }




    template<typename RangedIterator>
    auto is_eof( RangedIterator const& rng_it )
        -> bool
    {
        return rng_it.it_ == rng_it.end_;
    }

    template<typename RangedIterator>
    auto expect_not_eof( RangedIterator const& rng_it )
        -> void
    {
        if ( is_eof( rng_it ) ) {
            throw reached_to_eof();
        }
    }

    template<typename RangedIterator>
    auto step_iterator( RangedIterator& rng_it, std::size_t step = 1 )
        -> void
    {
        expect_not_eof( rng_it );

        for( std::size_t i=0; i<step && !is_eof( rng_it ); ++i ) {
            ++rng_it;
        }
    };






    template<typename RangedIterator>
    auto skip_space( RangedIterator& rng_it )
        -> void
    {
        if ( is_eof( rng_it ) ) return;

        //expect_not_eof( it, end );
        while( !is_eof( rng_it ) && std::isspace( *rng_it ) ) {
            step_iterator( rng_it );
        }
    }




    template<typename RangedIterator>
    auto parse_symbol( RangedIterator& rng_it )
        -> node*
    {
        RangedIterator begin = rng_it;

        if ( ( *rng_it >= 'A' && *rng_it <= 'Z' ) || ( *rng_it >= 'a' && *rng_it <= 'z' ) ) {
            step_iterator( rng_it );

            while( !is_eof( rng_it )
                   && ( ( *rng_it >= 'A' && *rng_it <= 'Z' )
                        || ( *rng_it >= 'a' && *rng_it <= 'z' )
                        || ( *rng_it >= '0' && *rng_it <= '9' ) )
                ) {
                step_iterator( rng_it );
            }

            return make_node<symbol>( std::string( begin.it(), rng_it.it() ) );

        } else {
            rng_it = begin;
            return nullptr;
        }
    }


    template<typename RangedIterator>
    auto parse_digit( RangedIterator& rng_it )
        -> bool
    {
        RangedIterator begin = rng_it;

        while( !is_eof( rng_it ) && ( *rng_it >= '0' && *rng_it <= '9' ) ) {
            step_iterator( rng_it );
        }

        // return true if digits are parsed
        return begin != rng_it;
    }

    template<typename RangedIterator>
    auto parse_sign( RangedIterator& rng_it )
        -> bool
    {
        RangedIterator begin = rng_it;

        if ( !is_eof( rng_it ) && ( *rng_it == '+' || *rng_it == '-' ) ) {
            step_iterator( rng_it );
        }

        // return true if digits are parsed
        return begin != rng_it;
    }



    template<typename RangedIterator>
    auto parse_digit_with_sign( RangedIterator& rng_it )
        -> bool
    {
        RangedIterator begin = rng_it;

        // sign is optional
        parse_sign( rng_it );

        skip_space( rng_it );

        //
        if ( parse_digit( rng_it ) ) {
            return true;

        } else {
            rng_it = begin;
            return false;
        }
    }





    template<typename RangedIterator>
    auto parse_integer( RangedIterator& rng_it )
        -> node*
    {
        RangedIterator begin = rng_it;
        std::string radix, number;

        if ( *rng_it == '#' ) {
            step_iterator( rng_it );
            skip_space( rng_it );

            if ( *rng_it == 'x' ) {
                //

            } else {
                // base spacified by number
                {
                    RangedIterator base_it = rng_it;
                    if ( !parse_digit( rng_it ) ) {
                        rng_it = begin;
                        return nullptr;
                    }
                    radix += std::string( base_it.it(), rng_it.it() );
                }
            }
            skip_space( rng_it );

            if ( *rng_it == 'r' || *rng_it == 'R' ) {
                rng_it = begin;
                return nullptr;
            }
        }
        skip_space( rng_it );

        {
            RangedIterator base_it = rng_it;
            if ( !parse_digit_with_sign( rng_it ) ) {
                rng_it = begin;
                return nullptr;
            }
            number += std::string( base_it.it(), rng_it.it() );
        }
        skip_space( rng_it );

        //
        if ( !is_eof( rng_it ) && ( *rng_it == '.' || *rng_it == 'e' || *rng_it == 'E' ) ) {
            rng_it = begin;
            return nullptr;
        }

        return make_node<integer_value>( radix, number );
    }


    // 0.0
    // .0
    // .0e10
    // (+|-) (digit*)? (.)? digit+ ( (e|E) (+|-) digit+ )?
    template<typename RangedIterator>
    auto parse_float( RangedIterator& rng_it )
        -> node*
    {
        RangedIterator begin = rng_it;
        std::string number, exp;

        // sign is optional
        bool const has_sign = parse_sign( rng_it );
        if ( has_sign ) {
            number += std::string( begin.it(), rng_it.it() );
        }
        skip_space( rng_it );



        //
        bool const has_digit = [&]() mutable {
            RangedIterator base_it = rng_it;
            if ( parse_digit( rng_it ) ) {
                skip_space( rng_it );

                number += std::string( base_it.it(), rng_it.it() );
                return true;
            }

            return false;
        }();


        //
        bool const has_float_point = [&]() mutable {
            if ( *rng_it == '.' ) {
                step_iterator( rng_it );
                skip_space( rng_it );

                number += ".";
                return true;
            }
            return false;
        }();

        bool const has_float_number = [&]() mutable {
            if ( has_float_point ) {
                RangedIterator base_it = rng_it;
                if ( parse_digit( rng_it ) ) {
                    skip_space( rng_it );
                    number += std::string( base_it.it(), rng_it.it() );
                    return true;
                }
            }
            return false;
        }();

        if ( *rng_it == 'e' || *rng_it == 'E' ) {
            if ( has_digit || has_float_number ) {
                step_iterator( rng_it );
                skip_space( rng_it );

                {
                    RangedIterator base_it = rng_it;
                    if ( !parse_digit_with_sign( rng_it ) ) {
                        rng_it = begin;
                        return nullptr;
                    }
                    exp += std::string( base_it.it(), rng_it.it() );
                }

            } else {
                rng_it = begin;
                return nullptr;
            }
        }

        return make_node<float_value>( number, exp );
    }




    template<typename RangedIterator>
    auto parse_numbers( RangedIterator& rng_it )
        -> node*
    {
        if ( node* i = parse_integer( rng_it ) ) {
            return i;

        } else if ( node* f = parse_float( rng_it ) ) {
            return f;

        } else {
            return nullptr;
        }
    }


    template<typename RangedIterator>
    auto parse_atom( RangedIterator& rng_it )
        -> node*
    {
        if ( auto s = parse_symbol( rng_it ) ) {
            return s;

        } else if ( auto n = parse_numbers( rng_it ) ) {
            return n;

        } else {
            throw "parse error";
        }
    }


    template<typename RangedIterator>
    auto parse_token_separate( RangedIterator& rng_it )
        -> bool
    {
        return true;
    }


    template<typename RangedIterator>
    auto parse_closer_s_expression( RangedIterator& rng_it, node* n = nullptr )
        -> node*
    {
        if ( *rng_it != ')' ) {
            assert( false );
        }
        step_iterator( rng_it );

        return n;
    }


    template<typename RangedIterator>
    auto parse_s_expression( RangedIterator&, bool const = false )
        -> node*;
    template<typename RangedIterator>
    auto parse_s_expression( RangedIterator&, cons*, bool const = false )
        -> cons*;


    template<typename RangedIterator>
    auto parse_s_expression_or_closer( RangedIterator& rng_it )
        -> node*
    {
        return parse_s_expression( rng_it, true );
    }
    template<typename RangedIterator>
    auto parse_s_expression_or_closer( RangedIterator& rng_it, cons* outer_cell )
        -> cons*
    {
        return parse_s_expression( rng_it, outer_cell, true );
    }


    template<typename RangedIterator>
    auto parse_s_expression( RangedIterator& rng_it, cons* outer_cell, bool const is_enable_closer )
        -> cons*
    {
        auto s = parse_s_expression( rng_it, is_enable_closer );

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


    template<typename RangedIterator>
    auto parse_s_expression( RangedIterator& rng_it, bool const is_enable_closer )
        -> node*
    {
        skip_space( rng_it );
        expect_not_eof( rng_it );

        if ( *rng_it == '(' ) {
            step_iterator( rng_it );

            // car
            auto s = parse_s_expression_or_closer( rng_it );
            if ( s == nullptr ) {
                return nullptr;
            }

            //
            auto cell = make_node<cons>();
            cell->car = s;

            skip_space( rng_it );
            if ( *rng_it == '.' ) {
                // cons cell
                step_iterator( rng_it );

                parse_s_expression( rng_it, cell );
                return parse_closer_s_expression( rng_it, cell );

            } else {
                // list
                auto last_cell = cell;
                while( auto&& v = parse_s_expression_or_closer( rng_it, last_cell ) ) {
                    last_cell = v;
                }

                return cell;
            }

        } else if ( is_enable_closer &&  *rng_it == ')' ) {
            return parse_closer_s_expression( rng_it );

        } else {
            // atom
            auto a = parse_atom( rng_it );

            if ( !parse_token_separate( rng_it ) ) {
                throw "parse error";
            }

            return a;
        }
    }



    template<typename RangedIterator>
    auto parse_program( RangedIterator& rng_it )
        -> node*
    {
        return parse_s_expression( rng_it );
    }












    auto eval( std::string const& source )
        -> void
    {
        auto rng_it = ranged_iterator<std::string::const_iterator>( source.cbegin(), source.cend() );



        try {
            for (;;) {
                auto s = parse_program( rng_it );

                std::cout << "Result: " << std::endl;
                print2( s );
                std::cout << std::endl;
            }

        } catch( char const* const message ) {
            std::cout << "exception!: " << message << std::endl;
        } catch( reached_to_eof const& e ) {
            std::cout << "reached to eof" << std::endl;
        }

        std::cout << "rest: " << std::string( rng_it.it(), rng_it.end() ) << std::endl;


    }
} // namespace yakkai






int main()
{
    std::string const test_case = R"::(
(list (quote a) (quote b) abc)
(a . b)
()
(add 1 2 3 4)
1
2
3
3.4
+3.2e10
+0E5
.0e2
e3
)::";
/* #10 r 10
# 20 R20
3/10
#10r10/3
*/
    std::cout << test_case << std::endl;


    yakkai::eval( test_case );
}
