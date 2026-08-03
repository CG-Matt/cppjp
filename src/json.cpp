#include "cppjp.hpp"
#include "parser.hpp"
#include "standalone.hpp"
#include "exceptions.hpp"
#include <string>
#include <exception>

//
//  JSON Class
//

JSON JSON::FromJSONString(const char* str)
{
    JSON json;
    json.node = new JSONNode;
    json.is_owning = true;
    json.is_valid = CPPJP::ParseJSON(str, json.node);
    return json;
}

JSON JSON::Wrap(JSONNode* node)
{
    JSON json;
    json.node = node;
    json.is_owning = false;
    json.is_valid = node != nullptr;
    return json;
}

JSON JSON::Adopt(JSONNode* node)
{
    JSON json;
    json.node = node;
    json.is_owning = node != nullptr;
    json.is_valid = node != nullptr;
    return json;
}

JSON::JSON() noexcept
    : node(nullptr), is_owning(false), is_valid(false)
{}

JSON::JSON(const JSON& src)
{
    if(src.is_owning)
        this->node = CPPJP::CloneNode(src.node);
    else
        this->node = src.node;

    this->is_owning = src.is_owning;
    this->is_valid = src.is_valid;
}

JSON::JSON(JSON&& src) noexcept
{
    this->node = src.node;
    src.node = nullptr;

    this->is_owning = src.is_owning;
    src.is_owning = false;

    this->is_valid = src.is_valid;
    src.is_valid = false;
}

JSON::~JSON()
{
    if(this->is_owning) this->erase();
}

JSON& JSON::operator=(const JSON& src)
{
    if(this == &src) return *this;

    JSONNode* copy_node = src.is_owning ? CPPJP::CloneNode(src.node) : src.node;

    if(this->is_owning) this->erase();

    this->node = copy_node;
    this->is_owning = src.is_owning;
    this->is_valid = src.is_valid;

    return *this;
}

JSON& JSON::operator=(JSON&& src) noexcept
{
    if(this == &src) return *this;

    if(this->is_owning) this->erase();

    this->node = src.node;
    src.node = nullptr;

    this->is_owning = src.is_owning;
    src.is_owning = false;

    this->is_valid = src.is_valid;
    src.is_valid = false;

    return *this;
}

JSON JSON::clone() const
{
    JSON json;
    json.node = CPPJP::CloneNode(this->node);
    json.is_owning = json.node != nullptr;
    json.is_valid = this->is_valid;
    return json;
}

JSON JSON::detach()
{
    if(!this->node) return JSON{};

    // Return self if already top level node
    if(this->node->parent == nullptr) return std::move(*this);

    JSON json;

    json.node = CPPJP::DetachNode(this->node);
    json.is_owning = json.node != nullptr;
    json.is_valid = this->is_valid;

    this->node = nullptr;
    this->is_owning = false;
    this->is_valid = false;

    return json;
}

JSONNode* JSON::release()
{
    JSONNode* node = this->node;
    this->node = nullptr;
    this->is_owning = false;
    this->is_valid = false;
    return node;
}

void JSON::erase()
{
    if(!this->node) return;

    if(this->node->parent) CPPJP::DetachNode(this->node);
    CPPJP::FreeNode(this->node);

    this->node = nullptr;
    this->is_owning = false;
    this->is_valid = false;
}

bool JSON::isValid() const { return this->is_valid; }

bool JSON::isOwning() const { return this->is_owning; }

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

    throw json::invalid_node_type({ JSONNodeType::TRUE, JSONNodeType::FALSE }, this->getType());
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

bool JSON::isEmpty() const
{
    if(this->node->type != JSONNodeType::ARRAY && this->node->type != JSONNodeType::OBJECT)
        throw json::invalid_node_type({ JSONNodeType::ARRAY, JSONNodeType::OBJECT }, this->getType());

    return this->node->child == nullptr;
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

JSON JSON::getEntry(const char* key){ return JSON::Wrap(this->getRawEntry(key)); }
JSON JSON::getElement(size_t index){ return JSON::Wrap(this->getRawElement(index)); }

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
        JSONNode* next_node = current_node->next;
        callback(JSON::Wrap(current_node));               // Node could be deleted here
        current_node = next_node;
    }
}

