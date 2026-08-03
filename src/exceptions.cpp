#include <cstring>
#include "exceptions.hpp"
#include "standalone.hpp"

json::invalid_node_type::invalid_node_type(JSONNodeType expected, JSONNodeType received, const char* source)
    : invalid_node_type({ expected }, received, source)
{}

json::invalid_node_type::invalid_node_type(std::initializer_list<JSONNodeType> expected, JSONNodeType received, const char* source)
{
    message = "JSON::";
    message += source;
    message += ": Expected node of type ";

    for(const JSONNodeType* expected_type = expected.begin(); expected_type != expected.end(); expected_type++)
    {
        if(expected_type != expected.begin())
            message += " or ";

        message += NodeTypeAsCString(*expected_type);
    }

    message += ". Received: ";
    message += NodeTypeAsCString(received);
}
