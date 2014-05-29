#pragma once

#include <memory>

#include "../node.hpp"
#include "../exception.hpp"
#include "../static_context.hpp"


namespace yakkai
{
    namespace syntax
    {
        template<typename RangedIterator, typename GC>
        class s_exp_parser
        {
            using input_type = RangedIterator;

        public:
            s_exp_parser( std::shared_ptr<GC> const& gc )
                : gc_( gc )
            {}

        public:
            auto parse_s_expression( RangedIterator& rng_it )
                -> node*
            {
                return parse_s_expression( rng_it, false );
            }

        private:
            auto parse_s_expression( RangedIterator& rng_it, bool const is_enable_closer )
                -> node*
            {
                skip_space( rng_it );
                expect_not_eof( rng_it );

                if ( *rng_it == '(' ) {
                    step_iterator( rng_it );

                    // car
                    node* volatile const s = parse_s_expression_or_closer( rng_it );
                    if ( s == nullptr ) {
                        return static_context::nil_object;
                    }

                    //
                    cons* volatile const cell = gc_->template make_object<cons>( s, static_context::nil_object );
                    // print2( cell );

                    skip_space( rng_it );
                    expect_not_eof( rng_it );

                    if ( *rng_it == '.' ) {
                        // cons cell
                        step_iterator( rng_it );

                        parse_s_expression( rng_it, cell );
                        return parse_closer_s_expression( rng_it, cell );

                    } else {
                        // e_list
                        cons* volatile last_cell = cell;
                        while( cons* volatile const v = parse_s_expression_or_closer( rng_it, last_cell ) ) {
                            // print2( v );
                            // std::cout << (void*)&v << " -> " << (void*)v << std::endl;

                            if ( is_nil( v ) ) break;
                            last_cell = v;
                        }

                        return cell;
                    }

                } else if ( is_enable_closer && *rng_it == ')' ) {
                    return parse_closer_s_expression( rng_it, nullptr );

                } else {
                    // atom
                    node* volatile a = parse_atom( rng_it );
                    assert( a != nullptr );

                    if ( !parse_token_separate( rng_it ) ) {
                        throw "parse error";
                    }

                    return a;
                }
            }

            auto parse_s_expression( RangedIterator& rng_it, cons* outer_cell, bool const is_enable_closer )
                -> cons*
            {
                node* volatile const s = parse_s_expression( rng_it, is_enable_closer );

                //
                if ( s == nullptr ) {
                    outer_cell->cdr = static_context::nil_object;
                    return static_context::nil_object;

                } else {
                    cons* volatile inner_cell = gc_->template make_object<cons>( s, static_context::nil_object );

                    assert( outer_cell != nullptr );
                    outer_cell->cdr = inner_cell;

                    return inner_cell;  // NOTE:
                }
            }

            auto parse_s_expression_or_closer( RangedIterator& rng_it )
                -> node*
            {
                return parse_s_expression( rng_it, true );
            }

            auto parse_s_expression_or_closer( RangedIterator& rng_it, cons* outer_cell )
                -> cons*
            {
                return parse_s_expression( rng_it, outer_cell, true );
            }

            auto parse_closer_s_expression( RangedIterator& rng_it, node* n = nullptr )
                -> node*
            {
                if ( *rng_it != ')' ) {
                    assert( false );
                }
                step_iterator( rng_it );

                return n;
            }

            auto parse_token_separate( RangedIterator& rng_it )
                -> bool
            {
                return true;
            }

        private:
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

        public:
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

                    return gc_->template make_object<keyword>( std::string( begin.it(), rng_it.it() ) );

                } else {
                    rng_it = begin;
                    return nullptr;
                }
            }

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

                    return gc_->template make_object<symbol>( std::string( begin.it(), rng_it.it() ) );

                } else {
                    rng_it = begin;
                    return nullptr;
                }
            }

        private:
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

                //
                if ( !is_eof( rng_it ) && ( *rng_it == '.' || *rng_it == 'e' || *rng_it == 'E' ) ) {
                    rng_it = begin;
                    return nullptr;
                }

                skip_space( rng_it );

                return gc_->template make_object<integer_value>( number );
            }


            // 0.0
            // .0
            // .0e10
            // (+|-) (digit*)? (.)? digit+ ( (e|E) (+|-) digit+ )?
            auto parse_float( RangedIterator& rng_it )
                -> node*
            {
                RangedIterator begin = rng_it;
                std::string number, exp;

                // sign is optional
                bool const has_sign = parse_sign( rng_it );
                if ( has_sign ) {
                    number += std::string( begin.it(), rng_it.it() );

                    skip_space( rng_it );
                }

                //
                bool const has_digit = [&]() mutable {
                    RangedIterator base_it = rng_it;
                    if ( parse_digit( rng_it ) ) {
                        number += std::string( base_it.it(), rng_it.it() );
                        return true;
                    }

                    return false;
                }();

                //
                bool const has_float_point = [&]() mutable {
                    if ( *rng_it == '.' ) {
                        step_iterator( rng_it );

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

                    return gc_->template make_object<float_value>( number, exp );

                } else {
                    rng_it = begin;
                    return nullptr;
                }
            }

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

        private:
            auto skip_space( RangedIterator& rng_it )
                -> void
            {
                if ( is_eof( rng_it ) ) return;

                //expect_not_eof( it, end );
                while( !is_eof( rng_it ) && std::isspace( *rng_it ) ) {
                    step_iterator( rng_it );
                }
            }

        private:
            auto is_eof( RangedIterator const& rng_it ) const
                -> bool
            {
                return rng_it.it_ == rng_it.end_;
            }

            auto expect_not_eof( RangedIterator const& rng_it ) const
                -> void
            {
                if ( is_eof( rng_it ) ) {
                    throw reached_to_eof();
                }
            }

            auto step_iterator( RangedIterator& rng_it, std::size_t step = 1 ) const
                -> void
            {
                expect_not_eof( rng_it );

                for( std::size_t i=0; i<step && !is_eof( rng_it ); ++i ) {
                    ++rng_it;
                }
            }

        private:
            std::shared_ptr<GC> gc_;
        };

    } // namespace syntax
} // namespace yakkai
