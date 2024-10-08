#pragma once

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
    private:
        JSONNode* node;
        bool is_owner;

    public:
    
    /*
        Create a JSON class from a JSONNode
    */
    JSON(JSONNode* node);

    /*
        Parse and create a JSON class from a string containing JSON formatted data
    */
    JSON(const char* json_str);

    ~JSON();

    JSONNodeType getType() const;
    // std::string GetTypeString();
    const char* getTypeCString() const;

    const std::string& getName() const;
    const char* getNameCString() const;

    std::string asString() const;
    const char* asCString() const;
    size_t asNumber() const;
    ssize_t asSignedNumber() const;
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

    // void iterate(void (*callback)(JSONNode* node));
    // void iterate(void (*callback)(JSON node));

    // void iterateObject(void (*callback)(JSONNode* node));
    // void iterateObject(void (*callback)(JSON node));

    void iterate(std::function<void(JSON node)> callback);
    void iterateObject(std::function<void(JSON node)> callback);

    std::string asPrintable() const;

    void writeOut(std::string& output_buffer) const;

    void destroy();
};