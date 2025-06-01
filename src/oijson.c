#include "oijson.h"

#define OIJSON_NULLCHAR ((const char*)0)
#define OIJSON_INVALID ((oijson) { .buffer = OIJSON_NULLCHAR, .size = 0, .type = oijson_type_invalid })
#define OIJSON_STEP_ITR() do { itr++; if (!(*size)) { return OIJSON_NULLCHAR; } (*size)--; } while(0)
#define OIJSON_CHECK_ITR() do { if(!itr || !(*size)) { return OIJSON_NULLCHAR; } } while(0)

static char oijson_internal_error[128] = "";

const char* oijson_error(void) {// TODO: set error messages upon errors
    return oijson_internal_error;
}

static int oijson_internal_is_whitespace(const char c) {
    switch (c) {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
            return 1;
    }
    return 0;
}

static int oijson_internal_is_digit(const char c) {
    return c >= '0' && c <= '9';
}

static int oijson_internal_is_hex_digit(const char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||(c >= 'a' && c <= 'f');
}

static const char* oijson_internal_consume_whitespace(const char* itr, unsigned int* size) {
    OIJSON_CHECK_ITR();
    while (*size && oijson_internal_is_whitespace(*itr)) {
        OIJSON_STEP_ITR();
    }
    return *size ? itr : OIJSON_NULLCHAR;
}

static const char* oijson_internal_consume_keyword(const char* itr, unsigned int* size, const char* keyword) {
    itr = oijson_internal_consume_whitespace(itr, size);
    OIJSON_CHECK_ITR();
    while(*size && *keyword && *itr == *keyword) {
        keyword++;
        OIJSON_STEP_ITR();
    }
    return *keyword ? OIJSON_NULLCHAR : itr;
}

static const char* oijson_internal_consume_true(const char* itr, unsigned int* size) {
    return oijson_internal_consume_keyword(itr, size, "true");
}

static const char* oijson_internal_consume_false(const char* itr, unsigned int* size) {
    return oijson_internal_consume_keyword(itr, size, "false");
}

static const char* oijson_internal_consume_null(const char* itr, unsigned int* size) {
    return oijson_internal_consume_keyword(itr, size, "null");
}

static const char* oijson_internal_consume_string(const char* itr, unsigned int* size) {
    itr = oijson_internal_consume_whitespace(itr, size);
    OIJSON_CHECK_ITR();

    if (*itr != '\"') {
        return OIJSON_NULLCHAR;
    }
    OIJSON_STEP_ITR();
    while (*size) {
        switch (*itr) {
            case '\\':
                OIJSON_STEP_ITR();
                OIJSON_CHECK_ITR();
                switch(*itr) {
                    case '\"':
                    case '\\':
                    case '/':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                        OIJSON_STEP_ITR();
                        break;
                    case 'u':
                        OIJSON_STEP_ITR();
                        for (int i = 0; i < 4; i++) {
                            OIJSON_CHECK_ITR();
                            if (!oijson_internal_is_hex_digit(*itr)) {
                                return OIJSON_NULLCHAR;
                            }
                            OIJSON_STEP_ITR();
                        }
                    default:
                        return OIJSON_NULLCHAR;// error, invalid character
                }
                break;
            case '\0':
                return OIJSON_NULLCHAR;
            case '\"':
                OIJSON_STEP_ITR();
                return itr;
            default:
                OIJSON_STEP_ITR();
                break;
        }
    }
    return OIJSON_NULLCHAR;
}

static const char* oijson_internal_consume_number(const char* itr, unsigned int* size) {
    itr = oijson_internal_consume_whitespace(itr, size);
    OIJSON_CHECK_ITR();

    if (*itr == '-') {
        OIJSON_STEP_ITR();
        OIJSON_CHECK_ITR();
    }

    if (*itr == '0') {// straight to fraction
        OIJSON_STEP_ITR();
    }
    else {
        if (!(*size) || !oijson_internal_is_digit(*itr)) {
            return OIJSON_NULLCHAR;
        }
        OIJSON_STEP_ITR();
        while (*size && oijson_internal_is_digit(*itr)) {
            OIJSON_STEP_ITR();
        }
    }

    if (*itr == '.') {// fraction
        OIJSON_STEP_ITR();
        if (!(*size) || !oijson_internal_is_digit(*itr)) {
            return OIJSON_NULLCHAR;
        }
        OIJSON_STEP_ITR();
        while (*size && oijson_internal_is_digit(*itr)) {
            OIJSON_STEP_ITR();
        }
    }

    if (*size && (*itr == 'e' || *itr == 'E')) {
        OIJSON_STEP_ITR();
        OIJSON_CHECK_ITR();
        if (*itr == '-' || *itr == '+') {
            OIJSON_STEP_ITR();
            OIJSON_CHECK_ITR();
        }
        if (!oijson_internal_is_digit(*itr)) {
            return OIJSON_NULLCHAR;
        }
        OIJSON_STEP_ITR();
        while (*size && oijson_internal_is_digit(*itr)) {
            OIJSON_STEP_ITR();
        }
    }
    return itr;
}
static const char* oijson_internal_consume_object(const char*, unsigned int*);
static const char* oijson_internal_consume_array(const char*, unsigned int*);

