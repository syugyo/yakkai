#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <cassert>
#include <cctype>
#include <iterator>
#include <stdexcept>
#include <limits>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <typeindex>
#include <array>


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
        e_native_function,
        e_keyword,
        e_string,
        e_integer,
        e_ratio,
        e_float,
        e_complex
    };


    enum class node_attribute
    {
        e_none,
        e_callable
    };



    //
    struct node
    {
        node( node_type const& t, node_attribute const& a = node_attribute::e_none )
            : type( t )
            , attr( a )
            {}

        auto set_attribute( node_attribute const& a )
            -> void
        {
            attr = a;
        }

        node_type type;
        node_attribute attr;
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

    //
    struct symbol : public node
    {
        symbol( std::string const& v )
            : node( node_type::e_symbol )
            , value( v )
        {}

        std::string value;
    };


    //
    struct native_function : public node
    {
        using func_type = std::function<
            node* (cons* const, std::shared_ptr<interpreter::scope> const&)
            >;

        explicit native_function( func_type const& f )
            : node( node_type::e_native_function, node_attribute::e_callable )
            , f_( f )
        {}

        func_type f_;
    };



    struct keyword : public node
    {
        keyword( std::string const& v )
            : node( node_type::e_keyword )
            , value( v )
        {}

        std::string value;
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
            assert( false );

        } else if ( n->type == node_type::e_list ) {
            auto&& c = static_cast<cons const* const>( n );
            return c->car == nullptr && c->cdr == nullptr;

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

#if 0
    auto print( node const* const n, std::size_t indent = 0 )
        -> void
    {
        auto const space = std::string( indent * 2, ' ' );

        if ( is_nil( n ) ) {
            std::cout << "(): unit" << std::endl;
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
            std::cout << "(): unit";

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
        std::cout << sizeof( T ) << std::endl;

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
    auto parse_keyword( RangedIterator& rng_it )
        -> node*
    {
        RangedIterator begin = rng_it;

        if ( ( *rng_it == '&' ) ) {
            step_iterator( rng_it );

            while( !is_eof( rng_it )
                   && ( ( *rng_it >= 'A' && *rng_it <= 'Z' )
                        || ( *rng_it >= 'a' && *rng_it <= 'z' )
                        || ( *rng_it >= '0' && *rng_it <= '9' ) )
                ) {
                step_iterator( rng_it );
            }

            return make_node<keyword>( std::string( begin.it(), rng_it.it() ) );

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

    auto const& nil_object = make_node<cons>();




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

        if ( has_digit || has_float_number ) {
            if ( *rng_it == 'e' || *rng_it == 'E' ) {
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
            }

            return make_node<float_value>( number, exp );

        } else {
            rng_it = begin;
            return nullptr;
        }
    }




    template<typename RangedIterator>
    auto parse_numbers( RangedIterator& rng_it )
        -> node*
    {
        if ( auto i = parse_integer( rng_it ) ) {
            return i;

        } else if ( auto f = parse_float( rng_it ) ) {
            return f;

        } else {
            return nullptr;
        }
    }


    template<typename RangedIterator>
    auto parse_atom( RangedIterator& rng_it )
        -> node*
    {
        if ( auto k = parse_keyword( rng_it ) ) {
            return k;

        } else if ( auto s = parse_symbol( rng_it ) ) {
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
            outer_cell->cdr = nil_object;
            return nil_object;

        } else {
            auto inner_cell = make_node<cons>();
            inner_cell->car = s;
            inner_cell->cdr = nil_object;

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
                return nil_object;
            }

            //
            auto cell = make_node<cons>();
            cell->car = s;
            cell->cdr = nil_object;

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
                    if ( is_nil( v ) ) break;
                    last_cell = v;
                }

                return cell;
            }

        } else if ( is_enable_closer && *rng_it == ')' ) {
            return parse_closer_s_expression( rng_it, nullptr );

        } else {
            // atom
            auto a = parse_atom( rng_it );
            assert( a != nullptr );

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
        syntax,
        unexpected,
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

            ec = error_code::syntax;
            return nil_object;

        } catch( reached_to_eof const& e ) {
            std::cout << "reached to eof" << std::endl;
        }

        ec = error_code::unexpected;
        return nil_object;
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
            auto def_symbol( std::string const& name, node* const n, std::shared_ptr<scope> const& s = nullptr )
                -> node*
            {
                environment_[name] = std::make_pair( n, s );

                return n;
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
                -> std::tuple<node*, std::shared_ptr<scope>>
            {
                auto&& it = environment_.find( name );
                if ( it != environment_.cend() ) return it->second;

                if ( has_parent() ) return parent_.lock()->find( name );

                return std::forward_as_tuple( nullptr, nullptr );
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


        template<typename S>
        auto as_node( std::tuple<node*, std::shared_ptr<S>> const& p )
            -> node*
        {
            return std::get<0>( p );
        }

        template<typename S>
        auto as_node( std::tuple<node const*, std::shared_ptr<S>> const& p )
            -> node const*
        {
            return std::get<0>( p );
        }

        template<typename N>
        auto as_scope( std::tuple<N*, std::shared_ptr<scope>> const& p )
            -> std::shared_ptr<scope> const&
        {
            return std::get<1>( p );
        }





        class page
        {
            static constexpr std::size_t block_max = 4096;

        public:
            using pointer_type = unsigned char*;

        public:
            page( std::size_t const& block_size )
                : block_size_( block_size )
                , total_size_( block_size < block_max ? block_max : block_size )
                , capacity_num_( block_size < block_max ? ( block_max / block_size ) : 1 )
                , object_num_( 0 )
                , data_( new unsigned char[total_size_] )
                , free_bitmap_{}
                , mark_bitmap_{}
            {
                assert( block_size >= 4 );
            }

            page( std::size_t const& block_size, std::function<void (void*)> const& deleter )
                : block_size_( block_size )
                , total_size_( block_size < block_max ? block_max : block_size )
                , capacity_num_( block_size < block_max ? ( block_max / block_size ) : 1 )
                , object_num_( 0 )
                , data_( new unsigned char[total_size_] )
                , free_bitmap_{}
                , mark_bitmap_{}
                , deleter_( deleter )
            {}

            page( page const& ) = delete;
            page( page&& ) = delete;

            ~page()
            {
                destruct_objects();
                delete[] data_;
            }

        public:
            template<typename T, typename... Args>
            inline auto construct_object( Args&&... args )
                -> T*
            {
                auto p = allocate();
                if ( p == nullptr ) return nullptr;

                return new( p ) T( std::forward<Args>( args )... );
            }

        private:
            auto destruct_object( std::size_t const& i )
                -> void
            {
                void* object = get_block_from_index( i );

                if ( deleter_ ) {
                    deleter_( object );
                }
                unmark( i );

                //
                free( i );
            }

            auto destruct_objects( bool do_skip_mask_check = true )
                -> std::size_t
            {
                std::size_t n = 0;
                for( std::size_t i=0; i<capacity_num_; ++i ) {
                    if ( is_used( i ) ) {
                        if ( !is_marked( i ) || do_skip_mask_check ) {
                            std::cout << "?????????????????" << std::endl;
                            destruct_object( i );
                            ++n;
                        }
                    }
                }

                return n;
            }

        private:
            auto allocate()
                -> pointer_type
            {
                if ( is_full() ) return nullptr;

                auto const bi = find_free_block_index();
                if ( bi == std::numeric_limits<std::size_t>::max() ) {
                    return nullptr;
                }

                mark_as_used( bi );
                ++object_num_;

                return get_block_from_index( bi );
            }

            auto free( std::size_t const& i )
                -> void
            {
                if ( is_used( i ) ) {
                    // this pointer is marked as used
                    mark_as_free( i );
                    --object_num_;
                } else {
                    assert( false && "invalid pointer was given..." );
                }
            }

        public:
            inline auto is_full() const
                -> bool
            {
                return object_num_ >= capacity_num_;
            }

            inline auto is_included( void const* const p ) const
                -> bool
            {
                std::cout << (void*)data_ << " <= " << (void*)p << " < " << (void*)( data_ + total_size_ ) << std::endl;
                return p >= data_ && p < ( data_ + total_size_ );
            }

        public:
            inline auto operator<( page const& rhs ) const
                -> bool
            {
                return data_ < rhs.data_;
            }

            inline auto operator<( void const* const p ) const
                -> bool
            {
                return data_ < p;
            }

            friend auto operator<<( std::ostream& os, page const& rhs )
                -> std::ostream&
            {
                std::cout << "memory: " << (void*)rhs.data_;
                return os;
            }

        public:
            template<typename T>
            auto mark( T const* const np )
                -> void
            {
                auto const i = get_index_from_pointer( np );
                bit_set( i, mark_bitmap_ );
            }

            inline auto sweep()
                -> std::size_t
            {
                return destruct_objects( false );
            }

        private:
            inline auto is_marked( std::size_t const& i ) const
                -> bool
            {
                return bit_test( i, mark_bitmap_ );
            }

            auto unmark( std::size_t const& i )
                -> void
            {
                bit_clear( i, mark_bitmap_ );
            }

        private:
            inline auto is_used( std::size_t const& i ) const
                -> bool
            {
                return bit_test( i, free_bitmap_ );
            }

            auto mark_as_used( std::size_t const& i )
                -> void
            {
                auto const array_index = i / 64;
                auto const bit_index = i % 64;

                free_bitmap_[array_index] |= static_cast<std::uint64_t>( 1 ) << ( 64 - bit_index - 1 );
            }

            auto mark_as_free( std::size_t const& i )
                -> void
            {
                auto const array_index = i / 64;
                auto const bit_index = i % 64;

                free_bitmap_[array_index] &= ~( static_cast<std::uint64_t>( 1 ) << ( 64 - bit_index - 1 ) );
            }

        private:
            inline auto get_block_from_index( std::size_t const& i )
                -> pointer_type
            {
                return data_ + ( block_size_ * i );
            }

            template<typename T>
            inline auto get_index_from_pointer( T const* const np ) const
                -> std::size_t
            {
                auto&& p = reinterpret_cast<unsigned char const*>( np );
                assert( p >= data_ && p < data_ + total_size_ );

                return static_cast<std::size_t>( p - data_ ) / block_size_;
            }

            auto find_free_block_index() const
                -> std::size_t
            {
                auto const test8 = []( std::uint8_t const& n ) -> std::size_t {
                    for( int i=0; i<8; ++i ) {
                        if ( ( n & ( 1 << ( 8 - i - 1 ) ) ) == 0 ) return i;
                    }

                    assert( false );
                };

                auto const test16 = [&test8]( std::uint16_t const& n ) -> std::size_t {
                    std::uint8_t const h = ( n & 0xff00 ) >> 8;
                    if ( h != std::numeric_limits<std::uint8_t>::max() ) {
                        return 8 * 0 + test8( h );
                    }

                    std::uint8_t const l = ( n & 0x00ff );
                    if ( l!= std::numeric_limits<std::uint8_t>::max() ) {
                        return 8 * 1 + test8( l );
                    }

                    assert( false );
                };

                auto const test32 = [&test16]( std::uint32_t const& n ) -> std::size_t {
                    std::uint16_t const h = ( n & 0xffff0000 ) >> 16;
                    if ( h != std::numeric_limits<std::uint16_t>::max() ) {
                        return 16 * 0 + test16( h );
                    }

                    std::uint16_t const l = ( n & 0x0000ffff );
                    if ( l != std::numeric_limits<std::uint16_t>::max() ) {
                        return 16 * 1 + test16( l );
                    }

                    assert( false );
                };

                std::size_t i = 0;
                for( auto&& n : free_bitmap_ ) {
                    if ( n != std::numeric_limits<std::uint64_t>::max() ) {
                        std::uint32_t const h = ( n & 0xffffffff00000000 ) >> 32;
                        if ( h != std::numeric_limits<std::uint32_t>::max() ) {
                            return 64 * i + 32 * 0 + test32( h );
                        }

                        std::uint32_t const l = ( n & 0x00000000ffffffff );
                        if ( l != std::numeric_limits<std::uint32_t>::max() ) {
                            return 64 * i + 32 * 1 + test32( l );
                        }
                    }

                    ++i;
                }

                // failed
                return std::numeric_limits<std::size_t>::max();
            }

        private:
            template<typename T>
            auto bit_set( std::size_t const& i, T& bitmap) const
                -> void
            {
                auto const array_index = i / 64;
                auto const bit_index = i % 64;

                bitmap[array_index] |= static_cast<std::uint64_t>( 1 ) << ( 64 - bit_index - 1 );
            }

            template<typename T>
            auto bit_clear( std::size_t const& i, T& bitmap) const
                -> void
            {
                auto const array_index = i / 64;
                auto const bit_index = i % 64;

                bitmap[array_index] &= ~( static_cast<std::uint64_t>( 1 ) << ( 64 - bit_index - 1 ) );
            }

            template<typename T>
            auto bit_test( std::size_t const& i, T& bitmap) const
                -> bool
            {
                auto const array_index = i / 64;
                auto const bit_index = i % 64;

                return ( bitmap[array_index] & ( static_cast<std::uint64_t>( 1 ) << ( 64 - bit_index - 1 ) ) ) != 0;
            }

        private:
            std::size_t block_size_;
            std::size_t total_size_;
            std::size_t capacity_num_;
            std::size_t object_num_;

            unsigned char* data_;
            std::array<std::uint64_t, block_max/4> free_bitmap_;
            std::array<std::uint64_t, block_max/4> mark_bitmap_;

            std::function<void (void*)> deleter_;
        };


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


        //
        class gc
        {
        private:
            constexpr static std::size_t const MinAlign = 4;

        public:
            gc( void volatile const* const p )
                : stack_begin_( reinterpret_cast<std::uintptr_t>( p ) )
            {}

        public:
            template<typename T, typename... Args>
            auto make_object( Args&&... args )
                -> T*
            {
                auto const block_size = lcm( sizeof( T ), alignof( T ) );

                prepare_page<T>( block_size );

                {
                    auto p = try_to_allocate<T>( block_size, std::forward<Args>( args )... );
                    if ( p != nullptr ) return p;   // Succeeded!
                }

                // if reached to this flow, page table is full. So run Garbage collector!
                auto&& freed_num = full_collect();
                if ( freed_num < 100 ) {
                    add_page<T>( block_size );
                }

                // retry
                {
                    auto p = try_to_allocate<T>( block_size, std::forward<Args>( args )... );
                    if ( p != nullptr ) return p;   // Succeeded!
                }

                // if reached to this flow, totally failed...
                assert( false );
            }

        private:
            template<typename T>
            auto prepare_page( std::size_t const& block_size )
                -> void
            {
                auto range = pages_.equal_range( typeid( T ) );
                if ( std::get<0>( range ) == std::get<1>( range ) ) {
                    add_page<T>( block_size );
                }
            }

            template<typename T>
            auto add_page( std::size_t const& block_size )
                -> void
            {
                auto p = std::make_shared<page>( block_size, []( void* p ) {
                        // TODO: check default destructable
                        static_cast<T*>( p )->~T();
                    } );

                pages_.emplace( typeid( T ), p );
                sorted_pages_.emplace_back( p );

                std::sort( sorted_pages_.begin(), sorted_pages_.end(), []( std::shared_ptr<page> const& r, std::shared_ptr<page> const& l ) {
                        return *r < *l;
                    } );

                std::cout << "=======" << std::endl;
                for( auto&& p : sorted_pages_ ) {
                    std::cout << *p << std::endl;
                }
            }

            template<typename T, typename... Args>
            auto try_to_allocate( std::size_t const& block_size, Args&&... args )
                -> T*
            {
                auto const& range = pages_.equal_range( typeid( T ) );

                for( auto it = std::get<0>( range ); it != std::get<1>( range ); ++it ) {
                    auto&& p = it->second;

                    if ( !p->is_full() ) {
                        auto const object = p->construct_object<T>( std::forward<Args>( args )... );
                        if ( object == nullptr ) {
                            assert( false && "" );
                        }

                        // succeeded!!
                        return object;
                    }
                }

                return nullptr;
            }

        public:
            auto set_base_stack_address( void volatile const* const p )
                -> void
            {
                stack_begin_ = reinterpret_cast<std::uintptr_t>( p );
            }

        private:
            auto full_collect()
                -> std::size_t
            {
                mark_stack();
                // TODO: mark_registers();
                if ( custom_marker_ ){
                    using namespace std::placeholders;
                    custom_marker_( std::bind( &gc::mark_object, this, _1 ) );
                }

                return sweep();
            }

        private:
            auto mark_stack()
                -> void
            {
                //
                alignas( void* ) int volatile _dummy_stack_end_object;

                auto high = stack_begin_;
                auto low = reinterpret_cast<std::uintptr_t>( &_dummy_stack_end_object );
                if ( high < low ) std::swap( high, low );

                //std::cout << "" << low << "/" << high << std::endl;

                // ub...?
                for( auto p=reinterpret_cast<node** const>( low ); p<reinterpret_cast<node** const>( high ); ++p ) {
                    mark_object( *p );
                }
            }

            auto mark_object( node* n )
                -> void
            {
                //std::cout << "??? -> " << n << std::endl;

                // binary search
                std::size_t begin = 0, end = sorted_pages_.size(), point = 0, prev_point = 0;
                for(;;) {
                    point = std::max<std::size_t>( 0, std::min<std::size_t>( end - 1, point + ( end - begin ) / 2 ) );
                    if ( point == prev_point ) break;

                    if ( *sorted_pages_[point] < n ) {
                        begin = point;
                    } else {
                        end = point;
                    }
                    //std::cout << "point: " << point << " / " << sorted_pages_.size() << "[" << begin << ", " << end << ")" << std::endl;

                    prev_point = point;
                }

                auto&& target_page = sorted_pages_[point];
                if ( target_page->is_included( n ) ) {
                    target_page->mark( n );
                    //std::cout << "!!!!!!!! pp: " << (void*)n << std::endl;
                }
            }

        private:
            auto sweep()
                -> std::size_t
            {
                std::size_t total_collected_num = 0;
                for( auto&& p : sorted_pages_ ) {
                    total_collected_num += p->sweep();
                }
                return total_collected_num;
            }

        private:
            std::uintptr_t stack_begin_;
            std::function<void (std::function<void (node*)> const&)> custom_marker_;

            std::unordered_multimap<std::type_index, std::shared_ptr<page>> pages_;
            std::vector<std::shared_ptr<page>> sorted_pages_;
        };


        class machine
        {
        public:
            machine( void volatile const* const p )
                : scope_( std::make_shared<scope>() )
                , gc_( p )
            {
                using namespace std::placeholders;

                //
                def_global_native_function( "deffun", std::bind( &machine::define_function, this, _1, _2 ) );
                def_global_native_function( "add", std::bind( &machine::add, this, _1, _2 ) );
                def_global_native_function( "multiply", std::bind( &machine::multiply, this, _1, _2 ) );

                def_global_native_function( "lambda", std::bind( &machine::make_lambda, this, _1, _2 ) );
                def_global_native_function( "progn", std::bind( &machine::progn, this, _1, _2 ) );

                def_global_native_function( "quote", std::bind( &machine::quote, this, _1, _2 ) );

                def_global_native_function( "if", std::bind( &machine::if_function, this, _1, _2 ) );
                // def_global_native_function( "car", std::bind( &machine::car, this, _1 ) );
                // def_global_native_function( "cdr", std::bind( &machine::cdr, this, _1 ) );
            }

        public:
            auto eval( node* const n )
                -> node*
            {
                return as_node( eval( n, scope_ ) );
            }

        private:
            auto eval( node* const n, std::shared_ptr<scope> const& current_scope )
                -> std::tuple<node*, std::shared_ptr<scope>>
            {
                if ( is_nil( n ) ) {
                    return std::forward_as_tuple( nil_object, current_scope );

                } else if ( n->type == node_type::e_list ) {
                    // list is given
                    auto c = static_cast<cons* const>( n );

                    if ( is_nil( c ) ) {
                        assert( false && "nil was given..." );
                    }

                    // try to call(function/macro)
                    auto&& head_p = eval( c->car, current_scope );
                    if ( is_callable( as_node( head_p ) ) ) {
                        assert( is_list( c->cdr ) );

                        return std::forward_as_tuple(
                            call_function(
                                static_cast<symbol const* const>( as_node( head_p ) ),
                                static_cast<cons* const>( c->cdr ),
                                as_scope( head_p ),
                                current_scope
                                ),
                            current_scope
                            );

                    } else {
                        // TODO: check that is macro given...
                        print2( as_node( head_p ) );
                        assert( false && "reciever is not callable..." );
                    }

                } else if ( n->type == node_type::e_symbol ) {
                    auto&& reciever_symbol = static_cast<symbol const* const>( n );

                    std::cout << "look_up: " << reciever_symbol->value << std::endl;
                    auto&& p = current_scope->find( reciever_symbol->value );

                    auto&& target_node = as_node( p );
                    if ( target_node == nullptr ) {
                        assert( false && "symbol was not found" );
                    }

                    return p;
                }

                // otherwise, return
                return std::forward_as_tuple( n, current_scope );
            }

        public:
            template<typename F>
            auto def_global_native_function( std::string const& name, F&& f )
                -> node*
            {
                return scope_->def_symbol( name, make_node<native_function>( std::forward<F>( f ) ), scope_->make_inner_scope() );
            }

        private:
            auto call_function(
                node const* const reciever,
                cons* const args,
                std::shared_ptr<scope> const& target_scope,
                std::shared_ptr<scope> const& current_scope
                )
                -> node*
            {
                if ( is_native_function( reciever ) ) {
                    auto&& ns = static_cast<native_function const* const>( reciever );
                    assert( ns->f_ != nullptr );

                    // call native function
                    return (ns->f_)( args, current_scope );

                } else {
                    //
                    assert( target_scope != nullptr );

                    //
                    // eval args( rewrite list )
                    {
                        auto head = args;
                        while( !is_nil( head ) ) {
                            auto&& p = as_node( eval( head->car, current_scope ) );
                            std::swap( head->car, p );

                            assert( is_list( head->cdr ) );
                            head = static_cast<cons* const>( head->cdr );
                        }
                    }

                    auto&& new_scope = target_scope->make_inner_scope();

                    //
                    assert( is_list( reciever ) && !is_nil( reciever ) );
                    auto params = static_cast<cons const* const>( reciever )->car;
                    assert( !is_nil( params ) );

                    //
                    auto function_body = static_cast<cons const* const>( reciever )->cdr;

                    cons const* parameter_head = static_cast<cons const* const>( params );
                    cons const* argument_head = args;

                    bool reached_to_last_parameter = false;

                    // map argument/parameter
                    // TODO: check length of parameter/arguments & optional keyword
                    assert( is_list( argument_head ) && is_list( parameter_head ) );
                    while( !is_nil( parameter_head ) ) {
                        // parameters
                        if ( is_symbol( parameter_head->car ) ) {
                            auto&& parameter_symbol = static_cast<symbol const* const>( parameter_head->car );
                            std::cout << "parameter : " << parameter_symbol->value << std::endl;

                            // set argument value
                            new_scope->def_symbol(
                                parameter_symbol->value,
                                argument_head->car
                                );

                        } else if ( is_keyword( parameter_head->car ) ) {
                            assert( false && "keyword was not supported yet" );

                        } else {
                            assert( false && "" );
                        }


                        parameter_head = static_cast<cons const* const>( parameter_head->cdr );

                        // argument
                        argument_head = static_cast<cons const* const>( argument_head->cdr );
                    }

                    assert( is_list( function_body ) );
                    return eval_prog_n(
                        static_cast<cons const*>( function_body ),
                        new_scope
                        );
                }

                assert( false );
                return nullptr;
            }

            auto eval_prog_n(
                cons const* const prog,
                std::shared_ptr<scope> const& target_scope
                )
                -> node*
            {
                node* last_value = nil_object;

                cons const* head = prog;
                while( !is_nil( head ) ) {
                    auto&& ret = eval( head->car, target_scope );
                    last_value = as_node( ret );

                    head = static_cast<cons const*>( head->cdr );
                }

                return last_value;
            }

        private:
            auto add( cons* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                node_type nt = node_type::e_integer;
                double result = 0.0;

                cons const* t = static_cast<cons const* const>( n );
                while( !is_nil( t ) ) {
                    auto&& v = as_node( eval( t->car, current_scope ) );

                    if ( is_integer( v ) ) {
                        auto&& typed_val = static_cast<integer_value const* const>( v );
                        result += typed_val->value;

                    } else if ( is_float( v ) ) {
                        auto&& typed_val = static_cast<float_value const* const>( v );
                        assert( false );

                    } else {
                        // type error
                        std::cout << "!!! type error" << std::endl;
                        print2( v );
                        assert( false );
                    }

                    assert( is_list( t->cdr ) );
                    t = static_cast<cons const* const>( t->cdr );
                }

                return [&]() -> node* {
                    if ( nt == node_type::e_integer ) {
                         return gc_.make_object<integer_value>( result );

                    } else if ( nt == node_type::e_float ) {
                        assert( false );
                    }

                    assert( false );
                    return nullptr;
                }();
            }

            auto multiply( cons* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                node_type nt = node_type::e_integer;
                double result = 1.0;

                cons const* t = static_cast<cons const* const>( n );
                while( !is_nil( t ) ) {
                    auto&& v = as_node( eval( t->car, current_scope ) );

                    if ( is_integer( v ) ) {
                        auto&& typed_val = static_cast<integer_value const* const>( v );
                        result *= typed_val->value;

                    } else if ( is_float( v ) ) {
                        auto&& typed_val = static_cast<float_value const* const>( v );
                        assert( false );

                    } else {
                        // type error
                        std::cout << "!!! type error" << std::endl;
                        print2( v );
                        assert( false );
                    }

                    assert( is_list( t->cdr ) );
                    t = static_cast<cons const* const>( t->cdr );
                }

                return [&]() -> node* {
                    if ( nt == node_type::e_integer ) {
                         return gc_.make_object<integer_value>( result );

                    } else if ( nt == node_type::e_float ) {
                        assert( false );
                    }

                    assert( false );
                    return nullptr;
                }();
            }

            auto quote( cons* const n, std::shared_ptr<scope> const& )
                -> node*
            {
                assert( !is_nil( n ) );

                return n;
            }

            auto define_function( cons* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                assert( !is_nil( n ) );

                // check function name
                cons const* const first = n;
                if ( !is_symbol( first->car ) ) {
                    assert( false && "function name must be symbol" );
                }
                symbol const* const function_name_symbol = static_cast<symbol const* const>( first->car );

                // check argument
                node* const second_n = first->cdr;
                if ( !is_list( second_n ) || is_nil( second_n ) ) {
                    assert( false && "missing lambda argument" );
                }

                node* const lambda_form
                    = make_lambda( static_cast<cons* const>( second_n ), current_scope );

                std::cout << "define function !> " << function_name_symbol->value << std::endl;
                current_scope->def_symbol( function_name_symbol->value, lambda_form, current_scope->make_inner_scope() );

                return lambda_form;
            }

            auto make_lambda( cons* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                assert( !is_nil( n ) );

                // check argument
                cons* const first_n = n;
                if ( !is_list( first_n ) || is_nil( first_n ) ) {
                    assert( false && "missing lambda argument" );
                }

                cons* const lambda_form = static_cast<cons* const>( first_n );
                if ( !is_list( lambda_form->car ) ) {
                    assert( false && "function argument must be list" );
                }

                // set lambda form as callable!
                lambda_form->set_attribute( node_attribute::e_callable );

                return lambda_form;
            }

            auto progn( cons const* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                return eval_prog_n( n, current_scope );
            }

            auto if_function( cons const* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                assert( !is_nil( n ) );

                cons const* const first = n;

                assert( is_list( first->cdr ) );
                cons const* const second = static_cast<cons const* const>( first->cdr );
                if ( is_nil( second ) ) {
                    assert( false && "few arguments for if" );
                }

                auto&& condition = as_node( eval( first->car, current_scope ) );

                if( !is_nil( condition ) ) {
                    // then
                    return as_node( eval( second->car, current_scope ) );

                } else {
                    // else
                    if ( is_nil( second->cdr ) ) {
                        return second->cdr;

                    } else {
                        assert( is_list( second->cdr ) );
                        return as_node( eval( static_cast<cons const* const>( second->cdr )->car, current_scope ) );
                    }
                }
            }

        private:
            std::shared_ptr<scope> scope_;
            gc gc_;
        };

    } // namespace






    auto eval_source_code( std::string const& source )
        -> void
    {
        auto rng_it = ranged_iterator<std::string::const_iterator>( source.cbegin(), source.cend() );

        // make dummy object into stack to get stack address...
        alignas( void* ) int volatile _dummy_stack_start_object;

        error_code ec;
        interpreter::machine m( &_dummy_stack_start_object );

        //
        for (;;) {
            auto s = parse_one_expression( rng_it, ec );

            std::cout << "<=== section ===>" << std::endl;
            std::cout << "Parsed Result #=> ";
            print2( s );

            if ( ec == error_code::none ) {
                //
                auto&& e = m.eval( s );
                std::cout << "Evaled Result #=> ";
                print2( e );
                std::cout << std::endl;

            } else if ( ec == error_code::syntax ) {
                break;
            }


            if ( rng_it.it() == rng_it.end() ) {
                break;
            }
        }
    }

} // namespace yakkai

#if 0
void test( yakkai::interpreter::gc& g )
{
    struct A
    {
        int ababa[128];

        A( int i )
            : ababa{ i }
        {}

        ~A()
        {
            std::cout << "destructed: " << ababa[0] << std::endl;
        }

        void f() const
        {
            std::cout << "f: " << ababa[0] << std::endl;
        }
    };

    A* p = nullptr;
    A* oldp = nullptr;
    for( int i=0; i<10; ++i ) {
        p = g.allocate<A>( i );
        if ( oldp != nullptr ) {
            oldp->f();
        }

        std::cout << (void*)p << " and " << (void*)oldp << std::endl;

        oldp = p;
    }

    std::cout << "pointer: " << sizeof( A* ) << "/" << alignof( A* )<< std::endl;
}
#endif


int main()
{


    std::string const test_case = R"::(



(add 1 2 3 4)
(deffun tasu (a b) (add a b))
(tasu 1 2)
(tasu 1 (tasu 2 3))

(tasu 10 (tasu 1 (tasu 2 3)))

(tasu 1 (tasu 2103 1))

(progn 1 2 3)

(progn (add 1 2) 2 3)

(lambda (x) (multiply x x))

((lambda (x) (multiply x x)) 9)

(if () 1)
(if 1 72)

(if () 1 2)
(if 1 2 3)

(if 1 (progn 1 2 3) (progn 2 2 4))
(if () (progn 1 2 3) (progn 2 2 4))
(if () (progn 1 2 3) (progn (tasu 1 2) (tasu 2 4)))

(quote b)

(deffun list (&rest objects) objects)
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

#if 0
    // make dummy object into stack to get stack address...
    alignas( void*) int volatile _dummy_stack_start_object;

    {
        yakkai::interpreter::gc g( &_dummy_stack_start_object );
        test( g );
    }
#endif

#if 0
    yakkai::interpreter::page p( 24 );

    for( int i=0; i<10000000; ++i ) {
        auto const index = p.find_free_block_index();
        if ( index == std::numeric_limits<std::size_t>::max() ) {
            break;
        }
        std::cout << index << std::endl;
        p.mark_as_used( index );
    }
#endif
}
