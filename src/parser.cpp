#include <string.h>
#include "parser.hpp"
#include "standalone.hpp"
#include "cppjp.hpp"

enum class LEXSTATE
{
    SEARCH_VALUE = 0,
    SEARCH_NAME,
    SEARCH_OBJECT_CHILD,
    SEARCH_COLON,
    AWAIT_NEXT
};

/*
    Checks if the supplied character is a valid escaped character.
    Does not check for the 4 hex digits after u as per the json spec.
    @param ch The character to check
    @return ```true``` if ch is a valid escaped character ```false``` otehrwise
*/
static bool IsEscaped(char ch)
{
    static const char escape_characters[] = {'"', '\\', '/', 'b', 'f', 'n', 'r', 't', 'u', '\0'};
    const char* chars = escape_characters;

    while(*chars)
    {
        if(ch == *chars) return true;
        chars++;
    }

    return false;
}

/*
    Stores string defined between two " marks in the output_buffer.
    current_char should point to the opening ".
    The returned pointer will point to the closing ".
    @param ch The opening ```"``` from which to start the string.
    @param out_buf The output buffer to which the string will be saved to.
    @return A pointer to the closing ```"```.
*/
static const char* ParseString(const char* ch, std::string& out_buf)
{
    // This function still needs fixing to correctly parse escaped characters and error with illegal characters (eg. linfeed or carrage return)
    out_buf.clear();
    ch++;
    while(*ch != '"')
    {
        switch(*ch)
        {
            case '\n':
                puts("Illegal newline character encountered while parsing a string");
                return nullptr;

            case '\r':
                puts("Illegal carridge return character encountered while parsing a string");
                return nullptr;

            case '\\':
                out_buf.push_back(*ch);
                ch++;
                if(!IsEscaped(*ch))
                {
                    printf("The character '%c' is not a valid escaped character.\n", *ch);
                    return nullptr;
                }
                out_buf.push_back(*ch);
                ch++;
                break;

            default:
                out_buf.push_back(*ch);
                ch++;
                break;
        }
    }
    return ch;
}

/*
    Function to check if a string of characters starting at ```current_char_ptr```
    matches the characters in ```match_string```
    @param cur_ch A pointer to the current character to start the matching from
    @param match_str The string to match
    @return The number of characters matched if successful, 0 otherwise.
*/
static size_t MatchString(const char* cur_ch, const char* match_str)
{
    size_t string_length = strlen(match_str);
    for(size_t i = 0; i < string_length; i++)
    {
        if(!*cur_ch) return 0; // This check is technically redundant as if a null terminator is encountered it will not match and the function wil exit
        if(cur_ch[i] != match_str[i]) return 0;
    }
    return string_length;
}

/*
    Returns pointer pointing to next non whitespace character
*/
static const char* JumpSpace(const char* current_char)
{
    current_char++; // Should always move at least 1 character
    while(isspace(*current_char)) current_char++; // Maybe implement a custom is space function to comply with JSON standard
    return current_char;
}

/**
 * Checks if the sequence of characters starting at `s` forms a number.
 * @param s Character to start check from
 * @return Number of characters representing the number if successful.
 *         0 if NaN.
 *         -1 on error.
 */
static int ScanNumber(const char* s)
{
    const char* start = s;

    // If the current character is a minus, add it to the buffer and move to next character
    if(*s == '-'){ s++; }
    
    // Check if the current character is is 0-9
    if(!isdigit(*s)) return 0;

    s++;

    // Check last character
    if(*(s - 1) != '0')         // Checks if the previous character was a zero
        while(isdigit(*s))
            s++;

    // Next search for fraction
    if(*s == '.')
    {
        s++;

        // There needs to be at least one digit after the '.'
        if(!isdigit(*s))
        {
            puts("Number parsing error, no digits after decimal point");
            return -1;
        }

        while(isdigit(*s))
            s++;
    }

    // Then exponent
    if(*s == 'e' || *s == 'E')
    {
        s++;

        if(*s == '+' || *s == '-')
            s++;

        // There needs to be at least one digit
        if(!isdigit(*s))
        {
            puts("Number parsing error, no digits after exponent");
            return -1;
        }

        while(isdigit(*s))
            s++;
    }

    return s - start;
}

