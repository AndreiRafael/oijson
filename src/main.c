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

void print_json(oijson json) {
    switch (json.type) {
        case oijson_type_invalid:
            puts("INVALID JSON!!\n");
            return;
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
    for (unsigned int i = 0; i < json.size; i++) {
        putchar(json.buffer[i]);
    }
    putchar('\n');
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    char buf[2048];
    const char* file = read_file("./res/test.json", buf, 2048);
    oijson json = oijson_parse(file, 2048);
    if (json.type != oijson_type_invalid) {
        print_json(json);
        print_json(oijson_object_value_by_name(json, "float3"));
        print_json(oijson_object_value_by_name(json, "float4"));

        print_json(oijson_object_value_by_index(json, 6));

        printf("object pairs: %d\n", oijson_object_pairs_count(json));
    }
    oijson json2 = oijson_parse("0", 1);
    print_json(json2);
    oijson json3 = oijson_parse("\"test\"", 5);
    print_json(json3);
    return 0;
}
