#pragma once

#include <string>
#include <iostream>

#include "../node.hpp"


namespace yakkai
{
    namespace detail
    {
        auto print_node_to_stream_with_type( std::ostream& os, node const* const n )
            -> std::ostream&;
    }


    auto print_node( node const* const n )
        -> void;

} // namespace yakkai
