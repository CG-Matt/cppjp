# CPPJP - The C++ JSON Parser
---
This is a simple still work in progress C++ JSON Parser which provides access to both raw JSON object structs and a wrapper JSON class for easier interaction with the data.

#### Currently Implemented Features
- Reading in and writing out of JSON data
- Extracting data from JSON objects as:
    - C String
    - C++ String
    - Unsigned Integer
    - Signed Integer
    - Float (Double)
    - Boolean
- Checking if the JSON object is null
- Checking if a sub-object exists with a given key
- Accessing of sub-objects by key
- Acessing of entries in an array by index
- Getting the size of an array
- Iterating over an array with a callback function
- Iterating over an object with a callback function
- Getting a printable string representation of the object
- Deleting objects

#### Currently Unimplmented Features
- Abilitiy to add new nodes to existing objects
- Ability to implant objects as sub-objects in other objects

#### Building the library
There is a makefile provided with this library.
Use ```make all``` to compile all library types (.a and .so) and the test file.
Use ```make test``` to compile only as a test file.
Use ```make lib``` to compile to both ```.a``` and ```.so``` libraries.
Use ```make static_lib``` to compile to a static library.
Use ```make dynamic_lib``` to compile to a dynamic library.