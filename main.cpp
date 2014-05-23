#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <cassert>
#include <cctype>
#include <iterator>
#include <stdexcept>


namespace yakkai
{
    // forward
    namespace interpreter
    {
        class scope;
    }

    class reached_to_eof : std::exception
    {
    };




    enum struct node_type
    {
        e_none,

        // list
        e_list,

        // atom
        e_symbol,
        e_string,
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
            : node( node_type::e_list )
            {}

        node* car = nullptr;
        node* cdr = nullptr;
    };

    enum class symbol_type
    {
        e_none,
        e_function,
        e_native_function
    };

    //
    struct symbol : public node
    {
        symbol( std::string const& v, symbol_type const& t = symbol_type::e_none )
            : node( node_type::e_symbol )
            , value( v )
            , sym_type( t )
        {}

        std::string value;
        symbol_type sym_type;
    };


    //
    struct native_function_symbol : public symbol
    {
        explicit native_function_symbol( std::function<node* (node*, std::shared_ptr<interpreter::scope> const&)> const& f )
            : symbol( "native_symbol", symbol_type::e_native_function )
            , f_( f )
            {}

        std::function<node* (node*, std::shared_ptr<interpreter::scope> const&)> f_;
    };




    //
    struct integer_value : public node
    {
        integer_value( long long int const& v )
            : node( node_type::e_integer )
            , value( v )
        {}

        long long int value;
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
        double value;
    };


    auto is_nil( node const* const n )
        -> bool
    {
        if ( n == nullptr ) {
            return true;

        } else if ( n->type == node_type::e_list ) {
            return is_nil( static_cast<cons const* const>( n )->car );

        } else {
            return false;
        }
    }

