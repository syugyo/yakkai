#include <functional>

#include "nodes.hpp"


namespace yakkai
{
    //
    namespace interpreter
    {
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

    } // namespace interpreter
} // namespace yakkai
