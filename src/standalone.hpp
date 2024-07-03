#include "../include/cppjp.hpp"

static const char* node_type_names[] = { "String", "Number", "Object", "Array", "True", "False", "Null" };

inline const char* getNodeTypeCString(JSONNode* node)
{
    return node_type_names[static_cast<uint8_t>(node->type)];
}

inline const char* NodeTypeAsCString(JSONNodeType type)
{
    return node_type_names[static_cast<uint8_t>(type)];
}

void destroyNode(JSONNode* node);