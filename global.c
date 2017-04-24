#include <stdarg.h>
#include <string.h>
#include <malloc.h>

#include "global.h"
#include "parse.h"

void **ptr_array = NULL;
int allocated_objs = 0, max_objs = 0;
sigjmp_buf error_jmp;

// Allocator used for the parse code, which is not managed by the student.
void *parse_alloc(size_t sz) {
    if (ptr_array == NULL) {
        ptr_array = malloc(sizeof(void *) * INITIAL_SIZE);
        max_objs = INITIAL_SIZE;
    } else if (allocated_objs == max_objs) {
        max_objs *= 2;
        ptr_array = realloc(ptr_array, sizeof(void *) * max_objs);
    }

    void *mem = malloc(sz);

    if (mem == NULL || ptr_array == NULL) {
        error(-1, "%s", "Allocation failed!");
    }

    ptr_array[allocated_objs++] = mem;
    return mem;
}

void error(int pos, const char *fmt, ...)  {
    printf("%s", curr_string());

    if (pos != -1) {
        for (int i = 0; i < pos; i++)
            printf("-");

        printf("^\n");

        printf("Parse error: ");
    } else {
        printf("Error: ");
    }

    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);

    printf("\n");

    longjmp(error_jmp, 1);
}

void parse_free_all() {
    for (int i = 0; i < allocated_objs; i++)
        free(ptr_array[i]);

    free(ptr_array);
    ptr_array = NULL;
    allocated_objs = max_objs = 0;
}

char *parse_string_dup(const char *str) {
    char *str2 = parse_alloc(sizeof(char) * (strlen(str) + 1));

    for (size_t i = 0; i < strlen(str) + 1; i++)
        str2[i] = str[i];

    return str2;
}