bool CPPJP::ParseJSON(const char* ch, JSONNode* dest)
{
    // Return early if the passed in pointer is null
    if(dest == nullptr) return false;

    dest->parent = nullptr;
    JSONNode* current_node = dest;
    LEXSTATE state = LEXSTATE::SEARCH_VALUE;
    std::string string_buffer;
    bool child_is_first = false;

    while(*ch)
    {
        while(isspace(*ch)) { ch++; } // Maybe implement a custom is space function to comply with JSON standard

        if(*ch == '"') // Encountered string
        {
            // Check if we are looking for a value or name, if neither then error
            switch(state)
            {
                case LEXSTATE::SEARCH_VALUE:
                    ch = ParseString(ch, string_buffer);           // Update current character position
                    current_node->type = JSONNodeType::STRING;      // Set the correct node type
                    current_node->string_data = string_buffer;      // Set current nodes string data to the extracted string
                    state = LEXSTATE::AWAIT_NEXT;
                    break;
                case LEXSTATE::SEARCH_OBJECT_CHILD:
                    ch = ParseString(ch, string_buffer);           // Update current character position
                    state = LEXSTATE::SEARCH_COLON;                 // Update state to search for a colon
                    break;
                default:
                    puts("Unexpected string token");
                    return false;
                    break;
            }
            // ch can be made null if ParseString throws an error
            if(!ch)
                return false;
        }

        int size = ScanNumber(ch);

        if(size) // Encountered number
        {
            if(size == -1)
                return false;
            
            current_node->type = JSONNodeType::NUMBER;
            current_node->string_data = std::string(ch, size);
            ch += size; // Advance the current character by the number of items traversed
            state = LEXSTATE::AWAIT_NEXT;
        }

        if(*ch == '{') // Encountered object
        {
            current_node->type = JSONNodeType::OBJECT;
            state = LEXSTATE::SEARCH_OBJECT_CHILD;
            child_is_first = true;
        }

        if(*ch == '[') // Encountered Array
        {
            if(state != LEXSTATE::SEARCH_VALUE)
            {
                puts("Invalid state @ array start");
                return false;
            }

            // Mark the current node as an array type
            current_node->type = JSONNodeType::ARRAY;
            
            // If the nextd character closes the array dont allocate memory and just continue
            if(*JumpSpace(ch) == ']')
            {
                // printf(" with no children\n");
                ch = JumpSpace(ch);
                ch++; // Needs to be incremented here
                state = LEXSTATE::AWAIT_NEXT;
                continue;
            }

            // Allocate memory for its child
            // List of next node properties that need initialisation: [parent]
            current_node->child = new JSONNode;             // Allocate memory for new child node
            current_node->child->parent = current_node;     // Set the childs parent
            current_node = current_node->child;             // Set the current node to the child
            state = LEXSTATE::SEARCH_VALUE;
        }

        if(*ch == ']')
        {
            if(state == LEXSTATE::AWAIT_NEXT)
                current_node = current_node->parent;
            state = LEXSTATE::AWAIT_NEXT;
        }

        if(*ch == 't') // Check if the word is true
        {
            if(MatchString(ch, "true") != 4)
            {
                puts("Unexpected token encountered when searching for true");
                return false;
            }

            current_node->type = JSONNodeType::TRUE;
            state = LEXSTATE::AWAIT_NEXT;
        }

        if(*ch == 'f') // Check if the word is false
        {
            if(MatchString(ch, "false") != 5)
            {
                puts("Unexpected token encountered when searching for false");
                return false;
            }

            current_node->type = JSONNodeType::FALSE;
            state = LEXSTATE::AWAIT_NEXT;
        }

        if(*ch == 'n') // Check if the word is null
        {
            if(MatchString(ch, "null") != 4)
            {
                puts("Unexpected token encountered when searching for null");
                return false;
            }

            current_node->type = JSONNodeType::JNULL;
            state = LEXSTATE::AWAIT_NEXT;
        }

        if(*ch == '}') // Encountered object end
        {
            // The two valid states that this case should be entered under are AWAIT_NEXT or SEACH_OBJECT_CHILD
            if(state == LEXSTATE::AWAIT_NEXT)
            {
                // We are at the end of the current object and should return to parent
                current_node = current_node->parent;
            }
            else if(state != LEXSTATE::SEARCH_OBJECT_CHILD)
            {
                puts("Invalid state @ object end");
                return false;
            }
            state = LEXSTATE::AWAIT_NEXT;
        }

        if(*ch == ',')
        {
            if(state != LEXSTATE::AWAIT_NEXT)
            {
                printf("Invalid state @ comma [State: %u]\n", static_cast<unsigned int>(state));
                return false;
            }

            if(current_node->parent->type == JSONNodeType::OBJECT)
            {
                child_is_first = false;
                state = LEXSTATE::SEARCH_OBJECT_CHILD;
            }
            if(current_node->parent->type == JSONNodeType::ARRAY)
            {
                // Allocate memory for the next node
                // List of child properties that need initialisation: [parent, previous_node]
                current_node->next = new JSONNode;                  // Allocate memory for new child node
                current_node->next->previous = current_node;        // Set the next nodes previous node
                current_node->next->parent = current_node->parent;  // Set the next nodes parent
                current_node = current_node->next;                  // Set the current node to the next node
                state = LEXSTATE::SEARCH_VALUE;
            }
        }

        if(*ch == ':')
        {
            if(state != LEXSTATE::SEARCH_COLON)
            {
                printf("Invalid state @ colon [State: %u]\n", static_cast<unsigned int>(state));
                return false;
            }

            // We know that we are in an object because a colon is not used elsewhere
            // We should now create a new child node or a next node for the current node based on child_is_first

            // List of child properties that need initialisation: [parent, name, previous_node]
            if(child_is_first)
            {
                current_node->child = new JSONNode;                 // Allocate memory for new child node
                current_node->child->parent = current_node;         // Set the childs parent
                current_node = current_node->child;                 // Set the current node to the child
            }
            else
            {
                current_node->next = new JSONNode;                  // Allocate memory for new next node
                current_node->next->parent = current_node->parent;  // Set the next nodes parent
                current_node->next->previous = current_node;        // Set the next nodes previous node
                current_node = current_node->next;                  // Set the current node to the child
            }

            current_node->name = string_buffer;                     // Set current nodes name to the extracted string
            state = LEXSTATE::SEARCH_VALUE;
        }

        ch++; // This should always run
    }

    if(current_node != dest) // If we are not back at root parsing was unsuccessful
    {
        puts("The final node was not root, invalid json file");
        return false;
    }

    return true;
}

