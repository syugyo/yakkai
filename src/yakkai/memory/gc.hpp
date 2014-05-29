#pragma once

#include <functional>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <cstdlib>

#include <iostream>

#include "page.hpp"
#include "../node.hpp"
#include "../util/math.hpp"

namespace yakkai
{
    namespace memory
    {
        //
        class gc
        {
        private:
            constexpr static std::size_t const MinAlign = 4;

        public:
            gc( void volatile const* const p )
                : stack_begin_( reinterpret_cast<std::uintptr_t>( p ) )
            {}

            gc( void volatile const* const p, std::function<void (std::function<void (node*)> const&)> const& cm )
                : stack_begin_( reinterpret_cast<std::uintptr_t>( p ) )
                , custom_marker_( cm )
            {}

        public:
            template<typename F>
            auto cha( F&& f )
                -> void
            {
                custom_marker_ = std::forward<F>( f );
            }

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
                        auto obj = static_cast<T*>( p );
                        //print2( obj );

                        obj->~T();
                    } );

                pages_.emplace( typeid( T ), p );
                sorted_pages_.emplace_back( p );

                std::sort( sorted_pages_.begin(), sorted_pages_.end(), []( std::shared_ptr<page> const& r, std::shared_ptr<page> const& l ) {
                        return *r < *l;
                    } );
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

        private:
            auto full_collect()
                -> std::size_t
            {
                std::cout << "gc: full collect" << std::endl;
                //
                mark_stack();

                // TODO: mark_registers();
                //

                //
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
                //__attribute__( (aligned(sizeof( void* ))) ) int volatile _dummy_stack_end_object;
                alignas( void* ) int volatile _dummy_stack_end_object;

                auto high = stack_begin_;
                auto low = reinterpret_cast<std::uintptr_t>( &_dummy_stack_end_object );
                if ( high < low ) std::swap( high, low );

                // std::cout << std::hex << low << "/" << high << std::dec << std::endl;

                // ub...?
                for( auto p=reinterpret_cast<node* const* const>( low ); p<reinterpret_cast<node* const* const>( high ); ++p ) {
                    mark_object( *p );
                }
            }

            auto mark_object( node* n )
                -> void
            {
                // std::cout << "??? -> " << n << std::endl;

                // TODO: fix to binary searching to find pages
                for( auto&& target_page : sorted_pages_ ) {
                    if ( target_page->is_included( n ) ) {
                        target_page->mark( n );
                        // std::cout << "!!!!!!!! MARKED: " << (void*)n << "  ";
                        // print2( n );

                        if ( is_list( n ) && !is_nil( n ) ) {
                            auto* l = static_cast<cons*>( n );
                            mark_object( l->car );
                            mark_object( l->cdr );
                        }
                    }
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

    } // namespace memory
} // namespace yakkai