    auto is_list( node const* const n )
        -> bool
    {
        if ( n == nullptr ) {
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
        if ( n == nullptr ) {
            return false;

        } else if ( n->type == node_type::e_symbol ) {
            return true;

        } else {
            return false;
        }
    }

    auto is_integer( node const* const n )
        -> bool
    {
        if ( n == nullptr ) {
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
        if ( n == nullptr ) {
            return false;

        } else if ( n->type == node_type::e_float ) {
            return true;

        } else {
            return false;
        }
    }

    auto is_function_symbol( node const* const s )
        -> bool
    {
        if ( !is_symbol( s ) ) return false;
        return static_cast<symbol const* const>( s )->sym_type == symbol_type::e_function;
    }

    auto is_native_function_symbol( node const* const s )
        -> bool
    {
        if ( !is_symbol( s ) ) return false;
        return static_cast<symbol const* const>( s )->sym_type == symbol_type::e_native_function;
    }

    auto is_callable_symbol( node const* const s )
        -> bool
    {
        return is_function_symbol( s ) || is_native_function_symbol( s );
    }


#if 0
    auto print( node const* const n, std::size_t indent = 0 )
        -> void
    {
        auto const space = std::string( indent * 2, ' ' );

        if ( is_nil( n ) ) {
            std::cout << space << "nil" << std::endl;
            return;

        } else if ( n->type == node_type::e_list ) {
            auto c = static_cast<cons const* const>( n );
            std::cout << space << "( " << std::endl;
            print( c->car, indent + 1 );
            std::cout << space << "." << std::endl;
            print( c->cdr, indent + 1 );
            std::cout << space << ")" << std::endl;

        } else if ( n->type == node_type::e_symbol ) {
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

        } else if ( n->type == node_type::e_list ) {

            std::cout << "( ";
            auto c = static_cast<cons const* const>( n );
            for(;;) {
                print2( c->car, indent + 1 );
                std::cout << " ";
                if ( is_nil( c->cdr ) ) {
                    break;
                }

                assert( c->cdr->type == node_type::e_list );
                c = static_cast<cons const* const>( c->cdr );
            }
            std::cout << "): e_list";

        } else if ( n->type == node_type::e_symbol ) {
            auto s = static_cast<symbol const* const>( n );
            std::cout << s->value << ": symbol";

        } else if ( n->type == node_type::e_integer ) {
            auto s = static_cast<integer_value const* const>( n );
            std::cout << s->value << ": int";

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
        int radix = 10;
        long long number = 0;

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
                    radix = std::stoi( std::string( base_it.it(), rng_it.it() ) );
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
            number = std::stoll( std::string( base_it.it(), rng_it.it() ), nullptr, radix );
        }
        skip_space( rng_it );

        //
        if ( !is_eof( rng_it ) && ( *rng_it == '.' || *rng_it == 'e' || *rng_it == 'E' ) ) {
            rng_it = begin;
            return nullptr;
        }

        return make_node<integer_value>( number );
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
                // e_list
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






    enum class error_code
    {
        none,
        unexpected
    };

    template<typename T>
    auto parse_one_expression( ranged_iterator<T>& rng_it, error_code& ec )
        -> node*
    {
        ec = error_code::none;

        try {
            return parse_program( rng_it );

        } catch( char const* const message ) {
            std::cout << "exception!: " << message << std::endl;

        } catch( reached_to_eof const& e ) {
            std::cout << "reached to eof" << std::endl;
        }

        ec = error_code::unexpected;
        return nullptr;
    }




    namespace interpreter
    {
        class scope
            : public std::enable_shared_from_this<scope>
        {
        public:
            scope() = default;

            scope( std::weak_ptr<scope> const& p )
                : parent_( p )
            {}

        public:
            auto def_symbol( std::string const& name, node* const n )
                -> node*
            {
                environment_[name] = std::make_pair( n, nullptr );
            }

            auto get_node_at( std::string const& name )
                -> node*
            {
                return std::get<0>( environment_.at( name ) );
            }

            auto get_scope_at( std::string const& name )
                -> std::shared_ptr<scope>
            {
                return std::get<1>( environment_.at( name ) );
            }

        public:
            auto has_parent() const
                -> bool
            {
                return !parent_.expired();
            }

            auto find( std::string const& name )
                -> std::pair<node*, std::shared_ptr<scope>>
            {
                auto&& it = environment_.find( name );
                if ( it != environment_.cend() ) return it->second;

                if ( has_parent() ) return parent_.lock()->find( name );

                return std::make_pair( nullptr, nullptr );
            }

            auto find_node( std::string const& name )
                -> node*
            {
                return std::get<0>( find( name ) );
            }

            auto find_scope( std::string const& name )
                -> std::shared_ptr<scope>
            {
                return std::get<1>( find( name ) );
            }

        public:
            auto make_inner_scope()
                -> std::shared_ptr<scope>
            {
                return std::make_shared<scope>( shared_from_this() );
            }

        private:
            std::weak_ptr<scope> parent_;
            std::map<std::string, std::pair<node*, std::shared_ptr<scope>>> environment_;
        };


        auto as_node( std::pair<node*, std::shared_ptr<scope>> const& p )
            -> node*
        {
            return std::get<0>( p );
        }

        auto as_scope( std::pair<node*, std::shared_ptr<scope>> const& p )
            -> std::shared_ptr<scope> const&
        {
            return std::get<1>( p );
        }


        class machine
        {
        public:
            machine()
                : scope_( std::make_shared<scope>() )
            {
                using namespace std::placeholders;

                //
                def_global_native_function( "add", std::bind( &machine::add, this, _1, _2 ) );
            }

            auto eval( node* const n )
                -> node*
            {
                return eval( n, scope_ );
            }

            auto eval( node* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                if ( is_nil( n ) ) {
                    return nullptr;

                } else if ( n->type == node_type::e_list ) {
                    auto c = static_cast<cons* const>( n );
                    if ( is_symbol( c->car ) ) {
                        // try to function call
                        return function_call( c, current_scope );
                    }
                }

                return n;
            }

        public:
            template<typename F>
            auto def_global_native_function( std::string const& name, F&& f )
                -> node*
            {
                scope_->def_symbol( name, make_node<native_function_symbol>( std::forward<F>( f ) ) );
            }

        private:
            auto function_call( cons* const c, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                assert( is_symbol( c->car ) );
                auto s = static_cast<symbol* const>( c->car );

                // check special function
                // if ( is_special_form() ) {
                // else {
                // check is callable?
                {
                    // look up symbol
                    auto&& p = current_scope->find( s->value );
                    if ( std::get<0>( p ) == nullptr ) {
                        std::cout << "error!!" << std::endl;
                        assert( false );
                    }

                    //
                    auto&& target_node = as_node( p );

                    //
                    if ( is_native_function_symbol( target_node ) ) {
                        auto&& ns = static_cast<native_function_symbol const* const>( target_node );
                        assert( ns->f_ != nullptr );

                        // call native function
                        return (ns->f_)( c->cdr, current_scope );

                    } else {
                        //
                        auto&& new_scope = as_scope( p )->make_inner_scope();
                        std::cout << "not supported" << std::endl;
                    }
                }

                assert( false );
                return nullptr;
            }

        private:
            auto add( node* n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                assert( is_list( n ) );

                node_type nt = node_type::e_integer;
                double result = 0.0;

                cons const* t = static_cast<cons const* const>( n );
                while( !is_nil( t ) ) {
                    auto&& v = eval( t->car );

                    if ( is_integer( v ) ) {
                        auto&& typed_val = static_cast<integer_value const* const>( v );
                        result += typed_val->value;

                    } else if ( is_float( v ) ) {
                        auto&& typed_val = static_cast<float_value const* const>( v );
                        assert( false );

                    } else {
                        // type error
                        assert( false );
                    }

                    assert( is_list( t->cdr ) );
                    t = static_cast<cons const* const>( t->cdr );
                }

                std::cout << "Hellooo: " << result << " / " << (int)nt << std::endl;

                return [&]() -> node* {
                    if ( nt == node_type::e_integer ) {
                         return make_node<integer_value>( result );

                    } else if ( nt == node_type::e_float ) {
                        assert( false );
                    }

                    assert( false );
                    return nullptr;
                }();
            }

        private:
            std::shared_ptr<scope> scope_;
        };
    } // namespace






    auto eval_source_code( std::string const& source )
        -> void
    {
        auto rng_it = ranged_iterator<std::string::const_iterator>( source.cbegin(), source.cend() );

        error_code ec;
        interpreter::machine m;

        //
        for (;;) {
            auto s = parse_one_expression( rng_it, ec );

            std::cout << "Result: " << std::endl;
            print2( s );
            std::cout << std::endl;

            if ( ec == error_code::none ) {
                //
                print2( m.eval( s ) );
            }


            if ( rng_it.it() == rng_it.end() ) {
                break;
            }
        }
    }

} // namespace yakkai






int main()
{
    std::string const test_case = R"::(
(add 1 2 3 4)
(list (quote a) (quote b) abc)
(a . b)
()
1
2
3
3.4
+3.2e10
+0E5
.0e2
e3
nil
)::";
/* #10 r 10
# 20 R20
3/10
#10r10/3
*/
    std::cout << test_case << std::endl;


    yakkai::eval_source_code( test_case );
}
