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

static const char* oijson_internal_consume_char(const char* itr, unsigned int* size) {
    OIJSON_CHECK_ITR();
    OIJSON_STEP_ITR();
    return itr;
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
                        break;
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

static const char* oijson_internal_consume_number_info(const char* itr, unsigned int* size, const char** out_integer, unsigned int* out_integer_size, const char** out_fraction, unsigned int* out_fraction_size, const char** out_exponent, unsigned int* out_exponent_size) {
    itr = oijson_internal_consume_whitespace(itr, size);
    OIJSON_CHECK_ITR();

    if (out_integer) {
        *out_integer = itr;
        if (out_integer_size) {
            *out_integer_size = *size;
        }
    }

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
        if (out_fraction) {
            *out_fraction = itr;
            if (out_fraction_size) {
                *out_fraction_size = *size;
            }
        }

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
        if (out_exponent) {
            *out_exponent = itr;
            if (out_exponent_size) {
                *out_exponent_size = *size;
            }
        }

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

static const char* oijson_internal_consume_number(const char* itr, unsigned int* size) {
    return oijson_internal_consume_number_info(itr, size, 0, 0, 0, 0, 0, 0);
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
    string = oijson_internal_consume_whitespace(string, &string_size);
    if (!string) {
        return OIJSON_INVALID;
    }

    oijson out_json = {
        .buffer = OIJSON_NULLCHAR,
        .size = 0,
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
            out_json.buffer = string;
            out_json.size = string_size - temp_size;
            out_json.type = types[i];
            break;
        }
    }
    return out_json;
}


unsigned int oijson_object_count(oijson object) {
    if (object.type != oijson_type_object) {
        return 0;
    }

    unsigned int size = object.size;
    const char* itr = oijson_internal_consume_whitespace(object.buffer, &size);
    itr = oijson_internal_consume_char(itr, &size);// skip '{'

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
        itr = oijson_internal_consume_char(itr, &size);// skip ','
    }
    return count;
}

static int oijson_internal_check_value(const char* start, const char* end, const char* name) {
    start++;
    end--;

    // TODO: check with unicode!!
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
    itr = oijson_internal_consume_char(itr, &size);// skip '{'

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
        if (!itr || *itr != ',') {
            break;
        }
        itr = oijson_internal_consume_char(itr, &size);// skip ','
    }
    return OIJSON_INVALID;
}

oijson oijson_object_value_by_index(oijson object, unsigned int index) {
    if (object.type != oijson_type_object) {
        return OIJSON_INVALID;
    }

    unsigned int size = object.size;
    const char* itr = oijson_internal_consume_whitespace(object.buffer, &size);
    itr = oijson_internal_consume_char(itr, &size);// skip '{'

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
        if (!itr || *itr != ',') {
            break;
        }
        itr = oijson_internal_consume_char(itr, &size);// skip ','
    }
    return OIJSON_INVALID;
}

oijson oijson_object_name_by_index(oijson object, unsigned int index) {
    if (object.type != oijson_type_object) {
        return OIJSON_INVALID;
    }

    unsigned int size = object.size;
    const char* itr = oijson_internal_consume_whitespace(object.buffer, &size);
    itr = oijson_internal_consume_char(itr, &size);// skip '{'

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
        if (!itr || *itr != ',') {
            break;
        }
        itr = oijson_internal_consume_char(itr, &size);// skip ','
    }
    return OIJSON_INVALID;
}


unsigned int oijson_array_count(oijson array) {
    if (array.type != oijson_type_array) {
        return 0;
    }

    unsigned int size = array.size;
    const char* itr = oijson_internal_consume_whitespace(array.buffer, &size);
    itr = oijson_internal_consume_char(itr, &size);// skip '['

    unsigned int count = 0;

    while (1) {
        itr = oijson_internal_consume_value(itr, &size);
        if (!itr) {
            break;
        }

        count++;

        itr = oijson_internal_consume_whitespace(itr, &size);
        if (!itr || *itr != ',') {
            break;
        }
        itr = oijson_internal_consume_char(itr, &size);// skip ','
    }
    return count;
}

