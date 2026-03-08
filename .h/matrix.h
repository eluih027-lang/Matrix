#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fieldinfo.h"
#include "complex.h"

typedef struct {
    int size;
    FieldInfo* fieldInfo;
    void** data;
} Matrix;

Matrix* CreateMatrix(int size, FieldInfo* fieldinfo);
void DestroyMatrix(Matrix* matrix);
void SetElement(Matrix* matrix, size_t row, size_t column, const void* value);
void SetMatrix(Matrix* matrix);
void* GetElement(Matrix* matrix, size_t row, size_t column);

Matrix* AddMatrix(const Matrix* m1, const Matrix* m2);
Matrix* MultiplyMatrix(const Matrix* m1, const Matrix* m2);
void MultiplyScalar(Matrix* matrix, const void* scalar);
void* CalculateDeterminant(const Matrix* matrix);
void PrintMatrix(Matrix* matrix);

#endif
