#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"
#include "fieldinfo.h"
#include "complex.h"

typedef struct {
    const char* name;
    Matrix* matrix;
} MatrixBinding;

typedef struct {
    Matrix** items;
    int size;
    int capacity;
} MatrixStack;

static void assert_double_close(double got, double expected, double eps) {
    assert(fabs(got - expected) <= eps);
}

static void assert_complex_eq(const Complex* got, const Complex* expected, double eps) {
    assert_double_close(got->real, expected->real, eps);
    assert_double_close(got->image, expected->image, eps);
}

static void stack_init(MatrixStack* stack) {
    stack->items = NULL;
    stack->size = 0;
    stack->capacity = 0;
}

static void stack_push(MatrixStack* stack, Matrix* value) {
    if (stack->size == stack->capacity) {
        int new_capacity = (stack->capacity == 0) ? 8 : stack->capacity * 2;
        Matrix** new_items = (Matrix**)realloc(stack->items, (size_t)new_capacity * sizeof(Matrix*));
        if (new_items == NULL) {
            fprintf(stderr, "Не удалось расширить стек для ОПЗ-тестов\n");
            exit(1);
        }
        stack->items = new_items;
        stack->capacity = new_capacity;
    }
    stack->items[stack->size++] = value;
}

static Matrix* stack_pop(MatrixStack* stack) {
    if (stack->size == 0) {
        fprintf(stderr, "Некорректная ОПЗ-формула: недостаточно операндов\n");
        exit(1);
    }
    return stack->items[--stack->size];
}

static void stack_destroy(MatrixStack* stack) {
    for (int i = 0; i < stack->size; ++i) {
        DestroyMatrix(stack->items[i]);
    }
    free(stack->items);
    stack->items = NULL;
    stack->size = 0;
    stack->capacity = 0;
}

static Matrix* clone_matrix(const Matrix* source) {
    Matrix* copy = CreateMatrix(source->size, source->fieldInfo);
    for (int i = 0; i < source->size; ++i) {
        for (int j = 0; j < source->size; ++j) {
            SetElement(copy, (size_t)i, (size_t)j, GetElement((Matrix*)source, (size_t)i, (size_t)j));
        }
    }
    return copy;
}

static const Matrix* find_binding(const MatrixBinding* bindings, int binding_count, const char* token) {
    for (int i = 0; i < binding_count; ++i) {
        if (strcmp(bindings[i].name, token) == 0) {
            return bindings[i].matrix;
        }
    }
    return NULL;
}

static int is_number_token(const char* token) {
    if (*token == '\0') {
        return 0;
    }
    char* end = NULL;
    strtod(token, &end);
    return end != token && *end == '\0';
}

static Matrix* evaluate_rpn_expression(const char* expression,
                                       const MatrixBinding* bindings,
                                       int binding_count,
                                       FieldInfo* field_info) {
    char* buffer = (char*)malloc(strlen(expression) + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Не удалось выделить память под выражение ОПЗ\n");
        exit(1);
    }
    strcpy(buffer, expression);

    MatrixStack stack;
    stack_init(&stack);

    for (char* token = strtok(buffer, " \t\r\n"); token != NULL; token = strtok(NULL, " \t\r\n")) {
        if (strcmp(token, "+") == 0) {
            Matrix* right = stack_pop(&stack);
            Matrix* left = stack_pop(&stack);
            Matrix* result = AddMatrix(left, right);
            DestroyMatrix(right);
            DestroyMatrix(left);
            stack_push(&stack, result);
            continue;
        }

        if (strcmp(token, "*") == 0) {
            Matrix* right = stack_pop(&stack);
            Matrix* left = stack_pop(&stack);
            Matrix* result = MultiplyMatrix(left, right);
            DestroyMatrix(right);
            DestroyMatrix(left);
            stack_push(&stack, result);
            continue;
        }

        if (is_number_token(token)) {
            Matrix* value = stack_pop(&stack);
            if (field_info->type == INT_TYPE) {
                int scalar = (int)strtol(token, NULL, 10);
                MultiplyScalar(value, &scalar);
            } else {
                double scalar = strtod(token, NULL);
                MultiplyScalar(value, &scalar);
            }
            stack_push(&stack, value);
            continue;
        }

        const Matrix* found = find_binding(bindings, binding_count, token);
        if (found == NULL) {
            fprintf(stderr, "Неизвестный токен в ОПЗ-выражении: %s\n", token);
            free(buffer);
            stack_destroy(&stack);
            exit(1);
        }
        stack_push(&stack, clone_matrix(found));
    }

    free(buffer);

    if (stack.size != 1) {
        fprintf(stderr, "Некорректная ОПЗ-формула: после вычисления в стеке %d элементов\n", stack.size);
        stack_destroy(&stack);
        exit(1);
    }

    Matrix* result = stack_pop(&stack);
    free(stack.items);
    return result;
}

