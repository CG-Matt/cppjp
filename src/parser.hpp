#pragma once

#include <string>
#include "../include/cppjp.hpp"

/*
    Parses a json string into a json object
    @param json_str The JSON string to parse
    @param dest The destination for the resulting JSON structure
    @return 0 is successful, otherwise error code
*/
JSONNode* ParseJSON(const char* json_str, JSONNode* dest);

void WriteJson(JSONNode* node, std::string& output_buffer);