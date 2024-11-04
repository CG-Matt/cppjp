#include "../include/cppjp.hpp"
#include "parser.hpp"
#include "standalone.hpp"
#include "exceptions.hpp"
#include <string>
#include <exception>

// JSON class
JSON::JSON(JSONNode* src)
{
    this->node = src;
    this->is_owner = false;
}

JSON::JSON(const char* ch)
{
    this->is_owner = true;
    this->node = new JSONNode;
    if(!CPPJP::ParseJSON(ch, this->node)){ exit(1); }
}

JSON::~JSON()
{
    if(this->is_owner)
        this->destroy();
}

JSONNodeType JSON::getType() const
{
    return this->node->type;
}

const char* JSON::getTypeCString() const
{
    return getNodeTypeCString(this->node);
}

const std::string& JSON::getName() const
{
    return this->node->name;
}

const char* JSON::getNameCString() const
{
    return this->node->name.data();
}

std::string JSON::asString() const
{
    if(this->node->type != JSONNodeType::STRING)
        throw json::invalid_node_type(JSONNodeType::STRING, this->getType());

    return this->node->string_data;
}

const char* JSON::asCString() const
{
    if(this->node->type != JSONNodeType::STRING)
        throw json::invalid_node_type(JSONNodeType::STRING, this->getType());

    return this->node->string_data.data();
}

size_t JSON::asNumber() const
{
    if(this->node->type != JSONNodeType::NUMBER)
        throw json::invalid_node_type(JSONNodeType::NUMBER, this->getType());

    return std::stoull(this->node->string_data);
}

ssize_t JSON::asSignedNumber() const
{
    if(this->node->type != JSONNodeType::NUMBER)
        throw json::invalid_node_type(JSONNodeType::NUMBER, this->getType());

    return std::stoll(this->node->string_data);
}

double JSON::asFloat() const
{
    if(this->node->type != JSONNodeType::NUMBER)
        throw json::invalid_node_type(JSONNodeType::NUMBER, this->getType());

    return std::stod(this->node->string_data);
}

bool JSON::asBool() const
{
    if(this->node->type == JSONNodeType::TRUE){ return true; }
    if(this->node->type == JSONNodeType::FALSE){ return false; }

    printf("JSON::asBool: Expected node of type True or False. Received: %s.\n", this->getTypeCString());
    exit(1);
}

JSONNode* JSON::asRaw(){ return this->node; }

std::string JSON::asNodeData()
{
    size_t buffer_size = 256 + this->node->name.size() + this->node->string_data.size();
    char* printable = static_cast<char*>(malloc(buffer_size));

    sprintf(printable, "{\n\tname = %s\n\ttype = %s\n\tparent = %p\n\tnext = %p\n\tprevious = %p\n\tchild = %p\n\tstring_data = %s\n}",
        this->node->name.data(),
        this->getTypeCString(),
        this->node->parent,
        this->node->next,
        this->node->previous,
        this->node->child,
        this->node->string_data.data()
    );

    std::string ret(printable);
    free(printable);
    return ret;
}


bool JSON::isNull() const { return this->node->type == JSONNodeType::JNULL; }

bool JSON::hasEntry(const char* key) const
{
    if(this->node->type != JSONNodeType::OBJECT)
        throw json::invalid_node_type(JSONNodeType::OBJECT, this->getType());

    JSONNode* current_node = this->node->child;
    while(current_node)
    {
        if(current_node->name == std::string(key)) return true;
        current_node = current_node->next;
    }

    return false;
}

size_t JSON::arraySize() const
{
    if(this->node->type != JSONNodeType::ARRAY)
        throw json::invalid_node_type(JSONNodeType::ARRAY, this->getType());

    if(!this->node->child) return 0;

    JSONNode* cur_node = this->node->child;
    size_t array_size = 1;

    while(cur_node)
    {
        if(cur_node->next) array_size++;
        cur_node = cur_node->next;
    }

    return array_size;
}

