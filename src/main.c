#include <stdio.h>
#include <string.h>

#include "oijson.h"

const char* read_file(const char* path, char* buffer, unsigned int buffer_size) {
    FILE* file = fopen(path, "r");
    if (!file) {
        return (const char*)0;
    }

    unsigned int index = 0;

    int ci;
    while ((ci = fgetc(file)) != EOF) {
        if (index >= buffer_size) {
            return (const char*)0;
        }
        buffer[index++] = (char)ci;
    }
    fclose(file);
    if (index >= buffer_size) {
        return (const char*)0;
    }
    buffer[index] = '\0';
    return buffer;
}

void print_json(const char* json) {
    oijson_type type;
    const char* end = oijson_parse(json, &type);

    if (!end) {
        printf("ERROR");
        return;
    }

    switch (type) {
        case oijson_type_object:
            puts("object\n");
            break;
        case oijson_type_array:
            puts("array\n");
            break;
        case oijson_type_string:
            puts("string\n");
            break;
        case oijson_type_number:
            puts("number\n");
            break;
        default:
            break;
    }
    while (json && end && json != end) {
        putchar(*json);
        json++;
    }
    putchar('\n');
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    char buf[2048];
    const char* json = read_file("./res/test.json", buf, 2048);
    print_json(json);
    print_json(oijson_object_value_by_name(json, "float3"));
    print_json(oijson_object_value_by_name(json, "float4"));

    return 0;
}
