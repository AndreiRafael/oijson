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

static void print_json_nobr(oijson);

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
                    print_json_nobr(value);
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

static void print_json_nobr(oijson json) {
    print_json_indented(json, 0);
}

static void print_json(oijson json) {
    print_json_indented(json, 0);
    putchar('\n');
}

static unsigned int string_length(const char* string) {
    unsigned int length = 0;
    while (*string) {
        length++;
        string++;
    }
    return length;
}

static int string_equal(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

static int test_json_type(const char* json_string, oijson_type expected) {
    printf("TEST JSON STRING: input: %s -> expected: %d -> got: ", json_string, expected);
    oijson json = oijson_parse(json_string, string_length(json_string));
    printf("%d\n", json.type);
    if (json.type == oijson_type_invalid) {
        puts(oijson_error());
    }
    return json.type == expected;
}

static int test_value_by_name(oijson object, const char* name, const char* expected) {
    printf("TEST VALUE BY NAME: input: %s -> expected: %s -> got: ", name, expected);
    oijson json = oijson_object_value_by_name(object, name);
    if (json.type != oijson_type_invalid) {
        print_json(json);
        for (unsigned int i = 0; i < json.size; i++) {
            if (!(*expected) || *expected != json.buffer[i]) {
                return 0;
            }
            expected++;
        }
        return 1;
    }

    puts(oijson_error());
    return 0;
}

static int test_double(const char* value, const char* expected) {
    printf("TEST DOUBLE: input: %s -> expected: %s -> got: ", value, expected);
    oijson json = oijson_parse(value, string_length(value));
    if (json.type == oijson_type_number) {
        double d;
        if (oijson_value_as_double(json, &d)) {
            char buffer[100];
            sprintf(buffer, "%lf", d);
            puts(buffer);
            return string_equal(buffer, expected);
        }
    }
    puts(oijson_error());
    return 0;
}

static int test_long(const char* value, const char* expected) {
    printf("TEST LONG: input: %s -> expected: %s -> got: ", value, expected);
    oijson json = oijson_parse(value, string_length(value));
    if (json.type == oijson_type_number) {
        long l;
        if (oijson_value_as_long(json, &l)) {
            char buffer[100];
            sprintf(buffer, "%ld", l);
            puts(buffer);
            return string_equal(buffer, expected);
        }
    }
    puts(oijson_error());
    return 0;
}

static int test_string(const char* string, const char* expected, char* buffer, unsigned int buffer_size) {
    puts("STRING TEST");
    printf("input: %s -> expected: %s -> got: ", string, expected);
    oijson json = oijson_parse(string, string_length(string));
    if (json.type != oijson_type_invalid) {
        if (oijson_value_as_string(json, buffer, buffer_size)) {
            puts(buffer);
            while (*buffer && *expected && *buffer == *expected) {
                buffer++;
                expected++;
            }
            return !(*buffer) && !(*expected);
        }
    }
    puts(oijson_error());
    return 0;
}

static int tests_passed = 0;
static int tests_count = 0;
static int tests_passed_partial = 0;
static int tests_count_partial = 0;

void check_test(int test_result, int expected_result) {
    printf("test returned %d, and %d was expected ", test_result, expected_result);
    tests_count++;
    tests_count_partial++;
    if (test_result == expected_result) {
        tests_passed++;
        tests_passed_partial++;
        puts("[PASSED]");
        return;
    }
    puts("[FAILED]");
}

void print_test_results(const char* tag, int passed, int total) {
    printf("\nTEST RESULTS(%s): %.2d/%.2d passed(%d failed)\n", tag, passed, total, total - passed);
}

void report_partial_tests(const char* tag) {
    print_test_results(tag, tests_passed_partial, tests_count_partial);
    tests_passed_partial = 0;
    tests_count_partial = 0;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    char buf[2048];
    {
        const char* file = read_file("./res/test.json", buf, 2048);
        oijson json = oijson_parse(file, string_length(file));
        check_test(json.type != oijson_type_invalid, 1);
        check_test(oijson_object_count(json) == 8, 1);

        check_test(test_json_type("\"abc\"", oijson_type_string), 1);
        check_test(test_json_type("\"\\udead\"", oijson_type_invalid), 1);

        check_test(test_json_type("true", oijson_type_true), 1);
        check_test(test_json_type("false", oijson_type_false), 1);
        check_test(test_json_type("null", oijson_type_null), 1);

        check_test(test_json_type("1", oijson_type_number), 1);
        check_test(test_json_type("1.2", oijson_type_number), 1);
        check_test(test_json_type("1.2e3", oijson_type_number), 1);
        check_test(test_json_type("01.2e3", oijson_type_invalid), 1);// leading zero

        check_test(test_json_type("{}", oijson_type_object), 1);
        check_test(test_json_type("{,}", oijson_type_invalid), 1);
        check_test(test_json_type("{\"a\":0}", oijson_type_object), 1);
        check_test(test_json_type("{\"a\":0,\"b\":1}", oijson_type_object), 1);
        check_test(test_json_type("{\"a\":0,\"b\":1,}", oijson_type_invalid), 1);
        check_test(test_json_type("{\"a\"0}", oijson_type_invalid), 1);

        check_test(test_json_type("[]", oijson_type_array), 1);
        check_test(test_json_type("[,]", oijson_type_invalid), 1);
        check_test(test_json_type("[0,1,2]", oijson_type_array), 1);
        check_test(test_json_type("[0,1,2,]", oijson_type_invalid), 1);
        check_test(test_json_type("[0,1,2", oijson_type_invalid), 1);

        report_partial_tests("object");

        if (json.type != oijson_type_invalid) {
            print_json(json);
            check_test(test_value_by_name(json, "float2", "10.0e-10"), 1);
            check_test(test_value_by_name(json, "float\\u0032", "10.0e-10"), 1);
            check_test(test_value_by_name(json, "float3", "-10.0e2"), 1);
            check_test(test_value_by_name(json, "float\\u0033", "-10.0e2"), 1);
            check_test(test_value_by_name(json, "float4", "not found"), 0);
            check_test(test_value_by_name(json, "float\\u0034", "not found"), 0);

            report_partial_tests("value by name");
        }
        oijson json2 = oijson_parse("0", 1);
        print_json(json2);
        oijson json3 = oijson_parse("\"test\"", 6);
        print_json(json3);
    }

    check_test(test_double("0", "0.000000"), 1);
    check_test(test_double("-0", "0.000000"), 1);
    check_test(test_double("0.12345", "0.123450"), 1);
    check_test(test_double("-0.12345", "-0.123450"), 1);
    check_test(test_double("10.5e7", "105000000.000000"), 1);
    check_test(test_double("-10.5e7", "-105000000.000000"), 1);
    check_test(test_double("10.5e-3", "0.010500"), 1);
    check_test(test_double("-10.5e-3", "-0.010500"), 1);

    report_partial_tests("double conversion");

    check_test(test_long("0", "0"), 1);
    check_test(test_long("-0", "0"), 1);
    check_test(test_long("0.5", "0"), 1);
    check_test(test_long("0.51", "1"), 1);
    check_test(test_long("-0.5", "0"), 1);
    check_test(test_long("-0.51", "-1"), 1);
    check_test(test_long("10.51e7", "110000000"), 1);
    check_test(test_long("-10.51e7", "-110000000"), 1);
    check_test(test_long("19.5e-1", "1"), 1);
    check_test(test_long("-19.5e-1", "-1"), 1);
    check_test(test_long("19.51e-1", "2"), 1);
    check_test(test_long("-19.51e-1", "-2"), 1);

    report_partial_tests("long conversion");

    {
        char string[10];
        check_test(test_string("\"ab\\nc\"", "ab\nc", string, 10), 1);// success - c is printed in newline
        check_test(test_string("\"ab\\nc\"", "buffer too small", string, 4), 0);// fails - no space for null terminator

        check_test(test_string("\"\\u002F\"", "/", string, 5), 1);// success - prints /
        check_test(test_string("\"/\"", "/", string, 5), 1);// success - prints /
        check_test(test_string("\"\\/\"", "/", string, 5), 1);// success - prints /

        check_test(test_string("\"\\uD834\\uDD1E\"", "𝄞", string, 10), 1);
        check_test(test_string("\"\\udead\"", "invalid escaped unicode", string, 10), 0);// should fail because of invalid escaped unicode
        check_test(test_string("\"\\uffff\\uffff\"", "invalid escaped unicode", string, 10), 0);// should fail because of invalid escaped unicode
        check_test(test_string("\"\\\n\"", "invalid escaped control character", string, 10), 0);// should fail because of invalid escaped unicode
        check_test(test_string("\"\n\"", "unescaped control character", string, 10), 0);// should fail because of invalid escaped unicode
        
        report_partial_tests("string escaping");
        
        print_test_results("TOTAL", tests_passed, tests_count);
    }
    return 0;
}