void JSON::iterateObject(std::function<void(JSON node)> callback)
{
    if(this->node->type != JSONNodeType::OBJECT)
        throw json::invalid_node_type(JSONNodeType::OBJECT, this->getType());

    JSONNode* current_node = this->node->child;
    while(current_node)
    {
        JSONNode* next_node = current_node->next;
        callback(JSON::Wrap(current_node));               // Node could be deleted here
        current_node = next_node;
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

namespace
{
    void _CopyNodeData(JSONNode* dest, JSONNode* src)
    {
        dest->name = src->name;
        dest->type = src->type;
        dest->string_data = src->string_data;

        dest->parent = nullptr;
        dest->next = nullptr;
        dest->previous = nullptr;
        dest->child = nullptr;
    }
}

namespace CPPJP
{
    JSONNode* CloneNode(JSONNode* node)
    {
        if(!node) return nullptr;
        JSONNode* copy = new JSONNode{};
        _CopyNodeData(copy, node);

        JSONNode* source_current = node;
        JSONNode* copy_current = copy;

        while(true)
        {
            if(source_current->child)
            {
                source_current = source_current->child;

                JSONNode* child_copy = new JSONNode{};
                _CopyNodeData(child_copy, source_current);

                child_copy->parent = copy_current;
                copy_current->child = child_copy;

                copy_current = child_copy;
                continue;
            }

            // Walk back up until a sibling is available.
            while(source_current != node && !source_current->next)
            {
                source_current = source_current->parent;
                copy_current = copy_current->parent;
            }

            // We returned to the cloned root: all descendants are done.
            if(source_current == node)
                break;

            // Clone the next sibling.
            source_current = source_current->next;

            JSONNode* sibling_copy = new JSONNode{};
            _CopyNodeData(sibling_copy, source_current);

            sibling_copy->parent = copy_current->parent;
            sibling_copy->previous = copy_current;
            copy_current->next = sibling_copy;

            copy_current = sibling_copy;
        }

        return copy;
    }

    JSONNode* DetachNode(JSONNode* node)
    {
        // Detach the node
        if(node->previous)
            node->previous->next = node->next;
        if(node->next)
            node->next->previous = node->previous;
        if(node->parent && node->parent->child == node)
            node->parent->child = node->next;

        node->parent    = nullptr;
        node->previous  = nullptr;
        node->next      = nullptr;

        return node;
    }

    void FreeNode(JSONNode* node)
    {
        // Check for child first then for next node

        JSONNode* current_node = node;
        JSONNode* next_node; // Initialise a pointer variable for the next node to move onto

        // First check if the current node has a next node
        // And if so then make it the next node of the previous node
        if(current_node->next)
        {
            if(current_node->previous)
            {
                current_node->previous->next = current_node->next;
                current_node->next = nullptr;
                current_node->previous = nullptr;
            }
            else
            {
                // No need to check if there current node has a next node as root node will never have a next node
                current_node->parent->child = current_node->next;
                current_node->next = nullptr;
                current_node->parent = nullptr;
            }
        }

        while(true)
        {
            if(current_node->child)
            {
                current_node = current_node->child;
                continue;
            }
            if(current_node->next)
            {
                current_node = current_node->next;
                continue;
            }

            // The current node now has no children and no next nodes
            if(current_node == node)
            {
                // We have arrived back at the start point. Delete it and remove any references to it
                // Not sure if this is actually nevessary as the code above us should take care of it
                if(current_node->previous)
                {
                    if(current_node->next)
                        current_node->previous->next = current_node->next;
                    else
                        current_node->previous->next = nullptr;
                }

                if(node->parent && node->parent->child == node)
                    node->parent->child = nullptr;

                delete current_node;
                node = nullptr;
                return;
            }
            else if(current_node->previous)  // If we have a previous node move onto that
            {
                next_node = current_node->previous;
                next_node->next = nullptr; // Remove pointer to next node from previous node as it will be deleted
            }
            else if(current_node->parent) // If the current node has a parent switch to the parent (remember: the root node does not have a parent)
            {
                next_node = current_node->parent;
                next_node->child = nullptr; // Remove pointer to child node from parent node as it will be deleted
            }
            else
            {
                // If we have no previous and no parent nodes we are at the root node which can now be safely deleted
                delete current_node;
                node = nullptr;
            }

            delete current_node;
            current_node = next_node;
        }
    }
}
