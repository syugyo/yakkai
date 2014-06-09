#pragma once

#include <memory>
#include <map>
#include <cassert>

#include <iostream>

#include "node.hpp"
#include "scope.hpp"
#include "../node.hpp"
#include "../static_context.hpp"
#include "../util/printer.hpp"


namespace yakkai
{
    namespace interpreter
    {
        template<typename GC>
        class machine
        {
        public:
            machine( std::shared_ptr<GC> const& gc )
                : scope_( std::make_shared<scope>() )
                , gc_( gc )
            {
                using namespace std::placeholders;

                gc_->cha( std::bind( &machine::mark_scoped_value, this, _1 ) );

                //
                def_global_native_function( "deffun", std::bind( &machine::define_function, this, _1, _2 ) );
                def_global_native_function( "add", std::bind( &machine::add, this, _1, _2 ) );
                def_global_native_function( "multiply", std::bind( &machine::multiply, this, _1, _2 ) );

                def_global_native_function( "lambda", std::bind( &machine::make_lambda, this, _1, _2 ) );
                def_global_native_function( "progn", std::bind( &machine::progn, this, _1, _2 ) );

                def_global_native_function( "quote", std::bind( &machine::quote, this, _1, _2 ) );

                def_global_native_function( "if", std::bind( &machine::if_function, this, _1, _2 ) );
                def_global_native_function( "car", std::bind( &machine::car_func, this, _1, _2 ) );
                def_global_native_function( "cdr", std::bind( &machine::cdr_func, this, _1, _2) );
            }

        private:
            auto mark_scoped_value( std::function<void (node*)> const& marker )
                -> void
            {

                std::cout << "ababa" << std::endl;
                scope_->f( marker );
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
                    return std::forward_as_tuple( static_context::nil_object, current_scope );

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
                        print_node( as_node( head_p ) );
                        assert( false && "reciever is not callable..." );
                    }

                } else if ( n->type == node_type::e_symbol ) {
                    auto&& reciever_symbol = static_cast<symbol const* const>( n );

                    // std::cout << "look_up: " << reciever_symbol->value << std::endl;
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
                return scope_->def_symbol(
                    name,
                    gc_->template make_object<native_function>( std::forward<F>( f ) ),
                    scope_->make_inner_scope()
                    );
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
                    cons* argument_head = args;

                    //bool reached_to_last_parameter = false;

                    // map argument/parameter
                    // TODO: check length of parameter/arguments & optional keyword
                    assert( is_list( argument_head ) && is_list( parameter_head ) );
                    while( !is_nil( parameter_head ) ) {
                        // parameters
                        if ( is_symbol( parameter_head->car ) ) {
                            auto&& parameter_symbol = static_cast<symbol const* const>( parameter_head->car );
                            // std::cout << "parameter : " << parameter_symbol->value << std::endl;

                            // set argument value
                            new_scope->def_symbol(
                                parameter_symbol->value,
                                argument_head->car
                                );

                        } else if ( is_keyword( parameter_head->car ) ) {
                            // keyword
                            auto&& k = static_cast<keyword const* const>( parameter_head->car );
                            std::cout << "? -> " << k->value << std::endl;

                            // update && take parameter name
                            if ( is_nil( parameter_head->cdr ) ) {
                                assert( false && "parameter name was not given" );
                            }
                            parameter_head = static_cast<cons const* const>( parameter_head->cdr );
                            if ( !is_symbol( parameter_head->car ) ) {
                                assert( false && "invalid parameter name" );
                            }
                            auto&& parameter_symbol = static_cast<symbol const* const>( parameter_head->car );

                            if ( k->value == "&rest" ) {
                                // set rest of arguments to this name
                                new_scope->def_symbol(
                                    parameter_symbol->value,
                                    argument_head
                                    );

                                // terminate
                                argument_head = static_context::nil_object;

                                // break argument matching
                                break;

                            } else {
                                assert( false && "keyword was not supported yet" );
                            }

                        } else {
                            assert( false && "" );
                        }

                        parameter_head = static_cast<cons const* const>( parameter_head->cdr );

                        // argument
                        argument_head = static_cast<cons* const>( argument_head->cdr );
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
                node* last_value = static_context::nil_object;

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
                        // auto&& typed_val = static_cast<float_value const* const>( v );
                        assert( false );

                    } else {
                        // type error
                        std::cout << "!!! type error" << std::endl;
                        print_node( v );
                        assert( false );
                    }

                    assert( is_list( t->cdr ) );
                    t = static_cast<cons const* const>( t->cdr );
                }

                return [&]() -> node* {
                    if ( nt == node_type::e_integer ) {
                         return gc_->template make_object<integer_value>( result );

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
                        // auto&& typed_val = static_cast<float_value const* const>( v );
                        assert( false );

                    } else {
                        // type error
                        std::cout << "!!! type error" << std::endl;
                        print_node( v );
                        assert( false );
                    }

                    assert( is_list( t->cdr ) );
                    t = static_cast<cons const* const>( t->cdr );
                }

                return [&]() -> node* {
                    if ( nt == node_type::e_integer ) {
                         return gc_->template make_object<integer_value>( result );

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
                
                return n->car;
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

                if ( !is_nil( condition ) ) {
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

            auto car_func( cons* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                if ( is_nil( n ) ) {
                    return static_context::nil_object;
                }

                assert( is_list( n->car ) );
                
                node const* const econs = as_node( eval( n->car, current_scope ) );
                
                assert( is_list( econs ) );

                if( is_nil( econs ) ) {
                    return static_context::nil_object;     
                } else {
                    return static_cast<cons const* const>( econs )->car;                    
                }
            }

            auto cdr_func( cons* const n, std::shared_ptr<scope> const& current_scope )
                -> node*
            {
                if ( is_nil( n ) ) {
                    return static_context::nil_object;
                }

                assert( is_list( n->car ) );

                node const* const econs = as_node( eval( n->car, current_scope ) );
                
                assert( is_list( econs ) );

                if( is_nil( econs ) ) {
                    return static_context::nil_object;     
                } else {
                    return static_cast<cons const* const>( econs )->cdr;                    
                }
            }

        private:
            std::shared_ptr<scope> scope_;
            std::shared_ptr<GC> gc_;
        };

    } // namespace interpreter
} // namespace yakkai
