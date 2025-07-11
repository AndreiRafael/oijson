# OIJSON
OIJSON is just another implementation of a c json parser(read only).

# Build
Compile and link oijson.c, include oijson.h to use the library. The included CMake build is for testing.

# Usage
Provide a null terminated json string to oijson_parse to identify objects, arrays or values, then use the desired object, array or value functions.

Example
```C
const char* json_string = "{ \"name\" : \"Jack\", \"age\" : 44 }";
oijson json = oijson_parse(json_string, strlen(json_string) + 1);
char name[20] = "";
oijson_value_as_string(oijson_object_value_by_name(json, "name"), name, 20);
int age;
oijson_value_as_int(oijson_object_value_by_name(json, "age"), &age);
printf("%s is %d years old", name, age);
```

# Documentation
TODO
