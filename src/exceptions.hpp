#pragma once

#include <exception>
#include "../include/cppjp.hpp"

namespace json
{
    class invalid_node_type: public std::exception
    {
        private: char* message = nullptr;
        public:
            invalid_node_type(JSONNodeType expected, JSONNodeType received, const char* source = __builtin_FUNCTION());
            virtual const char* what() const noexcept { return message; }
    };

};