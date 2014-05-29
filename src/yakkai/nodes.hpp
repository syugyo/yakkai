#pragma once

#include <string>
#include <memory>
#include <cassert>


namespace yakkai
{
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


    inline auto debug_string( node_type const& n )
        -> std::string
    {
        switch( n ) {
        case node_type::e_none:
            return "NONE";
        case node_type::e_list:
            return "LIST";
        case node_type::e_symbol:
            return "SYMBOL";
        case node_type::e_native_function:
            return "NATIVE_FUNCTION";
        case node_type::e_keyword:
            return "KEYWORD";
        case node_type::e_string:
            return "STRING";
        case node_type::e_integer:
            return "INTEGER";
        case node_type::e_ratio:
            return "RATIO";
        case node_type::e_float:
            return "FLOAT";
        case node_type::e_complex:
            return "COMPLEX";
        default:
            return "%";
        }
    }

    //
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

        cons( node* l, node* r )
            : node( node_type::e_list )
            , car( l )
            , cdr( r )
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


    // native function node will be defined elsewhere


    //
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


    ///
    ///


    auto is_nil( node const* const n )
        -> bool;

    auto is_atom( node const* const n )
        -> bool;
    auto is_list( node const* const n )
        -> bool;

    auto is_symbol( node const* const n )
        -> bool;

    auto is_keyword( node const* const n )
        -> bool;

    auto is_native_function( node const* const n )
        -> bool;

    auto is_integer( node const* const n )
        -> bool;

    auto is_float( node const* const n )
        -> bool;

    auto is_callable( node const* const n )
        -> bool;


} // namespace yakkai
