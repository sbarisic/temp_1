#include <core2.h>

#define CHUNK_SIZE 512

// char *buffer = NULL;
// size_t buffer_len = 0;
// size_t buffer_content_len = 0;
// bool need_comma;

void core2_json_concat(core2_json_t *json, const char *str)
{
    size_t str_len = strlen(str);

    if (str_len + json->buffer_content_len >= json->buffer_len)
    {
        // Make some space or somethin'
        json->buffer_len = str_len + json->buffer_content_len + CHUNK_SIZE;
        json->buffer = (char *)core2_realloc(json->buffer, json->buffer_len);
    }

    strncat(json->buffer, str, str_len);
    json->buffer_content_len = strlen(json->buffer);
}

core2_json_t *core2_json_create()
{
    core2_json_t *json = (core2_json_t *)malloc(sizeof(core2_json_t));

    json->buffer = (char *)core2_malloc(CHUNK_SIZE);
    json->buffer_len = CHUNK_SIZE;
    json->buffer_content_len = 0;
    json->need_comma = false;

    core2_json_concat(json, "{ ");

    return json;
}

void core2_json_delete(core2_json_t *json)
{
    if (json->buffer != NULL)
    {
        core2_free(json->buffer);
        json->buffer = NULL;
        json->buffer_len = 0;
    }

    core2_free(json);
}

char *core2_json_escape_string(char *str)
{
    // TODO
    return str;
}

const char *get_float_format(core2_json_fieldtype_t data_type)
{
    const char *fmt = "%f";

    switch (data_type)
    {
    case CORE2_JSON_FLOAT_ARRAY_DEC2:
    case CORE2_JSON_FLOAT_DEC2:
        fmt = "%.2f";
        break;

    case CORE2_JSON_FLOAT_ARRAY:
    case CORE2_JSON_FLOAT:
        fmt = "%f";
        break;

    default:
        dprintf("get_float_format(%d) NOT IMPLEMENTED", data_type);
        break;
    }

    return fmt;
}

void core2_json_add(core2_json_t *json, core2_json_fieldtype_t data_type)
{
    switch (data_type)
    {
    case CORE2_JSON_BEGIN_ARRAY:
    case CORE2_JSON_BEGIN_OBJECT:
        if (json->need_comma)
        {
            core2_json_concat(json, ", ");
            json->need_comma = false;
        }
        break;
    }

    switch (data_type)
    {
    case CORE2_JSON_BEGIN_ARRAY:
        core2_json_concat(json, "[ ");
        json->need_comma = false;
        break;

    case CORE2_JSON_END_ARRAY:
        core2_json_concat(json, " ]");
        json->need_comma = true;
        break;

    case CORE2_JSON_BEGIN_OBJECT:
        core2_json_concat(json, "{ ");
        json->need_comma = false;
        break;

    case CORE2_JSON_END_OBJECT:
        core2_json_concat(json, " }");
        json->need_comma = true;
        break;

    default:
        eprintf("core2_json_add unknown data_type %d\n", (int)data_type);
        break;
    }
}

void core2_json_begin_field(core2_json_t *json, const char *field_name, core2_json_fieldtype_t data_type)
{
    if (json->need_comma)
    {
        core2_json_concat(json, ", \"");
        json->need_comma = false;
    }
    else
    {
        core2_json_concat(json, "\"");
    }

    core2_json_concat(json, field_name);
    core2_json_concat(json, "\": ");
    core2_json_add(json, data_type);
}

void core2_json_add_field(core2_json_t *json, const char *field_name, const void *data, size_t len,
                          core2_json_fieldtype_t data_type)
{
    char temp_buffer[64];

    if (json->need_comma)
    {
        core2_json_concat(json, ", \"");
        json->need_comma = false;
    }
    else
    {
        core2_json_concat(json, "\"");
    }

    core2_json_concat(json, field_name);
    core2_json_concat(json, "\": ");

    switch (data_type)
    {
    case CORE2_JSON_STRING:
        if (data == NULL)
        {
            core2_json_concat(json, "null");
        }
        else
        {
            sprintf(temp_buffer, "\"%s\"", core2_json_escape_string((char *)data));
            core2_json_concat(json, temp_buffer);
        }

        json->need_comma = true;
        break;

    case CORE2_JSON_FLOAT_DEC2:
    case CORE2_JSON_FLOAT:
        sprintf(temp_buffer, get_float_format(data_type), *(float *)data);

        core2_json_concat(json, temp_buffer);
        json->need_comma = true;
        break;

    case CORE2_JSON_INT:
        // dtostrf(*(float *)data, 0, 2, temp_buffer);
        sprintf(temp_buffer, "%d", *(int *)data);

        core2_json_concat(json, temp_buffer);
        json->need_comma = true;
        break;

    case CORE2_JSON_FLOAT_ARRAY_DEC2:
    case CORE2_JSON_FLOAT_ARRAY:
        core2_json_concat(json, "[ ");

        for (size_t i = 0; i < len; i++)
        {
            // dtostrf(((float *)data)[i], 0, 2, temp_buffer);
            sprintf(temp_buffer, get_float_format(data_type), ((float *)data)[i]);

            core2_json_concat(json, temp_buffer);

            if (i < len - 1)
                core2_json_concat(json, ", ");
        }

        core2_json_concat(json, " ]");
        json->need_comma = true;
        break;

    default:
        eprintf("core2_json_add_field unknown data_type %d\n", (int)data_type);
        break;
    }
}

void core2_json_add_field_string(core2_json_t *json, const char *field_name, const char *str)
{
    core2_json_add_field(json, field_name, str, 0, CORE2_JSON_STRING);
}

void core2_json_add_field_int(core2_json_t *json, const char *field_name, int num)
{
    core2_json_add_field(json, field_name, &num, 0, CORE2_JSON_INT);
}

void core2_json_serialize(core2_json_t *json, char **dest_buffer, size_t *json_length)
{
    core2_json_concat(json, " }");

    if (dest_buffer != NULL && json_length != NULL)
    {
        *dest_buffer = (char *)core2_malloc(json->buffer_content_len + 1);
        *json_length = json->buffer_content_len;
        memcpy(*dest_buffer, json->buffer, json->buffer_content_len);

        core2_free(json->buffer);
        json->buffer = NULL;
        json->buffer_len = 0;
        json->buffer_content_len = 0;
    }
}

void core2_json_test()
{
    char *json_buffer;
    size_t json_len;
    core2_json_t *json = core2_json_create();

    const char *field1 = "Ayy lmao";
    core2_json_add_field_string(json, "field1", field1);

    const char *field1_5 = "Test field 1.5";
    core2_json_add_field_string(json, "field1_5", field1_5);

    float field2 = 42.69f;
    core2_json_add_field(json, "field2", &field2, 0, CORE2_JSON_FLOAT);

    float field3[] = {1.23f, 2.34f, 3, 4, 5};
    core2_json_add_field(json, "field3", &field3, sizeof(field3) / sizeof(*field3), CORE2_JSON_FLOAT_ARRAY);

    core2_json_serialize(json, &json_buffer, &json_len);

    printf("======= core2_json_test =======\n");
    printf("%s\n", json_buffer);
    printf("===============================\n");

    core2_free(json_buffer);
    core2_json_delete(json);
}