void CPPJP::WriteJson(JSONNode* node, std::string& output_buffer)
{
    // Iterare through all nodes and create a json file
    // Basically parsing in reverse

    // Steps to take:
    // 1. Write the current node data to the output_buffer
    // 2. Search for next node in correct order (child first then next)
    // 3. Set current node to the next node to parse and loop

    // Notes:

    // Nodes of type STRING, NUMBER, TRUE, FALSE, NULL should not have children

    JSONNode* current_node = node;

    bool supress_name_printing = true;

    while(current_node)
    {
        if(!supress_name_printing)
        {
            if(!current_node->name.empty())
            {
                output_buffer += "\"" + current_node->name + "\":";
            }
        }
        else
            supress_name_printing = false;

        switch(current_node->type)
        {
            case JSONNodeType::STRING:
                output_buffer += "\"" + current_node->string_data + "\"";
                break;
            case JSONNodeType::NUMBER:
                output_buffer += current_node->string_data;
                break;
            case JSONNodeType::TRUE:
                output_buffer += "true";
                break;
            case JSONNodeType::FALSE:
                output_buffer += "false";
                break;
            case JSONNodeType::JNULL:
                output_buffer += "null";
                break;

            case JSONNodeType::ARRAY:
                output_buffer += "[";
                if(!current_node->child)
                    output_buffer += ']';
                break;

            case JSONNodeType::OBJECT:
                output_buffer += "{";
                if(!current_node->child)
                    output_buffer += '}';
                break;
        }

        if(current_node->child)
        {
            current_node = current_node->child;
        }
        else if(current_node->next)
        {
            output_buffer += ",";
            current_node = current_node->next;
        }
        else
        {
            while(!current_node->next)
            {
                current_node = current_node->parent;

                // Close the parent node
                if(current_node->type == JSONNodeType::ARRAY)  output_buffer += "]";
                if(current_node->type == JSONNodeType::OBJECT) output_buffer += "}";

                // We have reached the root node
                if(current_node == node) break;
            }

            if(current_node == node) break;

            current_node = current_node->next;
            output_buffer += ",";
        }
    }
}