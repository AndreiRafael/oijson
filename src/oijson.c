#include "oijson.h"

#define OIJSON_NULLCHAR ((const char*)0)

static char oijson_internal_error[128] = "";

const char* oijson_error(void) {
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

static const char* oijson_internal_consume_whitespace(const char* itr) {
    if (!itr) {
        return OIJSON_NULLCHAR;
    }

    while (*itr && oijson_internal_is_whitespace(*itr)) {
        itr++;
    }
    return *itr ? itr : OIJSON_NULLCHAR;
}

static const char* oijson_internal_consume_keyword(const char* itr, const char* keyword) {
    itr = oijson_internal_consume_whitespace(itr);
    if (!itr) {
        return OIJSON_NULLCHAR;
    }

    while(*itr && *keyword && *itr == *keyword) {
        itr++;
        keyword++;
    }
    return *keyword ? OIJSON_NULLCHAR : itr;
}

static const char* oijson_internal_consume_tfn(const char* itr) {
    const char* itr2 = oijson_internal_consume_keyword(itr, "true");
    if (itr2) {
        return itr2;
    }
    itr2 = oijson_internal_consume_keyword(itr, "false");
    if (itr2) {
        return itr2;
    }

    itr2 = oijson_internal_consume_keyword(itr, "null");
    if (itr2) {
        return itr2;
    }

    return OIJSON_NULLCHAR;
}

static const char* oijson_internal_consume_string(const char* itr) {
    itr = oijson_internal_consume_whitespace(itr);
    if (!itr) {
        return OIJSON_NULLCHAR;
    }

    if (*itr != '\"') {
        return OIJSON_NULLCHAR;
    }
    itr++;

    while (*itr != '\"') {
        switch (*itr) {
            case '\\':
                itr++;
                switch(*itr) {
                    case '\"':
                    case '\\':
                    case '/':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':// TODO: support for 4 hex digits
                        itr++;
                        break;
                    default:
                        return OIJSON_NULLCHAR;// error, invalid character
                }
                break;
            case '\0':
                return OIJSON_NULLCHAR;
            default:
                itr++;
                break;
        }
    }
    return ++itr;
}

static const char* oijson_internal_consume_number(const char* itr) {
    itr = oijson_internal_consume_whitespace(itr);
    if (!itr) {
        return OIJSON_NULLCHAR;
    }

    if (*itr == '-') {
        itr++;
    }

    if (*itr == '0') {// straight to fraction
        itr++;
    }
    else {
        if (!oijson_internal_is_digit(*itr)) {
            return OIJSON_NULLCHAR;
        }
        itr++;
        while (oijson_internal_is_digit(*itr)) {
            itr++;
        }
    }

    if (*itr == '.') {// fraction
        itr++;
        if (!oijson_internal_is_digit(*itr)) {
            return OIJSON_NULLCHAR;
        }
        itr++;
        while (oijson_internal_is_digit(*itr)) {
            itr++;
        }
    }

    if (*itr == 'e' || *itr == 'E') {
        itr++;
        if (*itr == '-' || *itr == '+') {
            itr++;
        }
        if (!oijson_internal_is_digit(*itr)) {
            return OIJSON_NULLCHAR;
        }
        itr++;
        while (oijson_internal_is_digit(*itr)) {
            itr++;
        }
    }
    return itr;
}
static const char* oijson_internal_consume_object(const char*);
static const char* oijson_internal_consume_array(const char*);

static const char* oijson_internal_consume_value(const char* itr) {
    const char* itr2 = oijson_internal_consume_string(itr);
    if (itr2) {
        goto found;
    }
    itr2 = oijson_internal_consume_number(itr);
    if (itr2) {
        goto found;
    }
    itr2 = oijson_internal_consume_object(itr);
    if (itr2) {
       goto found;
    }
    itr2 = oijson_internal_consume_array(itr);
    if (itr2) {
       goto found;
    }
    itr2 = oijson_internal_consume_tfn(itr);
    if(itr2) {
        goto found;
    }
    return OIJSON_NULLCHAR;

    found:
    return itr2;
}

static const char* oijson_internal_consume_key_value_pair(const char* itr, const char** key_start, const char** key_end, const char** value_start) {
    itr = oijson_internal_consume_whitespace(itr);
    if (!itr) {
        return OIJSON_NULLCHAR;
    }

    // read name
    if (key_start) {
        *key_start = itr + 1;
    }
    itr = oijson_internal_consume_string(itr);
    if (!itr) {
        return OIJSON_NULLCHAR;
    }
    if (key_end) {
        *key_end = itr - 1;
    }

    itr = oijson_internal_consume_whitespace(itr);
    if (!itr || *itr != ':') {
        return OIJSON_NULLCHAR;
    }

    itr++;// skip ':'
    if (value_start) {
        *value_start = itr;
    }
    itr = oijson_internal_consume_value(itr);
    return itr;
}

static const char* oijson_internal_consume_object(const char* itr) {
    itr = oijson_internal_consume_whitespace(itr);
    if (!itr || *itr != '{') {
        return OIJSON_NULLCHAR;
    }

    itr++;
    itr = oijson_internal_consume_whitespace(itr);
    while(1) {
        itr = oijson_internal_consume_key_value_pair(itr, 0, 0, 0);

        itr = oijson_internal_consume_whitespace(itr);
        if (!itr) {
            return OIJSON_NULLCHAR;
        }
        if (*itr == '}') {
            itr++;
            break;
        }
        if (*itr != ',') {
            return OIJSON_NULLCHAR;
        }
        itr++;// step over comma
    }
    return itr;
}

static const char* oijson_internal_consume_array(const char* itr) {
    itr = oijson_internal_consume_whitespace(itr);
    if (!itr || *itr != '[') {
        return OIJSON_NULLCHAR;
    }

    itr++;
    itr = oijson_internal_consume_whitespace(itr);
    while(1) {
        itr = oijson_internal_consume_value(itr);// value
        if (!itr) {
            return OIJSON_NULLCHAR;
        }

        itr = oijson_internal_consume_whitespace(itr);
        if (!itr) {
            return OIJSON_NULLCHAR;
        }
        if (*itr == ']') {
            itr++;
            break;
        }
        if (*itr != ',') {
            return OIJSON_NULLCHAR;
        }
        itr++;// step over comma
    }
    return itr;
}

const char* oijson_parse(const char* json, oijson_type* out_type) {
    const char* itr = oijson_internal_consume_string(json);
    oijson_type type;
    if (itr) {
        type = oijson_type_string;
    }
    itr = oijson_internal_consume_number(json);
    if(itr) {
        type = oijson_type_number;
        goto found;
    }
    itr = oijson_internal_consume_object(json);
    if (itr) {
        type = oijson_type_object;
        goto found;
    }
    itr = oijson_internal_consume_array(json);
    if (itr) {
        type = oijson_type_array;
        goto found;
    }
    itr = oijson_internal_consume_keyword(json, "true");
    if (itr) {
        type = oijson_type_true;
        goto found;
    }
    itr = oijson_internal_consume_keyword(json, "false");
    if (itr) {
        type = oijson_type_false;
        goto found;
    }
    itr = oijson_internal_consume_keyword(json, "null");
    if (itr) {
        type = oijson_type_null;
        goto found;
    }
    return OIJSON_NULLCHAR;

    found:
    if (out_type) {
        *out_type = type;
    }
    return itr;
}

// string equality check, b is required null terminated
static int oijson_internal_check_value(const char* start, const char* end, const char* name) {
    while(*start && *name && *start == *name) {
        start++;
        name++;
    }
    return !(*name) && start == end;
}

const char* oijson_object_value_by_name(const char* object, const char* name) {
    oijson_type type;
    if (!oijson_parse(object, &type) || type != oijson_type_object || !name) {
        return OIJSON_NULLCHAR;
    }

    const char* itr = oijson_internal_consume_whitespace(object);
    itr++;

    while (1) {
        const char* start;
        const char* end;
        const char* value;
        itr = oijson_internal_consume_key_value_pair(itr, &start, &end, &value);
        if (!itr) {
            break;
        }

        if (oijson_internal_check_value(start, end, name)) {
            return oijson_internal_consume_whitespace(value);
        }

        itr = oijson_internal_consume_whitespace(itr);
        if (*itr != ',') {
            break;
        }
        itr++;
    }
    return OIJSON_NULLCHAR;
}
