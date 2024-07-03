#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include "include/cppjp.hpp"
#include "src/parser.hpp"

static char* program_name = nullptr;

static void print_usage()
{
    printf("Usage: %s json_file\n", program_name);
}

int main(int argc, char** argv)
{
    program_name = argv[0];
    if(argc < 2){ print_usage(); return 1; }
    std::string file_contents;
    std::ifstream file(argv[1]);
    if(!file.is_open()){ printf("Unable to open file \"%s\" for reading, exiting...\n", argv[1]); return 1; }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file_contents = buffer.str();
    JSON json_file(file_contents.data());
    printf("The JSON file is of type %s\n", json_file.getTypeCString());
    printf("Its structure is:\n%s\n", json_file.asPrintable().data());
    std::string output_buffer;
    json_file.writeOut(output_buffer);
    printf("%s\n", file_contents == output_buffer ? "true" : "false");
    return 0;
}