JSON JSON::getEntry(const char* key){ return JSON(this->getRawEntry(key)); }
JSON JSON::getElement(size_t index){ return JSON(this->getRawElement(index)); }

JSONNode* JSON::getRawEntry(const char* key)
{
    if(this->node->type != JSONNodeType::OBJECT)
        throw json::invalid_node_type(JSONNodeType::OBJECT, this->getType());

    JSONNode* current_node = this->node->child;
    while(current_node)
    {
        if(current_node->name == std::string(key)) return current_node;
        current_node = current_node->next;
    }

    printf("JSON::%s: Node has no entry with key \"%s\"\n", __func__, key);
    exit(1);
}

JSONNode* JSON::getRawElement(size_t index)
{
    if(this->node->type != JSONNodeType::ARRAY)
        throw json::invalid_node_type(JSONNodeType::ARRAY, this->getType());

    JSONNode* current_node = this->node->child;

    size_t i = 0;

    for(i = 0; i < index; i++)
    {
        current_node = current_node->next;
    }

    if(i != index)
    {
        printf("JSON::%s: The given index (%lu) is out of range of the array (%lu)\n", __func__, index, i);
        exit(1);
    }

    return current_node;
}

void JSON::iterate(std::function<void(JSON node)> callback)
{
    if(this->node->type != JSONNodeType::ARRAY)
        throw json::invalid_node_type(JSONNodeType::ARRAY, this->getType());

    JSONNode* current_node = this->node->child;
    while(current_node)
    {
        callback(JSON(current_node));
        current_node = current_node->next;
    }
}

void JSON::iterateObject(std::function<void(JSON node)> callback)
{
    if(this->node->type != JSONNodeType::OBJECT)
        throw json::invalid_node_type(JSONNodeType::OBJECT, this->getType());

    JSONNode* current_node = this->node->child;
    while(current_node)
    {
        callback(JSON(current_node));
        current_node = current_node->next;
    }
}

std::string JSON::asPrintable() const
{
    std::string out;

    switch(this->node->type)
    {
        case JSONNodeType::ARRAY:
        {
            out += "[\n";

            JSONNode* current_node = node->child;
            while(current_node)
            {
                out += "\t";
                out += getNodeTypeCString(current_node);
                out += "\n";
                current_node = current_node->next;
            }

            out += "]";
        } break;

        case JSONNodeType::OBJECT:
        {
            out += "{\n";

            JSONNode* current_node = node->child;
            while(current_node)
            {
                out += "\t" + current_node->name + ": ";
                // If the node is of a simple type (string, number, true, false, null), show the value
                switch(current_node->type)
                {
                    case JSONNodeType::STRING:
                        out += "\"" + current_node->string_data + "\""; // name: "str_data"
                        break;

                    case JSONNodeType::NUMBER:
                        out += current_node->string_data; // name: str_data
                        break;

                    case JSONNodeType::ARRAY:
                        if(!current_node->child)
                            out += "[]"; // name: []
                        else
                            out += getNodeTypeCString(current_node); // name: Type
                        break;

                    case JSONNodeType::OBJECT:
                        if(!current_node->child)
                            out += "{}"; // name: {}
                        else
                            out += getNodeTypeCString(current_node); // name: Type
                        break;

                    case JSONNodeType::TRUE:
                    case JSONNodeType::FALSE:
                    case JSONNodeType::JNULL:
                        out += getNodeTypeCString(current_node); // name: Type
                        break;
                }
                out += "\n";
                current_node = current_node->next;
            }

            out += "}";
        } break;

        case JSONNodeType::STRING:
        case JSONNodeType::NUMBER:
            out += this->node->string_data;
            break;

        case JSONNodeType::TRUE:
        case JSONNodeType::FALSE:
        case JSONNodeType::JNULL:
            out += this->getTypeCString();
            break;
    }

    return out;
}

void JSON::writeOut(std::string& out_buf) const { CPPJP::WriteJson(this->node, out_buf); }

void JSON::destroy(){ destroyNode(this->node); if(this->is_owner) this->is_owner = false; }