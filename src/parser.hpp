#pragma once

#include <string>
#include "../include/cppjp.hpp"

/*
    Parses a string of JSON data into a JSON Node object
    @param json_str The JSON string to parse
    @param dest The destination for the resulting JSON structure
    @return ```dest``` if successful, ```nullptr``` otherwise.
*/
JSONNode* ParseJSON(const char* json_str, JSONNode* dest);

void WriteJson(JSONNode* node, std::string& output_buffer);