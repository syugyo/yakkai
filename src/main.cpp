#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <cassert>
#include <cctype>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <typeindex>


namespace yakkai
{
    class reached_to_eof : std::exception
    {
    };
}

#include "yakkai/nodes.hpp"

namespace yakkai
{
    auto print2( node const* const n, std::size_t indent = 0 )
        -> void
    {
        auto const space = std::string( indent * 2, ' ' );

        if ( n == nullptr ) {
            return;
        }

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
            std::cout << debug_string( n->type ) << " : !!Unknown!!";
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

}

#include "yakkai/syntax.hpp"

namespace yakkai {







    enum class error_code
    {
        none,
        syntax,
        unexpected,
        reached_to_eof
    };

    template<typename T, typename GC>
    auto parse_one_expression( ranged_iterator<T>& rng_it, GC const& gc, error_code& ec )
        -> node*
    {
        ec = error_code::none;

        try {
            return parse_program( rng_it, gc );

        } catch( char const* const message ) {
            std::cout << "exception!: " << message << std::endl;

            ec = error_code::syntax;
            return nullptr;

        } catch( reached_to_eof const& e ) {
            std::cout << "reached to eof" << std::endl;
            ec = error_code::reached_to_eof;
            return nullptr;
        }

        ec = error_code::unexpected;
        return nullptr;
    }
}

#include "yakkai/gc.hpp"
#include "yakkai/interpreter.hpp"






namespace yakkai
{
    auto test( std::string const& source, int const volatile* const volatile stack_base )
        -> void
    {
        auto rng_it = ranged_iterator<std::string::const_iterator>( source.cbegin(), source.cend() );

        error_code ec;

        auto&& gc = std::make_shared<memory::gc>( stack_base );
        interpreter::machine<memory::gc> m( gc );

        //
        for (;;) {
            auto s = parse_one_expression( rng_it, gc, ec );

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
                std::cout << "syntax error" << std::endl;
                break;

            } else if ( ec == error_code::reached_to_eof ) {
                std::cout << "reached to eof" << std::endl;
                break;

            } else {
                assert( false );
            }


            if ( rng_it.it() == rng_it.end() ) {
                break;
            }
        }
    }

    auto eval_source_code( std::string const& source )
        -> void
    {
        // make dummy object into stack to get stack address...
        // __attribute__( (aligned(sizeof( void* ))) ) int volatile _dummy_stack_start_object;
        alignas( void* ) int volatile _dummy_stack_start_object;

        test( source, &_dummy_stack_start_object );
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
(list (quote a) (quote b) 123)
(quote (1 . 2))
()
1
2
3
3.4
+3.2e10
+0E5
.0e2
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
