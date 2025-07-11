# OIJSON Documentation

OIJson is supposed to be simple, with as little types and functions as needed. Therefore, it should be easy to learn to use the library by, well, *using the library*. That said, this single page documentation attempts to be a clarifying resource, when the code itself is not intuitive enough.

# Enums

### oijson_type

The type of a JSON value. *oijson_type_invalid* does not direcly map to a JSON type and is used when a JSON string fails to be parsed due to an error. In such cases, [oijson_error](#oijson_error) can be called for an error message.

|Value               |Json type |
|:-------------------|:-------- |
|oijson_type_invalid |None      |
|oijson_type_string  |string    |
|oijson_type_number  |number    |
|oijson_type_object  |object    |
|oijson_type_array   |array     |
|oijson_type_true    |true      |
|oijson_type_false   |false     |
|oijson_type_null    |null      |

<br>
<br>

# Structs

### oijson

Represents a JSON object, containg a reference to the raw JSON string as well as its length. To get usable data types from an oijson, use the [value functions](#Functions)

|Field  |Type          |Description        |
|:------|:-------------|:------------------|
|buffer | const char*  | Read-only. Pointer to the start of the object in the buffer provided in [oijson_parse](#oijson_parse). External modifications to this buffer may invalidate the object. |
|size   | unsigned int | Read-only. The size, in bytes, of the JSON string representing the object, including whitespace. |
|type   | type         | Read-only. The oijson_type of the JSON object. |

<br>
<br>

# Functions

- General
    - [oijson_error](#oijson_error)
    - [oijson_parse](#oijson_parse)
- Object
    - [oijson_object_count](#oijson_object_count)
    - [oijson_object_value_by_name](#oijson_object_value_by_name)
    - [oijson_object_name_by_index](#oijson_object_name_by_index)
    - [oijson_object_value_by_index](#oijson_object_value_by_index)
- Array
    - [oijson_array_count](#oijson_array_count)
    - [oijson_array_value_by_index](#oijson_array_value_by_index)
- Values
    - [oijson_value_as_string](#oijson_value_as_string)
    - [oijson_value_as_long](#oijson_value_as_long)
    - [oijson_value_as_int](#oijson_value_as_int)
    - [oijson_value_as_double](#oijson_value_as_double)
    - [oijson_value_as_float](#oijson_value_as_float)

<br>

### oijson_error
```C
const char* oijson_error(void)
```

Returns the pointer to a stack allocated error string. The string is guaranteed to be null terminated and is updated with any function call, becoming empty when a function succeeds.

<br>

### oijson_parse
```C
oijson oijson_parse(const char* JSON, unsigned int JSON_size)
```

Returns an [oijson](#oijson) struct. The [type](#oijson_type) field of the returned struct indicates if the operation was successful, with a value of *oijson_type_invalid* indicating failure. Use [oijson_error](#oijson_error) for details.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|json      |[oijson](#oijson) | Buffer containing null-terminated JSON string. Any manual modification to this buffer after calling [oijson_parse](oijson_parse) may invalidate the object. |
|json_size |unsigned int | Size of buffer. |

<br>

### oijson_object_count
```C
unsigned int oijson_object_count(oijson object)
```
Returns the amount of name/value pairs in **object**. The [type](#oijson_type) of **object** must be *oijson_type_object*, otherwise the function will simply return 0.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|object    |[oijson](#oijson) |The JSON object. This must be of [type](#oijson_type) *oijson_type_object*. |

<br>

### oijson_object_value_by_name
```C
oijson oijson_object_value_by_name(oijson object, const char* name)
```
Returns the value of the name/value pair called **name** as an [oijson](#oijson). If **object** is not of [type](#oijson_type) *oijson_type_object* or does not contain a name/value pair called **name**, returns an empty [oijson](#oijson) of [type](#oijson_type) *oijson_type_invalid*.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|object    |[oijson](#oijson) | The JSON object. This must be of [type](#oijson_type) *oijson_type_object*. |
|name      |const char*| The name of the value to query. |

<br>

### oijson_object_name_by_index
```C
oijson oijson_object_name_by_index(oijson object, unsigned int index)
```
Returns the name of the name/value pair at **index** as an [oijson](#oijson) of type *oijson_type_string*. If **object** is not of [type](#oijson_type) *oijson_type_object* or if **index** is out of bounds, returns an empty [oijson](#oijson) of [type](#oijson_type) *oijson_type_invalid*.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|object    |[oijson](#oijson) | The JSON object. This must be of [type](#oijson_type) *oijson_type_object*. |
|index     |unsigned int | The index of the name to query. |

<br>

### oijson_object_value_by_index
```C
oijson oijson_object_value_by_index(oijson object, unsigned int index)
```
Returns the value of the name/value pair at **index** as an [oijson](#oijson). If **object** is not of [type](#oijson_type) *oijson_type_object* or if **index** is out of bounds, returns an empty [oijson](#oijson) o [type](#oijson_type) *oijson_type_invalid*.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|object    |[oijson](#oijson) | The JSON object. This must be of [type](#oijson_type) *oijson_type_object*. |
|index     |unsigned int | The index of the value to query. |

<br>

### oijson_array_count
```C
unsigned int oijson_array_count(oijson array)
```

Returns the amount of values in **array**. The [type](#oijson_type) of **array** must be *oijson_type_array*, otherwise the function will simply return 0.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|array     |[oijson](#oijson) | The JSON array. This must be of [type](#oijson_type) *oijson_type_array*. |

<br>

### oijson_array_value_by_index
```C
oijson oijson_array_value_by_index(oijson array, unsigned int index)
```

Returns the value at **index** as an [oijson](#oijson). If **array** is not of [type](#oijson_type) *oijson_type_array* or if **index** is out of bounds, returns an empty [oijson](#oijson) o [type](#oijson_type) *oijson_type_invalid*.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|object    |[oijson](#oijson) | The JSON array. This must be of [type](#oijson_type) *oijson_type_array*. |
|index     |unsigned int | The index of the value to query. |

<br>

### oijson_value_as_string
```C
int oijson_value_as_string(oijson value, char* out, unsigned int out_size)
```

Gets the **value** as a string and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_string* or if the string cannot fit into the buffer of size **out_size**. Upon success, out will contain a null-terminated string.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|value     |[oijson](#oijson) | The string value. This must be of [type](#oijson_type) *oijson_type_string*. |
|out       |const char* | The buffer to which the value will be written. |
|out_size  |unsigned int |The size of **out**. |

<br>

### oijson_value_as_long
```C
int oijson_value_as_long(oijson value, long* out)
```

Gets the **value** as a long and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_number*.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|value     |[oijson](#oijson) | The number value. This must be of [type](#oijson_type) *oijson_type_number*. |
|out       |long* | Pointer to be filled in with the number value. |

<br>

### oijson_value_as_int
```C
int oijson_value_as_int(oijson value, int* out)
```

Gets the **value** as an int and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_number*.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|value     |[oijson](#oijson) | The number value. This must be of [type](#oijson_type) *oijson_type_number*. |
|out       |int* | Pointer to be filled in with the number value. |

<br>

### oijson_value_as_double
```C
int oijson_value_as_double(oijson value, double* out)
```

Gets the **value** as a double and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_number*.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|value     |[oijson](#oijson) | The number value. This must be of [type](#oijson_type) *oijson_type_number*. |
|out       |double* | Pointer to be filled in with the number value. |

<br>

### oijson_value_as_float
```C
int oijson_value_as_float(oijson value, float* out)
```

Gets the **value** as a float and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_number*.

|Parameter |Type|Description |
|:---------|:---|:-----------|
|value     |[oijson](#oijson) | The number value. This must be of [type](#oijson_type) *oijson_type_number*. |
|out       |float* | Pointer to be filled in with the number value. |
