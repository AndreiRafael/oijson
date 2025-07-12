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

typedef struct oijson_s {
    const char* buffer;
    unsigned int size;
    oijson_type type;
} oijson;

const char* oijson_error(void);

oijson oijson_parse(const char* json, unsigned int json_size);

unsigned int oijson_object_count(oijson object);
oijson oijson_object_value_by_name(oijson object, const char* name);
oijson oijson_object_name_by_index(oijson object, unsigned int index);
oijson oijson_object_value_by_index(oijson object, unsigned int index);

unsigned int oijson_array_count(oijson array);
oijson oijson_array_value_by_index(oijson array, unsigned int index);

int oijson_value_formatted(oijson value, char* out, unsigned int out_size);
int oijson_value_as_string(oijson value, char* out, unsigned int out_size);
int oijson_value_as_long(oijson value, long* out);
int oijson_value_as_int(oijson value, int* out);
int oijson_value_as_double(oijson value, double* out);
int oijson_value_as_float(oijson value, float* out);

#endif//OIJSON
