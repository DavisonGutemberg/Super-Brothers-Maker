#ifndef JSON_H
#define JSON_H


typedef enum JsonDataType{
    JSON_NULL,
    JSON_FALSE,
    JSON_TRUE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
}JsonDataType;


typedef struct JsonValue
{
    int type;
    union
    {
        double number;
        char*  string;
        struct JsonObject* object;
        struct JsonArray* array;
    }data;
}JsonValue;

typedef struct JsonObject
{
    int length;
    int capacity;
    struct
    {
        char* key;
        struct JsonValue value;
    }*buffer;
}JsonObject;

typedef struct JsonArray
{
    int length;
    int capacity;
    struct JsonValue* buffer;
}JsonArray;


int json_validate(char* buffer);
int json_serialize(char* buffer, JsonValue* value);
int json_deserialize(char* buffer, JsonValue* value);

int json_object_create(JsonObject **object);
int json_object_set(JsonObject* object, char* key, JsonValue* value);
int json_object_get(JsonObject* object, char* key, JsonValue* value);
int json_object_insert(JsonObject* object, int index, char* key, JsonValue* value);
int json_object_remove(JsonObject* object, int index, char* key, JsonValue* value);
int json_object_push(JsonObject* object, char* key, JsonValue* value);
int json_object_pop(JsonObject* object, char* key, JsonValue* value);
int json_object_peek(JsonObject* object, char* key, JsonValue* value);
int json_object_destroy(JsonObject** object);

int json_array_create(JsonArray** array);
int json_array_set(JsonArray* array, int index, JsonValue* value);
int json_array_get(JsonArray* array, int index, JsonValue* value);
int json_array_insert(JsonArray* array, int index, JsonValue* value);
int json_array_remove(JsonArray* array, int index, JsonValue* value);
int json_array_push(JsonArray* array, JsonValue* value);
int json_array_pop(JsonArray* array, JsonValue* value);
int json_array_peek(JsonArray* array, JsonValue* value);
int json_array_destroy(JsonArray** array);


#endif