static void assert_int_matrix_equal(const Matrix* left, const Matrix* right) {
    assert(left->size == right->size);
    for (int i = 0; i < left->size; ++i) {
        for (int j = 0; j < left->size; ++j) {
            int a = *(int*)GetElement((Matrix*)left, (size_t)i, (size_t)j);
            int b = *(int*)GetElement((Matrix*)right, (size_t)i, (size_t)j);
            assert(a == b);
        }
    }
}

static void assert_complex_matrix_equal(const Matrix* left, const Matrix* right, double eps) {
    assert(left->size == right->size);
    for (int i = 0; i < left->size; ++i) {
        for (int j = 0; j < left->size; ++j) {
            Complex* a = (Complex*)GetElement((Matrix*)left, (size_t)i, (size_t)j);
            Complex* b = (Complex*)GetElement((Matrix*)right, (size_t)i, (size_t)j);
            assert_complex_eq(a, b, eps);
        }
    }
}

static Matrix* create_int_matrix_from_array(const int* values, int size, FieldInfo* fi) {
    Matrix* m = CreateMatrix(size, fi);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            SetElement(m, (size_t)i, (size_t)j, &values[i * size + j]);
        }
    }
    return m;
}

static Matrix* create_complex_matrix_from_array(const Complex* values, int size, FieldInfo* fi) {
    Matrix* m = CreateMatrix(size, fi);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            SetElement(m, (size_t)i, (size_t)j, &values[i * size + j]);
        }
    }
    return m;
}

static void test_rpn_int_distributivity(void) {
    FieldInfo* fi = GetIntFieldInfo();
    const int a[] = {
        1, 2, 0,
        -1, 3, 4,
        2, -2, 5
    };
    const int b[] = {
        0, 1, 2,
        3, -1, 0,
        4, 2, 1
    };
    const int c[] = {
        2, 0, 1,
        -3, 1, 2,
        0, 5, -1
    };

    Matrix* A = create_int_matrix_from_array(a, 3, fi);
    Matrix* B = create_int_matrix_from_array(b, 3, fi);
    Matrix* C = create_int_matrix_from_array(c, 3, fi);

    MatrixBinding bindings[] = {
        {"A", A},
        {"B", B},
        {"C", C}
    };

    Matrix* left = evaluate_rpn_expression("A B C + *", bindings, 3, fi);
    Matrix* right = evaluate_rpn_expression("A B * A C * +", bindings, 3, fi);

    assert_int_matrix_equal(left, right);

    DestroyMatrix(right);
    DestroyMatrix(left);
    DestroyMatrix(C);
    DestroyMatrix(B);
    DestroyMatrix(A);
}

