#include <memory>
#include <map>
#include <list>
#include <cassert>

#include <iostream>

#include "nodes.hpp"


namespace yakkai
{
    namespace interpreter
    {
        template<typename F>
        auto visit( node* n, F const& marker )
            -> void
        {
            marker( n );
            std::cout << debug_string( n->type ) << std::endl;

            if ( is_list( n ) && !is_nil( n ) ) {
                std::cout << "list" << std::endl;
                auto* l = static_cast<cons*>( n );
                visit( l->car, marker );
                visit( l->cdr, marker );
            }
        }

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

            template<typename F>
            auto f( F const& marker )
                -> void
            {
                // scopes of childlen
                for( auto&& it : environment_ ) {
                    auto&& p = it.second;

                    auto&& n = std::get<0>( p );    // node
                    marker( n );

                    // recursive
                    auto&& s = std::get<1>( p );    // scope
                    if ( s != nullptr ) {
                        s->f( marker );
                    }
                }

                //
                for( auto it=inline_scopes_.begin(); it != inline_scopes_.end(); /**/ ) {
                    if ( it->expired() ) {
                        it = inline_scopes_.erase( it );

                    } else {
                        it->lock()->f( marker );
                        ++it;
                    }
                }
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
                auto const p = std::make_shared<scope>( shared_from_this() );
                inline_scopes_.push_back( p );

                return p;
            }

        private:
            std::weak_ptr<scope> parent_;

            std::map<std::string, std::pair<node*, std::shared_ptr<scope>>> environment_;
            std::list<std::weak_ptr<scope>> inline_scopes_;
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

    } // namespace interpreter
} // namespace yakkai
