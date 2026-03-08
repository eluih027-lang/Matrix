#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include "matrix.h"
#include "fieldinfo.h"
#include "complex.h"

static void ReadLine(char* buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        fprintf(stderr, "Ошибка чтения ввода.\n");
        exit(1);
    }
}

static int ReadMenuChoiceInRange(const char* prompt, int min, int max) {
    char buffer[256];
    int value;
    char extra;

    while (1) {
        printf("%s", prompt);
        ReadLine(buffer, sizeof(buffer));
        if (sscanf(buffer, " %d %c", &value, &extra) == 1 && value >= min && value <= max) {
            return value;
        }
        printf("Ошибка: неверный формат ввода. Введите число от %d до %d.\n", min, max);
    }
}

static int ReadIntValue(const char* prompt) {
    char buffer[256];
    int value;
    char extra;

    while (1) {
        printf("%s", prompt);
        ReadLine(buffer, sizeof(buffer));
        if (sscanf(buffer, " %d %c", &value, &extra) == 1) {
            return value;
        }
        printf("Ошибка: неверный формат ввода. Ожидалось целое число.\n");
    }
}

static double ReadDoubleValue(const char* prompt) {
    char buffer[256];
    double value;
    char extra;

    while (1) {
        printf("%s", prompt);
        ReadLine(buffer, sizeof(buffer));
        if (sscanf(buffer, " %lf %c", &value, &extra) == 1) {
            return value;
        }
        printf("Ошибка: неверный формат ввода. Ожидалось вещественное число.\n");
    }
}

static void* ReadScalar(FieldInfo* fieldInfo) {
    void* scalar = NULL;
    if (fieldInfo->type == COMPLEX_TYPE) {
        scalar = malloc(sizeof(double));
        if (scalar == NULL) {
            fprintf(stderr, "Не удалось выделить память для скаляра.\n");
            exit(1);
        }
        *(double*)scalar = ReadDoubleValue("Введите вещественный скаляр: ");
    } else {
        scalar = malloc(fieldInfo->elementSize);
        if (scalar == NULL) {
            fprintf(stderr, "Не удалось выделить память для скаляра.\n");
            exit(1);
        }
        *(int*)scalar = ReadIntValue("Введите скаляр: ");
    }
    return scalar;
}

static Matrix* ReadMatrix(FieldInfo* fieldInfo, const char* title) {
    printf("Ввод %s\n", title);
    int size = 0;
    while (size <= 0) {
        size = ReadIntValue("Введите размер матрицы: ");
        if (size <= 0) {
            printf("Ошибка: размер матрицы должен быть положительным целым числом.\n");
        }
    }

    Matrix* matrix = CreateMatrix(size, fieldInfo);
    SetMatrix(matrix);
    return matrix;
}

static void PrintDeterminant(FieldInfo* fieldInfo, void* determinant) {
    if (fieldInfo->type == INT_TYPE) {
        printf("Определитель: %d\n", *(int*)determinant);
    } else if (fieldInfo->type == COMPLEX_TYPE) {
        Complex value = *(Complex*)determinant;
        printf("Определитель: (%.6lf + %.6lfi)\n", value.real, value.image);
    }
}

int main() {
    setlocale(LC_ALL, "");

    while (1) {
        printf("Выберите операцию:\n");
        printf("1. Умножить матрицу на скаляр\n");
        printf("2. Умножить две матрицы\n");
        printf("3. Сложить две матрицы\n");
        printf("4. Вычислить определитель матрицы\n");
        printf("5. Выход\n");
        int choice = ReadMenuChoiceInRange("Ваш выбор: ", 1, 5);

        if (choice == 5) {
            printf("Выход из программы.\n");
            break;
        }

        printf("Выберите тип данных:\n");
        printf("1. Целочисленный.\n");
        printf("2. Комплексный (элементы комплексные, скаляры вещественные)\n");
        int typeChoice = ReadMenuChoiceInRange("Ваш выбор: ", 1, 2);

        FieldInfo fieldInfo;
        if (typeChoice == 1) {
            fieldInfo = *GetIntFieldInfo();
        } else {
            fieldInfo = *GetComplexFieldInfo();
        }

        Matrix* m1 = ReadMatrix(&fieldInfo, "матрицы");
        PrintMatrix(m1);

        Matrix* m2 = NULL;
        Matrix* result = NULL;

        if (choice == 1) {
            void* scalar = ReadScalar(&fieldInfo);
            MultiplyScalar(m1, scalar);
            free(scalar);
            result = m1;
            printf("Результат:\n");
            PrintMatrix(result);
        } else if (choice == 2) {
            m2 = ReadMatrix(&fieldInfo, "второй матрицы");
            PrintMatrix(m2);
            result = MultiplyMatrix(m1, m2);
            printf("Результат:\n");
            PrintMatrix(result);
        } else if (choice == 3) {
            m2 = ReadMatrix(&fieldInfo, "второй матрицы");
            PrintMatrix(m2);
            result = AddMatrix(m1, m2);
            printf("Результат:\n");
            PrintMatrix(result);
        } else {
            void* determinant = CalculateDeterminant(m1);
            PrintDeterminant(&fieldInfo, determinant);
            free(determinant);
        }

        if (result != m1 && result != NULL) DestroyMatrix(result);
        if (m2 != NULL) DestroyMatrix(m2);
        DestroyMatrix(m1);

        printf("\n");
    }

    return 0;
}