static const char* oijson_internal_consume_value(const char* itr, unsigned int* size) {
    const char*(*functions[])(const char*, unsigned int*) = {
        oijson_internal_consume_string,
        oijson_internal_consume_number,
        oijson_internal_consume_object,
        oijson_internal_consume_array,
        oijson_internal_consume_true,
        oijson_internal_consume_false,
        oijson_internal_consume_null,
    };
    int len = sizeof(functions) / sizeof(functions[0]);

    for (int i = 0; i < len; i++) {
        unsigned int temp_size = *size;
        const char* temp_itr = functions[i](itr, &temp_size);
        if (temp_itr) {
            *size = temp_size;
            return temp_itr;
        }
    }
    return OIJSON_NULLCHAR;
}

static const char* oijson_internal_consume_key_value_pair(const char* itr, unsigned int* size, unsigned int* size_value, const char** key_start, const char** key_end, const char** value_start) {
    itr = oijson_internal_consume_whitespace(itr, size);
    OIJSON_CHECK_ITR();

    // read name
    if (key_start) {
        *key_start = itr;
    }
    itr = oijson_internal_consume_string(itr, size);
    if (!itr) {
        return OIJSON_NULLCHAR;
    }
    if (key_end) {
        *key_end = itr;
    }

    itr = oijson_internal_consume_whitespace(itr, size);
    if (!itr || *itr != ':') {
        return OIJSON_NULLCHAR;
    }

    OIJSON_STEP_ITR();// skip ':'
    if (size_value) {
        *size_value = *size;
    }
    if (value_start) {
        *value_start = itr;
    }
    itr = oijson_internal_consume_value(itr, size);
    return itr;
}

static const char* oijson_internal_consume_object(const char* itr, unsigned int* size) {
    itr = oijson_internal_consume_whitespace(itr, size);
    OIJSON_CHECK_ITR();
    if (*itr != '{') {
        return OIJSON_NULLCHAR;
    }

    OIJSON_STEP_ITR();// skip '{'
    itr = oijson_internal_consume_whitespace(itr, size);
    while(1) {
        itr = oijson_internal_consume_key_value_pair(itr, size, 0, 0, 0, 0);

        itr = oijson_internal_consume_whitespace(itr, size);
        if (!itr) {
            return OIJSON_NULLCHAR;
        }
        if (*itr == '}') {
            OIJSON_STEP_ITR();
            break;
        }
        if (*itr != ',') {
            return OIJSON_NULLCHAR;
        }
        OIJSON_STEP_ITR();// step over comma
    }
    return itr;
}

static const char* oijson_internal_consume_array(const char* itr, unsigned int* size) {
    itr = oijson_internal_consume_whitespace(itr, size);
    OIJSON_CHECK_ITR();
    if (*itr != '[') {
        return OIJSON_NULLCHAR;
    }

    OIJSON_STEP_ITR();
    itr = oijson_internal_consume_whitespace(itr, size);
    while(1) {
        itr = oijson_internal_consume_value(itr, size);// value
        if (!itr) {
            return OIJSON_NULLCHAR;
        }

        itr = oijson_internal_consume_whitespace(itr, size);
        if (!itr) {
            return OIJSON_NULLCHAR;
        }
        OIJSON_CHECK_ITR();
        if (*itr == ']') {
            OIJSON_STEP_ITR();
            break;
        }
        if (*itr != ',') {
            return OIJSON_NULLCHAR;
        }
        OIJSON_STEP_ITR();// step over comma
    }
    return itr;
}

