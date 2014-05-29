#include <functional>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <cstdlib>

#include <iostream>

#include "nodes.hpp"


namespace yakkai
{
    namespace memory
    {
        class page
        {
            static constexpr std::size_t block_max = 128;

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
                            // std::cout << "DESTRUCT: " << std::endl;
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
                // std::cout << (void*)data_ << " <= " << (void*)p << " < " << (void*)( data_ + total_size_ ) << std::endl;
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
                // std::cout << "memory: " << (void*)rhs.data_;
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