oijson oijson_array_value_by_index(oijson array, unsigned int index) {
    if (array.type != oijson_type_array) {
        return OIJSON_INVALID;
    }

    unsigned int size = array.size;
    const char* itr = oijson_internal_consume_whitespace(array.buffer, &size);
    itr = oijson_internal_consume_char(itr, &size);// skip '['

    while (1) {
        if (!index) {
            return oijson_parse(itr, size);
        }
        index--;

        itr = oijson_internal_consume_value(itr, &size);
        if (!itr) {
            break;
        }

        itr = oijson_internal_consume_whitespace(itr, &size);
        if (!itr || *itr != ',') {
            break;
        }
        itr = oijson_internal_consume_char(itr, &size);// skip ','
    }
    return OIJSON_INVALID;
}


static int oijson_internal_push_char(char** buffer_ptr, unsigned int* buffer_size_ptr, char c) {
    if (*buffer_size_ptr) {
        **buffer_ptr = c;
        (*buffer_ptr)++;
        (*buffer_size_ptr)--;
        return 1;
    }
    return 0;
}

static int oijson_internal_utf16_to_codepoint(char utf16[4], unsigned long* out) {
    *out = 0;
    int shift = 0;
    for (int i = 1; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            (*out) |= ((utf16[i * 2 + 1] >> j) & 1) << shift;
            shift++;
        }
        for (int j = 0; j < 2; j++) {
            (*out) |= ((utf16[i * 2] >> j) & 1) << shift;
            shift++;
        }
    }
    if (utf16[0] || utf16[1]) {// TODO: correctly validade utf16 surrogate pair
        (*out) += 0x10000;
    }
    return 1;
}

static int oijson_internal_unicode_to_utf8(char** buffer_ptr, unsigned int* buffer_size_ptr, unsigned long codepoint) {
    int required_bits = 0;
    for(int i = 0; i < (int)(sizeof(unsigned long) * 8); i++) {
        if((codepoint >> i) & 1) {
            required_bits = i + 1;
        }
    }

    int required_bytes = required_bits <= 7 ? 1 : (required_bits <= 11 ? 2 : (required_bits <= 16 ? 3 : 4));
    if (*buffer_size_ptr < (unsigned int)required_bytes) {
        return 0;
    }

    unsigned int shift = 0;
    for(int i = required_bytes - 1; i >= 0; i--) {
        char* byte_ptr = *buffer_ptr + i;
        *byte_ptr = 0;
        if(required_bytes > 1) {// set initial string bits
            if (i == 0) {
                for(unsigned int b = 0; b < (unsigned int)required_bytes; b++) {
                    *byte_ptr |= 1 << (7 - b);
                }
            }
            else {// set continuation bits
                *byte_ptr |= 1 << 7;
            }
        }

        int num_bits = required_bytes == 1 ? 7 : (i == 0 ? 7 - required_bytes : 6);
        for(int b = 0; b < num_bits; b++) {
            *byte_ptr |= (((codepoint >> shift) & 1) << b);
            shift++;
        }
    }
    (*buffer_ptr) += required_bytes;
    *buffer_size_ptr -= required_bytes;
    return 1;
}

static char oijson_internal_hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return (c - 'A') + 10;
    }
    return (c - 'a') + 10;
}

static int escaped_unicode_to_bytes(const char** itr_ptr, unsigned int* size_ptr, char* bytes, int offset) {
    for (int i = offset * 2; i < (offset * 2) + 4; i++) {
        if (i % 2 == 0) {
            bytes[i / 2] = 0;
        }

        if (!(*size_ptr) || !oijson_internal_is_hex_digit(**itr_ptr)) {
            return 0;
        }
        char v = oijson_internal_hex_value(**itr_ptr);
        if ((i % 2) == 0) {
            v = v << 4;
        }
        bytes[i / 2] |= v;
        (*size_ptr)--;
        (*itr_ptr)++;
    }
    return 1;
}

