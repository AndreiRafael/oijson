#include <stdio.h>
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

static void print_indent(unsigned int indent) {
    while (indent) {
        for (int i = 0; i < 4; i++) {
            putchar(' ');
        }
        indent--;
    }
}

static void print_json(oijson);

static void print_json_indented(oijson json, unsigned int indent) {
    switch (json.type) {
        case oijson_type_invalid:
            puts("INVALID JSON!!");
            return;
        case oijson_type_object:
        {
            puts("{");
            unsigned int count = oijson_object_count(json);
            for (unsigned int i = 0; i < count; i++) {
                if (i) {
                    puts(",");
                }
                print_json_indented(oijson_object_name_by_index(json, i), indent + 1);
                putchar(':');
                oijson value = oijson_object_value_by_index(json, i);
                if (value.type == oijson_type_array || value.type == oijson_type_object) {
                    print_json_indented(value, indent + 1);
                }
                else {
                    print_json(value);
                }
            }
            putchar('\n');
            print_indent(indent);
            putchar('}');
            break;
        }
        case oijson_type_array:
        {
            puts("[");
            unsigned int count = oijson_array_count(json);
            for (unsigned int i = 0; i < count; i++) {
                if (i) {
                    puts(",");
                }
                print_json_indented(oijson_array_value_by_index(json, i), indent + 1);
            }
            putchar('\n');
            print_indent(indent);
            putchar(']');
            break;
        }
        default:
            print_indent(indent);
            for (unsigned int i = 0; i < json.size; i++) {
                putchar(json.buffer[i]);
            }
            break;
    }
}

static void print_json(oijson json) {
    print_json_indented(json, 0);
}

static unsigned int string_length(const char* string) {
    unsigned int length = 0;
    while (*string) {
        length++;
        string++;
    }
    return length;
}

static void test_double(const char* value) {
    fputs("testing double: ", stdout);
    fputs(value, stdout);
    oijson json = oijson_parse(value, string_length(value));
    if (json.type == oijson_type_number) {
        double d;
        if (oijson_value_as_double(json, &d)) {
            printf(" == %f\n", d);
        }
        else {
            puts("PARSE ERROR");
        }
    }
    else {
        puts("NOT A NUMBER");
    }
}

static void test_long(const char* value) {
    fputs("testing long: ", stdout);
    fputs(value, stdout);
    oijson json = oijson_parse(value, string_length(value));
    if (json.type == oijson_type_number) {
        long l;
        if (oijson_value_as_long(json, &l)) {
            printf(" == %ld\n", l);
        }
        else {
            puts("PARSE ERROR");
        }
    }
    else {
        puts("NOT A NUMBER");
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    char buf[2048];
    {
        const char* file = read_file("./res/test.json", buf, 2048);
        oijson json = oijson_parse(file, 2048);
        if (json.type != oijson_type_invalid) {
            print_json(json);
            print_json(oijson_object_value_by_name(json, "float3"));
            print_json(oijson_object_value_by_name(json, "float4"));

            print_json(oijson_object_value_by_index(json, 6));

            printf("object pairs: %d\n", oijson_object_count(json));
        }
        oijson json2 = oijson_parse("0", 1);
        print_json(json2);
        oijson json3 = oijson_parse("\"test\"", 6);
        print_json(json3);
    }
    {
        const char* file = read_file("./res/test2.json", buf, 2048);
        oijson json = oijson_parse(file, 2048);
        if (json.type != oijson_type_invalid) {
            print_json(json);
            print_json(oijson_object_value_by_name(json, "glossary"));
        }
    }

    test_double("0");
    test_double("-0");
    test_double("0.12345");
    test_double("-0.12345");
    test_double("10.5e10");
    test_double("-10.5e10");
    test_double("10.5e-3");
    test_double("-10.5e-3");

    test_long("0");
    test_long("-0");
    test_long("0.5");
    test_long("0.51");
    test_long("-0.5");
    test_long("-0.51");
    test_long("10.51e7");
    test_long("-10.51e7");
    test_long("19.5e-1");
    test_long("-19.5e-1");
    test_long("19.51e-1");
    test_long("-19.51e-1");
    return 0;
}
