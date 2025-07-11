# OIJSON
OIJSON is just another implementation of a C json parser(read-only).

# Build
Compile and link oijson.c, include oijson.h to use the library. The included CMake build is for testing.

# Usage
Provide a null terminated json string to oijson_parse to identify objects, arrays or values, then use the desired object, array or value functions.

Example
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

**Q:** Can I edit json fields/save to file?<br>
**A:** No, the library is (currently?) read-only. One of the goals of the library is avoiding allocations an libc dependency, so there are some challenges to changing existing values, altough it is not completely out of the question.

**Q:** Why no oijson_value_as_bool?<br>
**A:** There's no need, you can directly check the type for *oijson_type_true* or *oijson_type_false*. Same for *oijson_type_null*.