oijson oijson_parse(const char* string, unsigned int string_size) {
    const char* itr = oijson_internal_consume_whitespace(string, &string_size);
    if (!itr) {
        return OIJSON_INVALID;
    }

    oijson out_json = {
        .buffer = string,
        .size = string_size,
        .type = oijson_type_invalid
    };

    const char*(*consume_functions[])(const char*, unsigned int*) = {
        oijson_internal_consume_string,
        oijson_internal_consume_number,
        oijson_internal_consume_object,
        oijson_internal_consume_array,
        oijson_internal_consume_true,
        oijson_internal_consume_false,
        oijson_internal_consume_null,
    };
    oijson_type types[] = {
        oijson_type_string,
        oijson_type_number,
        oijson_type_object,
        oijson_type_array,
        oijson_type_true,
        oijson_type_false,
        oijson_type_null,
    };
    int len = sizeof(types) / sizeof(types[0]);

    for (int i = 0; i < len; i++) {
        unsigned int temp_size = string_size;
        const char* itr = consume_functions[i](string, &temp_size);
        if (itr) {
            out_json.size = string_size - temp_size;
            out_json.type = types[i];
            break;
        }
    }
    return out_json;
}


unsigned int oijson_object_pairs_count(oijson object) {
    if (object.type != oijson_type_object) {
        return 0;
    }

    unsigned int size = object.size;
    const char* itr = oijson_internal_consume_whitespace(object.buffer, &size);
    itr++;

    unsigned int count = 0;

    while (1) {
        itr = oijson_internal_consume_key_value_pair(itr, &size, 0, 0, 0, 0);
        if (!itr) {
            break;
        }

        count++;

        itr = oijson_internal_consume_whitespace(itr, &size);
        if (!itr || *itr != ',') {
            break;
        }
        itr++;
    }
    return count;
}

static int oijson_internal_check_value(const char* start, const char* end, const char* name) {
    start++;
    end--;

    while(*start && *name && *start == *name) {
        start++;
        name++;
    }
    return !(*name) && start == end;
}

oijson oijson_object_value_by_name(oijson object, const char* name) {
    if (object.type != oijson_type_object || !name) {
        return OIJSON_INVALID;
    }

    unsigned int size = object.size;
    const char* itr = oijson_internal_consume_whitespace(object.buffer, &size);
    itr++;

    while (1) {
        const char* start;
        const char* end;
        const char* value;
        unsigned int temp_size;
        itr = oijson_internal_consume_key_value_pair(itr, &size, &temp_size, &start, &end, &value);
        if (!itr) {
            break;
        }

        if (oijson_internal_check_value(start, end, name)) {
            return oijson_parse(value, temp_size);
        }

        itr = oijson_internal_consume_whitespace(itr, &size);
        if (*itr != ',') {
            break;
        }
        itr++;
    }
    return OIJSON_INVALID;
}

oijson oijson_object_value_by_index(oijson object, unsigned int index) {
    if (object.type != oijson_type_object) {
        return OIJSON_INVALID;
    }

    unsigned int size = object.size;
    const char* itr = oijson_internal_consume_whitespace(object.buffer, &size);
    itr++;

    while (1) {
        const char* value;
        unsigned int temp_size;
        itr = oijson_internal_consume_key_value_pair(itr, &size, &temp_size, 0, 0, &value);
        if (!itr) {
            break;
        }

        if (!index) {
            return oijson_parse(value, temp_size);
        }
        index--;

        itr = oijson_internal_consume_whitespace(itr, &size);
        if (*itr != ',') {
            break;
        }
        itr++;
    }
    return OIJSON_INVALID;
}

oijson oijson_object_name_by_index(oijson object, unsigned int index) {
    if (object.type != oijson_type_object) {
        return OIJSON_INVALID;
    }

    unsigned int size = object.size;
    const char* itr = oijson_internal_consume_whitespace(object.buffer, &size);
    itr++;

    while (1) {
        if (!index) {
            return oijson_parse(itr, size);
        }
        index--;

        itr = oijson_internal_consume_key_value_pair(itr, &size, 0, 0, 0, 0);
        if (!itr) {
            break;
        }

        itr = oijson_internal_consume_whitespace(itr, &size);
        if (*itr != ',') {
            break;
        }
        itr++;
    }
    return OIJSON_INVALID;
}
