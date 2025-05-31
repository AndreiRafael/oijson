#ifndef OIJSON
#define OIJSON

typedef enum oijson_type_e {
    oijson_type_invalid = 0,
    oijson_type_string,
    oijson_type_number,
    oijson_type_object,
    oijson_type_array,
    oijson_type_true,
    oijson_type_false,
    oijson_type_null,
} oijson_type;

const char* oijson_error(void);

const char* oijson_parse(const char* json, oijson_type* out_type);

unsigned int oijson_object_pairs_count(const char* object);
const char* oijson_object_value_by_name(const char* object, const char* name);
const char* oijson_object_name_by_index(const char* object, unsigned int index);
const char* oijson_object_value_by_index(const char* object, unsigned int index);

unsigned int oijson_array_values_count(const char* array);
const char* oijson_array_value_by_index(const char* array, unsigned int index);

int oijson_value_as_string(const char* value, char* out_buffer, unsigned int buffer_size);
int oijson_value_as_int(const char* value, int* out_int);
int oijson_value_as_float(const char* value, float* out_float);

#endif//OIJSON
