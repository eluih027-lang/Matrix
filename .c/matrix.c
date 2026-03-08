#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fieldinfo.h"
#include "complex.h"
#include "matrix.h"

static void ReadLine(char* buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        fprintf(stderr, "Ошибка чтения ввода.\n");
        exit(1);
    }
}

static int ReadIntElement(int row, int column) {
    char buffer[256];
    int value;
    char extra;

    while (1) {
        printf("Введите элемент [%d][%d]: ", row, column);
        ReadLine(buffer, sizeof(buffer));
        if (sscanf(buffer, " %d %c", &value, &extra) == 1) {
            return value;
        }
        printf("Ошибка: неверный формат ввода. Ожидалось целое число.\n");
    }
}

static Complex ReadComplexElement(int row, int column) {
    char buffer[256];
    Complex value;
    char extra;

    while (1) {
        printf("Введите элемент [%d][%d] (вещественная и мнимая части через пробел): ", row, column);
        ReadLine(buffer, sizeof(buffer));
        if (sscanf(buffer, " %lf %lf %c", &value.real, &value.image, &extra) == 2) {
            return value;
        }
        printf("Ошибка: неверный формат ввода. Ожидались две вещественные величины.\n");
    }
}

static Matrix* CreateMinorMatrix(const Matrix* matrix, int skipRow, int skipColumn) {
    Matrix* minor = CreateMatrix(matrix->size - 1, matrix->fieldInfo);
    int minorRow = 0;

    for (int i = 0; i < matrix->size; i++) {
        if (i == skipRow) {
            continue;
        }
        int minorColumn = 0;
        for (int j = 0; j < matrix->size; j++) {
            if (j == skipColumn) {
                continue;
            }
            SetElement(minor, (size_t)minorRow, (size_t)minorColumn, GetElement((Matrix*)matrix, (size_t)i, (size_t)j));
            minorColumn++;
        }
        minorRow++;
    }

    return minor;
}

static void AddInPlace(FieldInfo* fieldInfo, void* target, const void* value) {
    void* temp = malloc(fieldInfo->elementSize);
    if (temp == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        exit(1);
    }
    fieldInfo->add(temp, target, value);
    memcpy(target, temp, fieldInfo->elementSize);
    free(temp);
}

Matrix* CreateMatrix(int size, FieldInfo* fieldInfo) {
    if (size <= 0) {
        fprintf(stderr, "Невозможно создать матрицу размерностью меньше единицы\n");
        exit(1);
    }
    if (fieldInfo == NULL) {
        fprintf(stderr, "Не задан FieldInfo\n");
        exit(1);
    }
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    if (matrix == NULL) {
        fprintf(stderr, "Не удалось выделить память для параметров матрицы\n");
        exit(1);
    }
    matrix->size = size;
    matrix->fieldInfo = fieldInfo;
    matrix->data = (void**)malloc((size_t)size * sizeof(void*));
    if (matrix->data == NULL) {
        fprintf(stderr, "Не удалось выделить память для массива указателей матрицы\n");
        free(matrix);
        exit(1);
    }
    for (int i = 0; i < size; i++) {
        matrix->data[i] = calloc((size_t)size, fieldInfo->elementSize);
        if (matrix->data[i] == NULL) {
            fprintf(stderr, "Не удалось выделить память для строки матрицы\n");
            for (int j = 0; j < i; j++) {
                free(matrix->data[j]);
            }
            free(matrix->data);
            free(matrix);
            exit(1);
        }
    }
    return matrix;
}

void DestroyMatrix(Matrix* matrix) {
    if (matrix == NULL) return;
    for (int i = 0; i < matrix->size; i++) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
}

void SetElement(Matrix* matrix, size_t row, size_t column, const void* value) {
    if (row >= (size_t)matrix->size || column >= (size_t)matrix->size) {
        fprintf(stderr, "Выход за границы матрицы\n");
        exit(1);
    }
    size_t elementSize = matrix->fieldInfo->elementSize;
    memcpy((char*)matrix->data[row] + column * elementSize, value, elementSize);
}

void SetMatrix(Matrix* matrix) {
    void* value = malloc(matrix->fieldInfo->elementSize);
    if (value == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память для значения элемента\n");
        exit(1);
    }

    for (int i = 0; i < matrix->size; i++) {
        for (int j = 0; j < matrix->size; j++) {
            if (matrix->fieldInfo->type == INT_TYPE) {
                *(int*)value = ReadIntElement(i, j);
            } else if (matrix->fieldInfo->type == COMPLEX_TYPE) {
                *(Complex*)value = ReadComplexElement(i, j);
            } else {
                fprintf(stderr, "Неверный тип данных\n");
                free(value);
                exit(1);
            }
            SetElement(matrix, (size_t)i, (size_t)j, value);
        }
    }

    free(value);
}

void* GetElement(Matrix* matrix, size_t row, size_t column) {
    if (row >= (size_t)matrix->size || column >= (size_t)matrix->size) {
        fprintf(stderr, "Выход за границы матрицы\n");
        exit(1);
    }
    return (char*)matrix->data[row] + column * matrix->fieldInfo->elementSize;
}

Matrix* AddMatrix(const Matrix* m1, const Matrix* m2) {
    if (m1->size != m2->size) {
        fprintf(stderr, "Нельзя сложить матрицы разной размерности\n");
        exit(1);
    }
    if (m1->fieldInfo->type != m2->fieldInfo->type) {
        fprintf(stderr, "Нельзя сложить матрицы с разными типами данных\n");
        exit(1);
    }

    Matrix* result = CreateMatrix(m1->size, m1->fieldInfo);

    for (int i = 0; i < m1->size; i++) {
        for (int j = 0; j < m1->size; j++) {
            void* a = GetElement((Matrix*)m1, (size_t)i, (size_t)j);
            void* b = GetElement((Matrix*)m2, (size_t)i, (size_t)j);
            void* r = GetElement(result, (size_t)i, (size_t)j);
            m1->fieldInfo->add(r, a, b);
        }
    }

    return result;
}

