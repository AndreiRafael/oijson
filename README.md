# OIJSON
OIJSON is just another implementation of a C JSON parser(read-only).

Goals:
- Tiny.
- No allocations.
- No dependencies.
- Strictly compliant to RFC 8259.

# Build
Compile and link oijson.c, include oijson.h to use the library. The included CMake build is for testing.

# Usage
Provide a null terminated JSON string to oijson_parse to identify objects, arrays or values, then use the desired object, array or value functions.

Example:
```C
// read from string
const char* json_string = "{ \"name\" : \"Jack\", \"age\" : 44 }";
oijson json = oijson_parse(json_string, strlen(json_string) + 1);

// get string value into buffer
char name[20] = "";
oijson_value_as_string(oijson_object_value_by_name(json, "name"), name, 20);

// get number value as integer
int age;
oijson_value_as_int(oijson_object_value_by_name(json, "age"), &age);

printf("%s is %d years old", name, age);
```

# Documentation
Check [Documentation.md](Documentation.md)

# FAQ

**Q: How do I read from a file?**<br>
**A:** Read the contents of the file into a buffer of type char*. Null terminate the contents and provide said buffer and its size to [oijson_parse](Documentation.md#oijson_parse). While operating on the returned [oijson](Documentation.md#oijson) struct, do NOT modify the contents of the buffer.

**Q: Can I edit JSON fields/save to file?**<br>
**A:** No, the library is *(currently?)* read-only. One of the goals of the library is avoiding allocations an libc dependency, so there are some challenges to changing existing values, altough it is not completely out of the question.

**Q: Why no oijson_value_as_bool?**<br>
**A:** There's no need, since you can directly check the type for *oijson_type_true* or *oijson_type_false*. Same for *oijson_type_null*.

# Known Issue

**Large and small numbers**
Number parsing is somewhat rudimentary and may lead to incorrect values and even C undefined behaviour when reading large numbers. Reading small numbers as int or long will also lead to absurd rounding errors. Suggestions are welcome!
