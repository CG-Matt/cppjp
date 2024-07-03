#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exceptions.hpp"
#include "standalone.hpp"

json::invalid_node_type::invalid_node_type(JSONNodeType expected, JSONNodeType received, const char* source)
{
    size_t required_size = 45;
    required_size += strlen(source);
    required_size += strlen(NodeTypeAsCString(expected));
    required_size += strlen(NodeTypeAsCString(received));

    this->message = static_cast<char*>(malloc(required_size));
    sprintf(this->message, "JSON::%s: Expected node of type %s, Received: %s.", source, NodeTypeAsCString(expected), NodeTypeAsCString(received));
}