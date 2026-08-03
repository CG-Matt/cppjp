#pragma once

#include <cstdint>
#include <string>
#include <functional>

enum class JSONNodeType
{
    STRING,
    NUMBER,
    OBJECT,
    ARRAY,
    TRUE,
    FALSE,
    JNULL
};

struct JSONNode
{
    std::string name;
    JSONNodeType type;
    JSONNode* parent;
    JSONNode* next = nullptr;
    JSONNode* previous = nullptr;
    JSONNode* child = nullptr;
    std::string string_data;
};

class JSON
{
    public:
    
    /**
     * Creates an owning JSON object by parsing a JSON string.
     * @param str The JSON string to parse.
     * @return The parsed JSON object.
     */
    static JSON FromJSONString(const char* str);

    /**
     * Creates a non-owning JSON object that wraps a JSON node.
     * @param node The node to wrap.
     * @return A non-owning JSON object.
    */
    static JSON Wrap(JSONNode* node);

    /**
     * Creates an owning JSON object by adopting a JSON node.
     * @param node The node whose ownership to adopt.
     * @return An owning JSON object.
     */
    static JSON Adopt(JSONNode* node);

    /**
     * Creates an owning deep copy of this JSON node.
     * @return An owning clone of this JSON node.
     */
    JSON clone() const;

    /**
     * Detaches this JSON node from its parent.
     * @return The detached JSON node.
     */
    JSON detach();

    /**
     * Releases the wrapped JSON node without deleting it.
     * If this JSON object owns the node, ownership transfers to the caller.
     * @return A pointer to the released node.
     */
    JSONNode* release();

    /**
     * Erases this JSON node and all of its descendants.
     * The node is detached from its parent before being deleted.
     */
    void erase();

    // Automatic conversion
    operator JSONNode*() { return this->node; }

    bool isValid() const;
    bool isOwning() const;
    JSONNodeType getType() const;
    const char* getTypeCString() const;

    const std::string& getName() const;
    const char* getNameCString() const;

    std::string asString() const;
    const char* asCString() const;
    std::uintmax_t asNumber() const;
    std::intmax_t asSignedNumber() const;
    double asFloat() const;
    bool asBool() const;
    JSONNode* asRaw();

    std::string asNodeData();

    bool isNull() const;

    bool hasEntry(const char* key) const;
    size_t arraySize() const;

    JSON getEntry(const char* key);
    JSON getElement(size_t index);
    JSONNode* getRawEntry(const char* key);
    JSONNode* getRawElement(size_t index);

    /**
     * Iterates over this JSON array.
     *
     * The callback may erase the current node. Modifying or erasing any
     * other node in the iterated tree invalidates the iteration.
     */
    void iterate(std::function<void(JSON node)> callback);

    /**
     * Iterates over this JSON object.
     *
     * The callback may erase the current node. Modifying or erasing any
     * other node in the iterated tree invalidates the iteration.
     */
    void iterateObject(std::function<void(JSON node)> callback);

    std::string asPrintable() const;

    void writeOut(std::string& output_buffer) const;

    JSON(const JSON& src);
    JSON(JSON&& src) noexcept;
    ~JSON();

    JSON& operator=(const JSON& src);
    JSON& operator=(JSON&& src) noexcept;

    private:
        JSONNode* node;     // Stores the JSON data
        bool is_owning;     // Does this class own the JSONNode data?
        bool is_valid;      // Is the JSONNode data valid?

    JSON() noexcept;
};

namespace CPPJP
{
    /**
     * Parses a string of JSON data into a JSON Node object.
     * @param json_str The JSON string to parse
     * @param dest The destination for the resulting JSON structure
     * @return ```true``` if successful, ```false``` otherwise.
     */
    bool ParseJSON(const char* json_str, JSONNode* dest);

    /**
     * Clones (deep copies) a JSON node.
     * @param node The node to clone.
     * @return A pointer to the cloned node.
     */
    JSONNode* CloneNode(JSONNode* node);

    /**
     * Detaches a JSON node from its parent.
     * @param node The node to detach.
     * @return A pointer to the detached node.
     */
    JSONNode* DetachNode(JSONNode* node);

    /**
     * Frees the memory of a node and all of its sub nodes.
     * @param node The node to be deleted.
     */
    void FreeNode(JSONNode* node);
}
