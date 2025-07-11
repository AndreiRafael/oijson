# Enums

### oijson_type

The type of a json value. _oijson_type_invalid_ does not direcly map to a json type and is used when a json string fails to be parsed due to an error. In such cases, [oijson_error](#oijson_error) can be called for an error message.

|Value               |Json type          |
|--------------------|-------------------|
|oijson_type_invalid |None, invalid json |
|oijson_type_string  |string             |
|oijson_type_number  |number             |
|oijson_type_object  |object             |
|oijson_type_array   |array              |
|oijson_type_true    |true               |
|oijson_type_false   |false              |
|oijson_type_null    |null               |



# Structs

### oijson {#oijson}



# Functions

### const char* oijson_error(void) {#oijson-error}
Returns the pointer to a stack allocated error string. The string is guaranteed to be null terminated and is updated with any function call, becoming empty when a function succeeds.



### oijson oijson_parse(const char* json, unsigned int json_size) {#oijson-parse}
Returns an [oijson](#oijson) struct. The type field of the returned struct indicates if the operation was successful, with a value of _oijson_type_invalid_ indicating failure. Use [oijson_error](#oijson-error) for details.

|Parameter |Description |
|:---------|:-----------|
|json      | Buffer containing null-terminated json string |
|json_size | Size of buffer |


unsigned int oijson_object_count(oijson object);
oijson oijson_object_value_by_name(oijson object, const char* name);
oijson oijson_object_name_by_index(oijson object, unsigned int index);
oijson oijson_object_value_by_index(oijson object, unsigned int index);

unsigned int oijson_array_count(oijson array);
oijson oijson_array_value_by_index(oijson array, unsigned int index);

int oijson_value_as_string(oijson value, char* out, unsigned int out_size);
int oijson_value_as_long(oijson value, long* out);
int oijson_value_as_int(oijson value, int* out);
int oijson_value_as_double(oijson value, double* out);
### int oijson_value_as_float(oijson value, float* out) {#oijson-value-as-float}