Matrix* MultiplyMatrix(const Matrix* m1, const Matrix* m2) {
    if (m1->size != m2->size) {
        fprintf(stderr, "Нельзя перемножить матрицы разной размерности\n");
        exit(1);
    }
    if (m1->fieldInfo->type != m2->fieldInfo->type) {
        fprintf(stderr, "Нельзя перемножить матрицы с разными типами данных\n");
        exit(1);
    }

    Matrix* result = CreateMatrix(m1->size, m1->fieldInfo);
    size_t elemSize = m1->fieldInfo->elementSize;
    void* sum = malloc(elemSize);
    void* current = malloc(elemSize);
    if (!sum || !current) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        exit(1);
    }

    for (int i = 0; i < m1->size; i++) {
        for (int j = 0; j < m1->size; j++) {
            memset(sum, 0, elemSize);
            for (int k = 0; k < m1->size; k++) {
                void* elem1 = GetElement((Matrix*)m1, (size_t)i, (size_t)k);
                void* elem2 = GetElement((Matrix*)m2, (size_t)k, (size_t)j);
                m1->fieldInfo->mul(current, elem1, elem2);
                m1->fieldInfo->add(sum, sum, current);
            }
            SetElement(result, (size_t)i, (size_t)j, sum);
        }
    }

    free(sum);
    free(current);
    return result;
}

void MultiplyScalar(Matrix* matrix, const void* scalar) {
    for (int i = 0; i < matrix->size; i++) {
        for (int j = 0; j < matrix->size; j++) {
            void* element = GetElement(matrix, (size_t)i, (size_t)j);
            matrix->fieldInfo->scalarMul(element, element, scalar);
        }
    }
}

void* CalculateDeterminant(const Matrix* matrix) {
    if (matrix == NULL) {
        fprintf(stderr, "Матрица не задана\n");
        exit(1);
    }

    void* determinant = calloc(1, matrix->fieldInfo->elementSize);
    if (determinant == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        exit(1);
    }

    if (matrix->size == 1) {
        memcpy(determinant, GetElement((Matrix*)matrix, 0, 0), matrix->fieldInfo->elementSize);
        return determinant;
    }

    if (matrix->fieldInfo->type == INT_TYPE) {
        if (matrix->size == 2) {
            int a = *(int*)GetElement((Matrix*)matrix, 0, 0);
            int b = *(int*)GetElement((Matrix*)matrix, 0, 1);
            int c = *(int*)GetElement((Matrix*)matrix, 1, 0);
            int d = *(int*)GetElement((Matrix*)matrix, 1, 1);
            *(int*)determinant = a * d - b * c;
            return determinant;
        }

        for (int j = 0; j < matrix->size; j++) {
            Matrix* minor = CreateMinorMatrix(matrix, 0, j);
            int* minorDet = (int*)CalculateDeterminant(minor);
            int term = *(int*)GetElement((Matrix*)matrix, 0, (size_t)j) * (*minorDet);
            if (j % 2 != 0) {
                term = -term;
            }
            AddInPlace(matrix->fieldInfo, determinant, &term);
            free(minorDet);
            DestroyMatrix(minor);
        }
        return determinant;
    }

    if (matrix->fieldInfo->type == COMPLEX_TYPE) {
        if (matrix->size == 2) {
            Complex leftProduct;
            Complex rightProduct;
            Complex negativeOne = { .real = -1.0, .image = 0.0 };

            matrix->fieldInfo->mul(&leftProduct, GetElement((Matrix*)matrix, 0, 0), GetElement((Matrix*)matrix, 1, 1));
            matrix->fieldInfo->mul(&rightProduct, GetElement((Matrix*)matrix, 0, 1), GetElement((Matrix*)matrix, 1, 0));
            matrix->fieldInfo->scalarMul(&rightProduct, &rightProduct, &negativeOne.real);
            matrix->fieldInfo->add(determinant, &leftProduct, &rightProduct);
            return determinant;
        }

        for (int j = 0; j < matrix->size; j++) {
            Matrix* minor = CreateMinorMatrix(matrix, 0, j);
            Complex* minorDet = (Complex*)CalculateDeterminant(minor);
            Complex term;
            double sign = (j % 2 == 0) ? 1.0 : -1.0;

            matrix->fieldInfo->mul(&term, GetElement((Matrix*)matrix, 0, (size_t)j), minorDet);
            matrix->fieldInfo->scalarMul(&term, &term, &sign);
            AddInPlace(matrix->fieldInfo, determinant, &term);
            free(minorDet);
            DestroyMatrix(minor);
        }
        return determinant;
    }

    fprintf(stderr, "Неверный тип данных\n");
    free(determinant);
    exit(1);
}

void PrintMatrix(Matrix* matrix) {
    printf("Матрица размером %d x %d:\n", matrix->size, matrix->size);
    for (int i = 0; i < matrix->size; i++) {
        for (int j = 0; j < matrix->size; j++) {
            void* elem = GetElement(matrix, (size_t)i, (size_t)j);
            if (matrix->fieldInfo->type == INT_TYPE) {
                printf("%d ", *(int*)elem);
            } else if (matrix->fieldInfo->type == COMPLEX_TYPE) {
                Complex c = *(Complex*)elem;
                printf("(%.6lf + %.6lfi) ", c.real, c.image);
            }
        }
        printf("\n");
    }
    printf("\n");
}