static void test_rpn_int_associativity(void) {
    FieldInfo* fi = GetIntFieldInfo();
    const int a[] = {1, 0, 2, -1};
    const int b[] = {3, 1, 4, 2};
    const int c[] = {0, 5, -2, 1};

    Matrix* A = create_int_matrix_from_array(a, 2, fi);
    Matrix* B = create_int_matrix_from_array(b, 2, fi);
    Matrix* C = create_int_matrix_from_array(c, 2, fi);

    MatrixBinding bindings[] = {
        {"A", A},
        {"B", B},
        {"C", C}
    };

    Matrix* left = evaluate_rpn_expression("A B + C +", bindings, 3, fi);
    Matrix* right = evaluate_rpn_expression("A B C + +", bindings, 3, fi);

    assert_int_matrix_equal(left, right);

    DestroyMatrix(right);
    DestroyMatrix(left);
    DestroyMatrix(C);
    DestroyMatrix(B);
    DestroyMatrix(A);
}

static void test_rpn_int_scalar_distribution(void) {
    FieldInfo* fi = GetIntFieldInfo();
    const int a[] = {2, -1, 4, 3};
    const int b[] = {5, 0, -2, 1};

    Matrix* A = create_int_matrix_from_array(a, 2, fi);
    Matrix* B = create_int_matrix_from_array(b, 2, fi);

    MatrixBinding bindings[] = {
        {"A", A},
        {"B", B}
    };

    Matrix* left = evaluate_rpn_expression("A B + 3", bindings, 2, fi);
    Matrix* right = evaluate_rpn_expression("A 3 B 3 +", bindings, 2, fi);

    assert_int_matrix_equal(left, right);

    DestroyMatrix(right);
    DestroyMatrix(left);
    DestroyMatrix(B);
    DestroyMatrix(A);
}

static void test_rpn_complex_distributivity(void) {
    FieldInfo* fi = GetComplexFieldInfo();
    const Complex a[] = {
        {1.0, 1.0}, {2.0, -1.0},
        {0.0, 3.0}, {-2.0, 0.5}
    };
    const Complex b[] = {
        {-1.0, 2.0}, {0.5, 0.0},
        {4.0, -1.0}, {0.0, -2.0}
    };
    const Complex c[] = {
        {0.0, 1.5}, {2.0, 2.0},
        {-3.0, 0.5}, {1.0, -1.0}
    };

    Matrix* A = create_complex_matrix_from_array(a, 2, fi);
    Matrix* B = create_complex_matrix_from_array(b, 2, fi);
    Matrix* C = create_complex_matrix_from_array(c, 2, fi);

    MatrixBinding bindings[] = {
        {"A", A},
        {"B", B},
        {"C", C}
    };

    Matrix* left = evaluate_rpn_expression("A B C + *", bindings, 3, fi);
    Matrix* right = evaluate_rpn_expression("A B * A C * +", bindings, 3, fi);

    assert_complex_matrix_equal(left, right, 1e-9);

    DestroyMatrix(right);
    DestroyMatrix(left);
    DestroyMatrix(C);
    DestroyMatrix(B);
    DestroyMatrix(A);
}

static void test_rpn_complex_scalar_distribution(void) {
    FieldInfo* fi = GetComplexFieldInfo();
    const Complex a[] = {
        {1.0, -1.0}, {2.5, 0.0},
        {0.0, 4.0}, {-3.0, 2.0}
    };
    const Complex b[] = {
        {0.0, 0.5}, {-1.0, 1.0},
        {3.0, -2.0}, {1.5, 1.5}
    };

    Matrix* A = create_complex_matrix_from_array(a, 2, fi);
    Matrix* B = create_complex_matrix_from_array(b, 2, fi);

    MatrixBinding bindings[] = {
        {"A", A},
        {"B", B}
    };

    Matrix* left = evaluate_rpn_expression("A B + 2.5", bindings, 2, fi);
    Matrix* right = evaluate_rpn_expression("A 2.5 B 2.5 +", bindings, 2, fi);

    assert_complex_matrix_equal(left, right, 1e-9);

    DestroyMatrix(right);
    DestroyMatrix(left);
    DestroyMatrix(B);
    DestroyMatrix(A);
}

int main(void) {
    test_rpn_int_distributivity();
    test_rpn_int_associativity();
    test_rpn_int_scalar_distribution();
    test_rpn_complex_distributivity();
    test_rpn_complex_scalar_distribution();

    printf("formula tests OK\n");
    return 0;
}
