#pragma once

#include <string>
#include "cppjp.hpp"

namespace CPPJP
{
    void WriteJson(JSONNode* node, std::string& output_buffer);
}