static const char* oijson_internal_parse_char(const char* string, unsigned int* string_size_ptr, char** out_ptr, unsigned int* out_size_ptr) {
    if (!string || !(*string_size_ptr)) {
        return OIJSON_NULLCHAR;
    }

    const char escape_table[][2] = {
        { '\"', '\"' },
        { '\\', '\\' },
        { '/', '/' },
        { 'b', '\b' },
        { 'f', '\f' },
        { 'n', '\n' },
        { 'r', '\r' },
        { 't', '\t' },
    };
    int escape_table_len = sizeof(escape_table) / sizeof(escape_table[0]);

    switch (*string) {
        case '\\':// escaped character
            string++;
            (*string_size_ptr)--;
            if (!(*string_size_ptr)) {
                return OIJSON_NULLCHAR;
            }
            switch (*string) {
                case 'u':
                {
                    string++;
                    (*string_size_ptr)--;
                    char bytes[4] = { 0, 0, 0, 0};
                    if (!escaped_unicode_to_bytes(&string, string_size_ptr, bytes, 2)) {
                        return OIJSON_NULLCHAR;
                    }
                    if (*string_size_ptr >= 2) {// check sequential escaped 'u'
                        if (string[0] == '\\' && string[1] == 'u') {
                            string += 2;
                            (string_size_ptr) -= 2;
                            bytes[0] = bytes[2];
                            bytes[1] = bytes[3];
                            if (!escaped_unicode_to_bytes(&string, string_size_ptr, bytes, 2)) {
                                return OIJSON_NULLCHAR;
                            }
                        }
                    }
                    unsigned long codepoint;
                    if (!oijson_internal_utf16_to_codepoint(bytes, &codepoint) || !oijson_internal_unicode_to_utf8(out_ptr, out_size_ptr, codepoint)) {
                        return OIJSON_NULLCHAR;
                    }
                    break;
                }
                default:
                    for (int i = 0; i < escape_table_len; i++) {
                        if (*string == escape_table[i][0]) {
                            if (!oijson_internal_push_char(out_ptr, out_size_ptr, escape_table[i][1])) {
                                return OIJSON_NULLCHAR;
                            }
                            break;
                        }
                    }
            }
            break;
        case '\0':
            return OIJSON_NULLCHAR;
        case '\"':
            break;
        default:
            if (!oijson_internal_push_char(out_ptr, out_size_ptr, *string)) {
                return OIJSON_NULLCHAR;
            }
            break;
    }
    string++;
    (*string_size_ptr)--;
    return string;
}

int oijson_value_as_string(oijson value, char* out, unsigned int out_size) {
    if (value.type != oijson_type_string) {
        return 0;
    }

    const char* itr = value.buffer + 1;
    unsigned int size = value.size - 1;
    do {
        itr = oijson_internal_parse_char(itr, &size, &out, &out_size);
    } while (itr);
    if (out_size) {
        *out = '\0';
        return 1;
    }
    return 0;
}

static const char* oijson_internal_parse_ull(const char* string, unsigned int string_size, unsigned long long* out) {
    string = oijson_internal_consume_whitespace(string, &string_size);
    if(!string || !string_size) {
        return OIJSON_NULLCHAR;
    }

    const char* ptr = string;
    const char* start = ptr;//due to previous checks, start is guaranteed to be a number here
    do {
        ptr++;
        string_size--;
    } while (string_size && ptr[0] >= '0' && ptr[0] <= '9');
    const char* end = ptr;

    if(out) {
        unsigned long long mul = 1;
        *out = 0;
        while(ptr != start) {
            ptr--;
            *out += mul * (unsigned long long)(*ptr - '0');
            mul *= 10;
        }
    }

    return end;
}

static const char* oijson_internal_parse_ll(const char* string, unsigned int string_size, long long* out) {
    string = oijson_internal_consume_whitespace(string, &string_size);
    if (!string || !string_size) {
        return OIJSON_NULLCHAR;
    }

    int negative = *string == '-';
    if (negative) {
        string++;
        string_size--;
    }
    unsigned long long ull;
    const char* ptr = oijson_internal_parse_ull(string, string_size, &ull);
    if(out) {
        *out = (long long)ull;
        if (negative) {
            *out = -(*out);
        }
    }
    return ptr;
}

