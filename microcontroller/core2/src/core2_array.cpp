
#include <core2.h>

core2_array_t *core2_array_create(size_t element_size)
{
    dprintf("core2_array_create(%d)\n", element_size);

    core2_array_t *array = (core2_array_t *)core2_malloc(sizeof(core2_array_t));
    array->element_size = element_size;
    array->length = 0;
    array->memory = NULL;
    return array;
}

void core2_array_realloc(core2_array_t *array, size_t new_length)
{
    dprintf("core2_array_realloc(array, %d)\n", new_length);

    if (new_length <= 0)
    {
        dprintf("core2_array_realloc(array, %d) - ERROR, new_length <= 0!\n", new_length);
        return;
    }

    if (array->memory == NULL)
    {
        array->memory = core2_malloc(array->element_size * new_length);
    }
    else
    {
        core2_realloc(array->memory, array->element_size * new_length);
    }

    array->length = new_length;
}

void core2_array_delete(core2_array_t *array)
{
    dprintf("core2_array_delete(array)\n");

    if (array->memory != NULL)
    {
        core2_free(array->memory);
        array->memory = NULL;
    }

    core2_free(array);
}

bool c2_array_index_check(core2_array_t *array, int index)
{
    if (index < 0 || index >= array->length)
    {
        dprintf("c2_array_index_check() - ERROR, out of bounds array access, index %d\n", index);
        return false;
    }

    return true;
}

void core2_array_get(core2_array_t *array, int index, void *element_ptr)
{
    dprintf("core2_array_get(array, %d, %p)\n", index, element_ptr);

    if (!c2_array_index_check(array, index))
    {
        *(void **)element_ptr = NULL;
        return;
    }

    size_t offset = index * array->element_size;
    *(void **)element_ptr = (void *)((size_t)array->memory + offset);
}

void core2_array_set(core2_array_t *array, int index, void *element_ptr)
{
    dprintf("core2_array_set(array, %d, %p)\n", index, element_ptr);

    if (!c2_array_index_check(array, index))
    {
        return;
    }

    size_t offset = index * array->element_size;
    memcpy((void *)((size_t)array->memory + offset), element_ptr, array->element_size);
}

void core2_array_insert_end(core2_array_t *array, void *element_ptr)
{
    dprintf("core2_array_insert_end(array, %p)\n", element_ptr);

    core2_array_realloc(array, array->length + 1);
    core2_array_set(array, array->length - 1, element_ptr);
}

//====================== Tests ==========================================================

typedef struct
{
    const char *str;
    int num;
} test_struct;

void core2_array_run_tests()
{
    core2_array_t *arr = core2_array_create(sizeof(test_struct));

    test_struct ts1 = {.str = "Element 1", .num = 42};
    core2_array_insert_end(arr, &ts1);

    test_struct ts2 = {.str = "Second Element", .num = 69};
    core2_array_insert_end(arr, &ts2);

    printf("Array length = %d\n", arr->length);

    for (size_t i = 0; i < arr->length; i++)
    {
        test_struct *elptr = NULL;
        core2_array_get(arr, i, &elptr);

        printf("elptr = %p\n", elptr);
        printf("%d - str = \"%s\", num = %d\n", i, elptr->str, elptr->num);
    }

    core2_array_delete(arr);
}