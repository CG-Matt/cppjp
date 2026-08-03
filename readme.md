# CPPJP

CPPJP is a work-in-progress C++ JSON parser. It exposes both raw `JSONNode`
structures and a `JSON` wrapper for querying and managing JSON trees.

## Features

- Parse JSON strings and write JSON back to a string.
- Read strings, numbers, booleans, and null values.
- Access object entries and array elements.
- Check object keys, array sizes, and node types.
- Iterate over objects and arrays.
- Clone JSON trees with deep copies.
- Wrap, adopt, release, detach, and erase JSON nodes.

## Building

The supplied Makefile provides the following targets:

- `make all` builds the test executable and both library types.
- `make test` builds `build/cppjp-test`.
- `make lib` builds both static and shared libraries.
- `make static_lib` builds `build/libcppjp.a`.
- `make dynamic_lib` builds `build/libcppjp.so`.

Run the test executable with a JSON file:

```sh
./build/cppjp-test path/to/file.json
```

## Basic usage

```cpp
#include "cppjp.hpp"

#include <iostream>

int main()
{
    JSON document = JSON::FromJSONString(R"({"name":"Ada","age":36})");

    JSON name = document.getEntry("name");
    std::cout << name.asString() << '\n';

    JSON independent_copy = name.clone();
}
```

## Ownership

`JSON::FromJSONString()` returns an owning JSON object. Objects returned by
`getEntry()` and `getElement()` are non-owning views and remain valid only
while their original tree remains alive.

- `JSON::Wrap(node)` creates a non-owning view of `node`.
- `JSON::Adopt(node)` transfers ownership of `node` to a new `JSON` object.
  The node must not already be owned by another `JSON` object.
- `clone()` creates an independent, owning deep copy.
- `detach()` removes a node from its parent tree and returns an owning JSON
  object for the detached node.
- `erase()` removes a node from its parent tree and deletes it along with its
  descendants.
- `release()` returns the raw node pointer without deleting it. If the JSON
  object owned the node, the caller becomes responsible for freeing it.

## Iteration

The callback passed to `iterate()` or `iterateObject()` may erase its current
node. Modifying or erasing any other node in the iterated tree invalidates the
iteration.

## Planned

- Add nodes to existing objects.
- Insert existing nodes into other objects.