int oijson_value_as_double(oijson value, double* out) {
    if (value.type != oijson_type_number) {
        return 0;
    }

    long long integer = 0;
    unsigned long long fraction = 0;
    unsigned long long fraction_digits = 0;
    long long exponent = 0;

    unsigned int size = value.size;
    const char* integer_ptr = OIJSON_NULLCHAR;
    unsigned int integer_size;
    const char* fraction_ptr = OIJSON_NULLCHAR;
    unsigned int fraction_size;
    const char* exponent_ptr = OIJSON_NULLCHAR;
    unsigned int exponent_size;
    const char* ptr = oijson_internal_consume_number_info(value.buffer, &size, &integer_ptr, &integer_size, &fraction_ptr, &fraction_size, &exponent_ptr, &exponent_size);
    if(!ptr) {
        return 0;
    }

    if (out) {
        if (!oijson_internal_parse_ll(integer_ptr, integer_size, &integer)) {
            return 0;
        }
        *out = (double)integer;
        if (fraction_ptr) {
            if (!oijson_internal_parse_ull(fraction_ptr, fraction_size, &fraction)) {
                return 0;
            }
            const char* ptr2 = fraction_ptr;
            while (fraction_size && *ptr2 >= '0' && *ptr2 <= '9') {
                fraction_digits++;
                ptr2++;
            }

            double fraction_d = integer_ptr[0] == '-' ? -(double)fraction : (double)fraction;
            while (fraction_digits) {
                fraction_d /= 10.0;
                fraction_digits--;
            }
            *out += fraction_d;
        }
        if (exponent_ptr) {
            if (!oijson_internal_parse_ll(exponent_ptr, exponent_size, &exponent)) {
                return 0;
            }

            while (exponent > 0) {
                *out *= 10.0;
                exponent--;
            }
            while (exponent < 0) {
                *out /= 10.0;
                exponent++;
            }
        }
    }

    return 1;
}

int oijson_value_as_float(oijson value, float* out) {
    double d;
    if (oijson_value_as_double(value, &d)) {
        if (out) {
            *out = (float)d;
        }
        return 1;
    }
    return 0;
}

int oijson_value_as_long(oijson value, long* out) {
    if (value.type != oijson_type_number) {
        return 0;
    }

    long long integer = 0;
    unsigned long long fraction = 0;
    unsigned long long fraction_digits = 0;
    long long exponent = 0;

    unsigned int size = value.size;
    const char* integer_ptr = OIJSON_NULLCHAR;
    unsigned int integer_size;
    const char* fraction_ptr = OIJSON_NULLCHAR;
    unsigned int fraction_size;
    const char* exponent_ptr = OIJSON_NULLCHAR;
    unsigned int exponent_size;
    const char* ptr = oijson_internal_consume_number_info(value.buffer, &size, &integer_ptr, &integer_size, &fraction_ptr, &fraction_size, &exponent_ptr, &exponent_size);
    if(!ptr) {
        return 0;
    }

    if (out) {
        if (!oijson_internal_parse_ll(integer_ptr, integer_size, &integer)) {
            return 0;
        }
        *out = (long)integer;
        if (fraction_ptr) {
            if (!oijson_internal_parse_ull(fraction_ptr, fraction_size, &fraction)) {
                return 0;
            }
            const char* ptr2 = fraction_ptr;
            while (fraction_size && *ptr2 >= '0' && *ptr2 <= '9') {
                fraction_digits++;
                ptr2++;
            }

            double fraction_d = integer_ptr[0] == '-' ? -(double)fraction : (double)fraction;
            while (fraction_digits) {
                fraction_d /= 10.0;
                fraction_digits--;
            }
            if (fraction_d < -0.5) {
                (*out)--;
            }
            else if (fraction_d > 0.5) {
                (*out)++;
            }
        }
        if (exponent_ptr) {
            if (!oijson_internal_parse_ll(exponent_ptr, exponent_size, &exponent)) {
                return 0;
            }

            while (exponent > 0) {
                *out *= 10;
                exponent--;
            }
            while (exponent < 0 && *out) {
                *out /= 10;
                exponent++;
            }
        }
    }

    return 1;
}

int oijson_value_as_int(oijson value, int* out) {
    long l;
    if (oijson_value_as_long(value, &l)) {
        if (out) {
            *out = (int)l;
        }
        return 1;
    }
    return 0;
}
