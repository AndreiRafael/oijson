# OIJSON Documentation

OIJSON is supposed to be simple, with as little types and functions as needed. This single page documentation attempts to be a clarifying resource, when the code itself is not intuitive enough.

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

### oijson_iterator_type

The type of an iterator. *oijson_iterator_type_invalid* indicates an invalid iterator. An iterator will be invalid if the [oijson](#oijson) value it is iterating is not an object nor an array. An interator will also become invalid once it reaches the end of an object or array, or if the object or array is empty.

|Value                        |Description |
|:----------------------------|:--------   |
|oijson_iterator_type_invalid | Type of an iterator that was created incorrectly, has reached the end of an object or array, or is iterating over an empty object or array |
|oijson_iterator_type_object  | Type of an iterator that was created with an object. Fields name and value may be accessed. |
|oijson_iterator_type_array   | Type of an iterator that was created with an array. The field value may be accessed, while the field name will always be of [type](#oijson_type) *oijson_type_invalid*. |

<br>
<br>

# Structs

### oijson

Represents a JSON object, containg a reference to the raw JSON string as well as its length. To get usable data types from an oijson, use the [value functions](#Functions). The contents of **buffer** should NOT be modified externally, as doing so may invalidate the JSON object and lead to undesired behaviour.

|Field  |Type          |Description        |
|:------|:-------------|:------------------|
|buffer | const char*  | Read-only. Pointer to the start of the object in the buffer provided in [oijson_parse](#oijson_parse). External modifications to this buffer may invalidate the object. |
|size   | unsigned int | Read-only. The size, in bytes, of the JSON string representing the object, including whitespace. |
|type   | type         | Read-only. The oijson_type of the JSON object. |

<br>

### oijson_iterator

Represents a JSON object, containg a reference to the raw JSON string as well as its length. To get usable data types from an oijson, use the [value functions](#Functions). The contents of **buffer** should NOT be modified externally, as doing so may invalidate the JSON object and lead to undesired behaviour.

|Field  |Type                  |Description        |
|:------|:---------------------|:------------------|
|type   | oijson_iterator_type | Read-only. Type of the iterator. If other than *oijson_iterator_type*, the fields name and value may be acessed. |
|name   | oijson               | Read-only. The name field for iterators of type *oijson_iterator_type_object*. |
|value  | oijson               | Read-only. The name field for iterators of type *oijson_iterator_type_object* or *oijson_iterator_type_array*. |
|ptr    | const char*          | Read-only. Pointer to object or array data. Used internally. |
|size   | unsigned int         | Read-only. Size of object or array data. Used internally. |

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
    - [oijson_value_formatted](#oijson_value_formatted)
    - [oijson_value_as_string](#oijson_value_as_string)
    - [oijson_value_as_long](#oijson_value_as_long)
    - [oijson_value_as_int](#oijson_value_as_int)
    - [oijson_value_as_double](#oijson_value_as_double)
    - [oijson_value_as_float](#oijson_value_as_float)
- Iterators
    - [oijson_iterator_create](#oijson_iterator_create)
    - [oijson_iterator_advance](#oijson_iterator_advance)

<br>

### oijson_error
```C
const char* oijson_error(void)
```

Returns the pointer to a stack allocated error string. The string is guaranteed to be null terminated and is updated whenever an error occurs. The value is not updated when a function succeeds.

<br>

### oijson_parse
```C
oijson oijson_parse(const char* json, unsigned int json_size)
```

Returns an [oijson](#oijson) struct. The [type](#oijson_type) field of the returned struct indicates if the operation was successful, with a value of *oijson_type_invalid* indicating failure. Use [oijson_error](#oijson_error) for details.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|json      |[oijson](#oijson) | Buffer containing a JSON string. If the string is null terminated, parsing will stop at at the null terminator. Otherwise, **json_size** indicates the size of the string in bytes. Any manual modification to this buffer after calling [oijson_parse](oijson_parse) may invalidate the object. |
|json_size |unsigned int | Size of buffer in bytes. |

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

|Parameter |Type |Description |
|:---------|:----|:-----------|
|object    |[oijson](#oijson) | The JSON object. This must be of [type](#oijson_type) *oijson_type_object*. |
|name      |const char*| The name of the value to query. |

<br>

### oijson_object_name_by_index
```C
oijson oijson_object_name_by_index(oijson object, unsigned int index)
```
Returns the name of the name/value pair at **index** as an [oijson](#oijson) of type *oijson_type_string*. If **object** is not of [type](#oijson_type) *oijson_type_object* or if **index** is out of bounds, returns an empty [oijson](#oijson) of [type](#oijson_type) *oijson_type_invalid*.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|object    |[oijson](#oijson) | The JSON object. This must be of [type](#oijson_type) *oijson_type_object*. |
|index     |unsigned int | The index of the name to query. |

<br>

### oijson_object_value_by_index
```C
oijson oijson_object_value_by_index(oijson object, unsigned int index)
```
Returns the value of the name/value pair at **index** as an [oijson](#oijson). If **object** is not of [type](#oijson_type) *oijson_type_object* or if **index** is out of bounds, returns an empty [oijson](#oijson) of [type](#oijson_type) *oijson_type_invalid*.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|object    |[oijson](#oijson) | The JSON object. This must be of [type](#oijson_type) *oijson_type_object*. |
|index     |unsigned int | The index of the value to query. |

<br>

### oijson_array_count
```C
unsigned int oijson_array_count(oijson array)
```

Returns the amount of values in **array**. The [type](#oijson_type) of **array** must be *oijson_type_array*, otherwise the function will simply return 0.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|array     |[oijson](#oijson) | The JSON array. This must be of [type](#oijson_type) *oijson_type_array*. |

<br>

### oijson_array_value_by_index
```C
oijson oijson_array_value_by_index(oijson array, unsigned int index)
```

Returns the value at **index** as an [oijson](#oijson). If **array** is not of [type](#oijson_type) *oijson_type_array* or if **index** is out of bounds, returns an empty [oijson](#oijson) o [type](#oijson_type) *oijson_type_invalid*.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|object    |[oijson](#oijson) | The JSON array. This must be of [type](#oijson_type) *oijson_type_array*. |
|index     |unsigned int | The index of the value to query. |

<br>

### oijson_value_formatted
```C
int oijson_value_formatted(oijson value, char* out, unsigned int out_size)
```
Gets the **value** as a formatted UTF-8 string and copies it into **out**, removing whitespace. Returns 1 on success, or 0 if the string cannot fit into the buffer of size **out_size**. Upon success, out will contain a null terminated string. If the function fails, the string will be truncated.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|value     |[oijson](#oijson) | The JSON value. |
|out       |const char* | The buffer to which the value will be written. |
|out_size  |unsigned int |The size of **out** in bytes. |

<br>

### oijson_value_as_string
```C
int oijson_value_as_string(oijson value, char* out, unsigned int out_size)
```

Gets the **value** as a string and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_string* or if the string cannot fit into the buffer of size **out_size**. Upon success, out will contain a null terminated string. If the function fails, the string will be truncated.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|value     |[oijson](#oijson) | The string value. This must be of [type](#oijson_type) *oijson_type_string*. |
|out       |const char* | The buffer to which the value will be written. |
|out_size  |unsigned int |The size of **out** in bytes. |

<br>

### oijson_value_as_long
```C
int oijson_value_as_long(oijson value, long* out)
```

Gets the **value** as a long and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_number*.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|value     |[oijson](#oijson) | The number value. This must be of [type](#oijson_type) *oijson_type_number*. |
|out       |long* | Pointer to be filled in with the number value. |

<br>

### oijson_value_as_int
```C
int oijson_value_as_int(oijson value, int* out)
```

Gets the **value** as an int and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_number*.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|value     |[oijson](#oijson) | The number value. This must be of [type](#oijson_type) *oijson_type_number*. |
|out       |int* | Pointer to be filled in with the number value. |

<br>

### oijson_value_as_double
```C
int oijson_value_as_double(oijson value, double* out)
```

Gets the **value** as a double and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_number*.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|value     |[oijson](#oijson) | The number value. This must be of [type](#oijson_type) *oijson_type_number*. |
|out       |double* | Pointer to be filled in with the number value. |

<br>

### oijson_value_as_float
```C
int oijson_value_as_float(oijson value, float* out)
```

Gets the **value** as a float and copies it into **out**. Returns 1 on success, or 0 if **value** is not of [type](#oijson_type) *oijson_type_number*.

|Parameter |Type |Description |
|:---------|:----|:-----------|
|value     |[oijson](#oijson) | The number value. This must be of [type](#oijson_type) *oijson_type_number*. |
|out       |float* | Pointer to be filled in with the number value. |

<br>

### oijson_iterator_create
```C
oijson_iterator oijson_iterator_create(oijson value);
```

Creates an iterator for an object or array. Returns a valid *oijson_iterator* if **value** is of type *oijson_type_object* or *oijson_type_array* containing at least one item. Otherwise, returns an invalid *oijson_iterator*. The [type](#oijson_iterator_type) field should be checked to determine iterator validity.

|Parameter |Type              |Description |
|:---------|:-----------------|:-----------|
|value     |[oijson](#oijson) | The object or array to iterate over. This must be of [type](#oijson_type) *oijson_type_object* or *oijson_type_array*. |

<br>

### oijson_iterator_advance
```C
void oijson_iterator_advance(oijson_iterator* iterator);
```

Advances the iterator to the next position. This will invalidate the iterator if it goes over the amount of values of the object or array being iterated over. If the iterator is already invalid, does nothing.

|Parameter |Type                                 |Description |
|:---------|:------------------------------------|:-----------|
|value     |[oijson_iterator](#oijson_iterator)* | Pointer to the iterator to be advanced. |
