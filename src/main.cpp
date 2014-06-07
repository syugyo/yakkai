#include <iostream>
#include <string>
#include <memory>
#include <cassert>

#include "yakkai/memory/gc.hpp"
#include "yakkai/interpreter/machine.hpp"
#include "yakkai/syntax/parser.hpp"

#include "yakkai/util/ranged_iterator.hpp"
#include "yakkai/util/printer.hpp"


enum class error_code
{
    none,
    syntax,
    unexpected,
    reached_to_eof
};


template<typename Parser, typename It>
auto parse_one_expression( Parser& p, It& rng_it, error_code& ec )
    -> yakkai::node*
{
    // reset error code
    ec = error_code::none;

    try {
        return p.parse_s_expression( rng_it );

    } catch( char const* const message ) {
        std::cout << "exception!: " << message << std::endl;

        ec = error_code::syntax;
        return nullptr;

    } catch( yakkai::reached_to_eof const& e ) {
        std::cout << "reached to eof" << std::endl;
        ec = error_code::reached_to_eof;
        return nullptr;
    }

    ec = error_code::unexpected;
    return nullptr;
}


//
auto eval_source_code(
    int const volatile* const volatile stack_base,
    std::string const& source
    )
    -> void
{
    using namespace yakkai;
    using itearator_t = ranged_iterator<std::string::const_iterator>;

    auto rng_it = itearator_t( source.cbegin(), source.cend() );

    error_code ec;

    //
    auto&& gc = std::make_shared<memory::gc>( stack_base );

    //
    syntax::s_exp_parser<itearator_t, memory::gc> p( gc );
    interpreter::machine<memory::gc> m( gc );

    //
    for (;;) {
        auto s = parse_one_expression( p, rng_it, ec );

        std::cout << "<=== section ===>" << std::endl;
        std::cout << "Parsed Result #=> ";
        print_node( s );

        if ( ec == error_code::none ) {
            //
            auto&& e = m.eval( s );
            std::cout << "Evaled Result #=> ";
            print_node( e );
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


//
int repl(
    int const volatile* const volatile stack_base
    )
{
    using namespace yakkai;
    using itearator_t = ranged_iterator<std::string::const_iterator>;

    auto&& gc = std::make_shared<memory::gc>( stack_base );

    syntax::s_exp_parser<itearator_t, memory::gc> p( gc );
    interpreter::machine<memory::gc> m( gc );

    // REPL loop
    std::string input;

    for(;;) {
        // Read
        std::cout << "in > ";

        std::getline( std::cin, input );

        //
        auto rng_it = itearator_t( input.cbegin(), input.cend() );
        while( rng_it.it() != rng_it.end() ) {
            error_code error;
            auto s = parse_one_expression( p, rng_it, error );

            // std::cout << "<=== section ===>" << std::endl;
            // std::cout << "Parsed Result #=> ";
            // print_node( s );

            if ( error == error_code::none ) {
                //
                auto&& e = m.eval( s );
                print_node( e );

            } else if ( error == error_code::syntax ) {
                std::cout << "syntax error" << std::endl;
                break;

            } else if ( error == error_code::reached_to_eof ) {
                std::cout << "reached to eof" << std::endl;
                break;

            } else {
                assert( false );
            }
        }
    }
}


template<typename F, typename... Args>
auto exec_with_stack_base( F&& f, Args&&... args )
    -> decltype( ( std::forward<F>( f ) )( std::declval<int volatile*>(), std::forward<Args>( args )... ) )
{
    // make dummy object into stack to get stack address...
    // __attribute__( (aligned(sizeof( void* ))) ) int volatile _dummy_stack_start_object;
    alignas( void* ) int volatile _dummy_stack_start_object;

    return ( std::forward<F>( f ) )( &_dummy_stack_start_object, std::forward<Args>( args )... );
}


int main()
{
    // exec_with_stack_base( repl );

#if 1
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
(quote (1 2 3))

(deffun list (&rest objects) objects)
(list (quote a) (quote b) 123)

(car ())
(car (quote (1 2 3)))
(car (quote ((1 2) 3)))

(cdr (quote (1 2)))
(cdr ())
(cdr (quote (1 2 3)))

(car (cdr (quote (1 2 3))))

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
    exec_with_stack_base( eval_source_code, test_case );
#endif
}
