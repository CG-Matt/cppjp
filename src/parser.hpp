#pragma once

#include <string>
#include "../include/cppjp.hpp"

namespace CPPJP
{
    void WriteJson(JSONNode* node, std::string& output_buffer);
}