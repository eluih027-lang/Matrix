#ifndef FIELDINFO_H
#define FIELDINFO_H

#include <stdlib.h>
#include <stdio.h>
#include "complex.h"

typedef enum {
    INT_TYPE,
    COMPLEX_TYPE
} FieldType;

typedef struct {
    FieldType type;
    size_t elementSize;
    void (*add)(void* result, const void* a, const void* b);
    void (*mul)(void* result, const void* a, const void* b);
    void (*scalarMul)(void* result, const void* a, const void* scalar);
} FieldInfo;

FieldInfo* GetIntFieldInfo();
FieldInfo* GetComplexFieldInfo();
int FieldInfoComparator(const FieldInfo* a, const FieldInfo* b);
void FreeFieldInfo();

#endif
