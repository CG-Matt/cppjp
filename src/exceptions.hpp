#pragma once

#include <exception>
#include <string>
#include <initializer_list>
#include "cppjp.hpp"

namespace json
{
    class invalid_node_type: public std::exception
    {
        private: std::string message;
        public:
            invalid_node_type(JSONNodeType expected, JSONNodeType received, const char* source = __builtin_FUNCTION());
            invalid_node_type(std::initializer_list<JSONNodeType> expected, JSONNodeType received, const char* source = __builtin_FUNCTION());
            const char* what() const noexcept override { return message.c_str(); }
    };

    class bad_node_access: public std::exception
    {
        private: std::string message;
        public:
            bad_node_access(const char* source = __builtin_FUNCTION());
            const char* what() const noexcept override { return message.c_str(); }
    };
};
