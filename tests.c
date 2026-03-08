#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"
#include "fieldinfo.h"
#include "complex.h"


static void assert_int_eq(int got, int expected) {
    assert(got == expected);
}

static void assert_double_close(double got, double expected, double eps) {
    assert(fabs(got - expected) <= eps);
}

static void assert_complex_eq(const Complex* got, const Complex* expected, double eps) {
    assert_double_close(got->real, expected->real, eps);
    assert_double_close(got->image, expected->image, eps);
}

static Complex complex_add_value(Complex a, Complex b) {
    Complex r = { a.real + b.real, a.image + b.image };
    return r;
}

static Complex complex_mul_value(Complex a, Complex b) {
    Complex r = {
        a.real * b.real - a.image * b.image,
        a.real * b.image + a.image * b.real
    };
    return r;
}

static Complex complex_scalar_mul_value(Complex a, double scalar) {
    Complex r = { a.real * scalar, a.image * scalar };
    return r;
}

static Complex complex_sub_value(Complex a, Complex b) {
    Complex r = { a.real - b.real, a.image - b.image };
    return r;
}

static Matrix* create_int_matrix_from_array(const int* values, int size, FieldInfo* fi) {
    Matrix* m = CreateMatrix(size, fi);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int v = values[i * size + j];
            SetElement(m, i, j, &v);
        }
    }
    return m;
}

static Matrix* create_complex_matrix_from_array(const Complex* values, int size, FieldInfo* fi) {
    Matrix* m = CreateMatrix(size, fi);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            Complex v = values[i * size + j];
            SetElement(m, i, j, &v);
        }
    }
    return m;
}

static Matrix* create_identity_int_matrix(int size, FieldInfo* fi) {
    Matrix* m = CreateMatrix(size, fi);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int v = (i == j) ? 1 : 0;
            SetElement(m, i, j, &v);
        }
    }
    return m;
}

static Matrix* create_zero_int_matrix(int size, FieldInfo* fi) {
    Matrix* m = CreateMatrix(size, fi);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int v = 0;
            SetElement(m, i, j, &v);
        }
    }
    return m;
}

static void assert_int_matrix_matches_array(const Matrix* m, const int* expected) {
    for (int i = 0; i < m->size; ++i) {
        for (int j = 0; j < m->size; ++j) {
            int got = *(int*)GetElement((Matrix*)m, i, j);
            assert_int_eq(got, expected[i * m->size + j]);
        }
    }
}

static void assert_complex_matrix_matches_array(const Matrix* m, const Complex* expected, double eps) {
    for (int i = 0; i < m->size; ++i) {
        for (int j = 0; j < m->size; ++j) {
            Complex* got = (Complex*)GetElement((Matrix*)m, i, j);
            assert_complex_eq(got, &expected[i * m->size + j], eps);
        }
    }
}

static void compute_expected_int_sum(const int* a, const int* b, int* out, int size) {
    for (int i = 0; i < size * size; ++i) {
        out[i] = a[i] + b[i];
    }
}

static void compute_expected_int_product(const int* a, const int* b, int* out, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int sum = 0;
            for (int k = 0; k < size; ++k) {
                sum += a[i * size + k] * b[k * size + j];
            }
            out[i * size + j] = sum;
        }
    }
}

static void compute_expected_int_scalar_product(const int* a, int scalar, int* out, int size) {
    for (int i = 0; i < size * size; ++i) {
        out[i] = a[i] * scalar;
    }
}

static int manual_int_determinant(const int* values, int size) {
    if (size == 1) {
        return values[0];
    }
    if (size == 2) {
        return values[0] * values[3] - values[1] * values[2];
    }

    int det = 0;
    for (int col = 0; col < size; ++col) {
        int* minor = (int*)malloc((size - 1) * (size - 1) * sizeof(int));
        int idx = 0;
        for (int i = 1; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (j == col) {
                    continue;
                }
                minor[idx++] = values[i * size + j];
            }
        }
        int sign = (col % 2 == 0) ? 1 : -1;
        det += sign * values[col] * manual_int_determinant(minor, size - 1);
        free(minor);
    }
    return det;
}

static void compute_expected_complex_sum(const Complex* a, const Complex* b, Complex* out, int size) {
    for (int i = 0; i < size * size; ++i) {
        out[i] = complex_add_value(a[i], b[i]);
    }
}

static void compute_expected_complex_product(const Complex* a, const Complex* b, Complex* out, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            Complex sum = {0.0, 0.0};
            for (int k = 0; k < size; ++k) {
                sum = complex_add_value(sum,
                                        complex_mul_value(a[i * size + k], b[k * size + j]));
            }
            out[i * size + j] = sum;
        }
    }
}

static void compute_expected_complex_scalar_product(const Complex* a, double scalar, Complex* out, int size) {
    for (int i = 0; i < size * size; ++i) {
        out[i] = complex_scalar_mul_value(a[i], scalar);
    }
}

static Complex manual_complex_determinant(const Complex* values, int size) {
    if (size == 1) {
        return values[0];
    }
    if (size == 2) {
        return complex_sub_value(
            complex_mul_value(values[0], values[3]),
            complex_mul_value(values[1], values[2])
        );
    }

    Complex det = {0.0, 0.0};
    for (int col = 0; col < size; ++col) {
        Complex* minor = (Complex*)malloc((size - 1) * (size - 1) * sizeof(Complex));
        int idx = 0;
        for (int i = 1; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (j == col) {
                    continue;
                }
                minor[idx++] = values[i * size + j];
            }
        }

        Complex cofactor = manual_complex_determinant(minor, size - 1);
        Complex term = complex_mul_value(values[col], cofactor);
        if (col % 2 != 0) {
            term.real = -term.real;
            term.image = -term.image;
        }
        det = complex_add_value(det, term);
        free(minor);
    }
    return det;
}

static void test_int_add_mul_2x2_formula_based(void) {
    FieldInfo* fi = GetIntFieldInfo();
    const int a[] = {1, 2, 3, 4};
    const int b[] = {5, 6, 7, 8};
    int expected_sum[4];
    int expected_mul[4];

    compute_expected_int_sum(a, b, expected_sum, 2);
    compute_expected_int_product(a, b, expected_mul, 2);

    Matrix* A = create_int_matrix_from_array(a, 2, fi);
    Matrix* B = create_int_matrix_from_array(b, 2, fi);
    Matrix* S = AddMatrix(A, B);
    Matrix* M = MultiplyMatrix(A, B);

    assert_int_matrix_matches_array(S, expected_sum);
    assert_int_matrix_matches_array(M, expected_mul);

    DestroyMatrix(M);
    DestroyMatrix(S);
    DestroyMatrix(B);
    DestroyMatrix(A);
}

static void test_int_scalar_mul_formula_based(void) {
    FieldInfo* fi = GetIntFieldInfo();
    const int a[] = {-1, 0, 7, -3};
    int expected_zero[4];
    int expected_negative[4];

    Matrix* A = create_int_matrix_from_array(a, 2, fi);

    compute_expected_int_scalar_product(a, 0, expected_zero, 2);
    int scalar = 0;
    MultiplyScalar(A, &scalar);
    assert_int_matrix_matches_array(A, expected_zero);

    compute_expected_int_scalar_product(expected_zero, -2, expected_negative, 2);
    scalar = -2;
    MultiplyScalar(A, &scalar);
    assert_int_matrix_matches_array(A, expected_negative);

    DestroyMatrix(A);
}

static void test_int_identity_and_zero_for_many_sizes(void) {
    FieldInfo* fi = GetIntFieldInfo();

    for (int size = 1; size <= 4; ++size) {
        int* values = (int*)malloc(size * size * sizeof(int));
        for (int i = 0; i < size * size; ++i) {
            values[i] = i - 3;
        }

        Matrix* A = create_int_matrix_from_array(values, size, fi);
        Matrix* I = create_identity_int_matrix(size, fi);
        Matrix* Z = create_zero_int_matrix(size, fi);

        Matrix* left = MultiplyMatrix(I, A);
        Matrix* right = MultiplyMatrix(A, I);
        Matrix* sum = AddMatrix(A, Z);

        assert_int_matrix_matches_array(left, values);
        assert_int_matrix_matches_array(right, values);
        assert_int_matrix_matches_array(sum, values);

        DestroyMatrix(sum);
        DestroyMatrix(right);
        DestroyMatrix(left);
        DestroyMatrix(Z);
        DestroyMatrix(I);
        DestroyMatrix(A);
        free(values);
    }
}

static void test_int_distributivity_3x3(void) {
    FieldInfo* fi = GetIntFieldInfo();
    const int a[] = {1, 2, 0, -1, 3, 4, 2, -2, 5};
    const int b[] = {0, 1, 2, 3, -1, 0, 4, 2, 1};
    const int c[] = {2, 0, 1, -3, 1, 2, 0, 5, -1};

    int b_plus_c[9];
    int expected_left[9];
    int expected_ab[9];
    int expected_ac[9];
    int expected_right[9];

    compute_expected_int_sum(b, c, b_plus_c, 3);
    compute_expected_int_product(a, b_plus_c, expected_left, 3);
    compute_expected_int_product(a, b, expected_ab, 3);
    compute_expected_int_product(a, c, expected_ac, 3);
    compute_expected_int_sum(expected_ab, expected_ac, expected_right, 3);

    Matrix* A = create_int_matrix_from_array(a, 3, fi);
    Matrix* B = create_int_matrix_from_array(b, 3, fi);
    Matrix* C = create_int_matrix_from_array(c, 3, fi);

    Matrix* BplusC = AddMatrix(B, C);
    Matrix* left = MultiplyMatrix(A, BplusC);
    Matrix* AB = MultiplyMatrix(A, B);
    Matrix* AC = MultiplyMatrix(A, C);
    Matrix* right = AddMatrix(AB, AC);

    assert_int_matrix_matches_array(left, expected_left);
    assert_int_matrix_matches_array(right, expected_right);
    assert_int_matrix_matches_array(left, expected_right);

    DestroyMatrix(right);
    DestroyMatrix(AC);
    DestroyMatrix(AB);
    DestroyMatrix(left);
    DestroyMatrix(BplusC);
    DestroyMatrix(C);
    DestroyMatrix(B);
    DestroyMatrix(A);
}

static void test_int_determinant_formula_based(void) {
    FieldInfo* fi = GetIntFieldInfo();

    const int a2[] = {1, 2, 3, 4};
    const int a3[] = {
        6, 1, 1,
        4, -2, 5,
        2, 8, 7
    };

    Matrix* A2 = create_int_matrix_from_array(a2, 2, fi);
    Matrix* A3 = create_int_matrix_from_array(a3, 3, fi);

    int expected2 = manual_int_determinant(a2, 2);
    int expected3 = manual_int_determinant(a3, 3);

    int* det2 = (int*)CalculateDeterminant(A2);
    int* det3 = (int*)CalculateDeterminant(A3);

    assert_int_eq(*det2, expected2);
    assert_int_eq(*det3, expected3);

    free(det3);
    free(det2);
    DestroyMatrix(A3);
    DestroyMatrix(A2);
}

static void test_complex_add_mul_formula_based(void) {
    FieldInfo* fi = GetComplexFieldInfo();

    const Complex a[] = {
        {1, 1}, {2, -1},
        {0, 3}, {-2, 0.5}
    };
    const Complex b[] = {
        {-1, 2}, {0.5, 0},
        {4, -1}, {0, -2}
    };

    Complex expected_sum[4];
    Complex expected_mul[4];

    compute_expected_complex_sum(a, b, expected_sum, 2);
    compute_expected_complex_product(a, b, expected_mul, 2);

    Matrix* A = create_complex_matrix_from_array(a, 2, fi);
    Matrix* B = create_complex_matrix_from_array(b, 2, fi);
    Matrix* S = AddMatrix(A, B);
    Matrix* M = MultiplyMatrix(A, B);

    assert_complex_matrix_matches_array(S, expected_sum, 1e-9);
    assert_complex_matrix_matches_array(M, expected_mul, 1e-9);

    DestroyMatrix(M);
    DestroyMatrix(S);
    DestroyMatrix(B);
    DestroyMatrix(A);
}

static void test_complex_scalar_mul_formula_based(void) {
    FieldInfo* fi = GetComplexFieldInfo();

    const Complex a[] = {
        {0, 0}, {1.25, -4},
        {1.25, -4}, {0, 0}
    };
    Complex expected_zero[4];
    Complex expected_negative[4];

    Matrix* A = create_complex_matrix_from_array(a, 2, fi);

    compute_expected_complex_scalar_product(a, 0.0, expected_zero, 2);
    double scalar = 0.0;
    MultiplyScalar(A, &scalar);
    assert_complex_matrix_matches_array(A, expected_zero, 1e-12);

    compute_expected_complex_scalar_product(expected_zero, -2.0, expected_negative, 2);
    scalar = -2.0;
    MultiplyScalar(A, &scalar);
    assert_complex_matrix_matches_array(A, expected_negative, 1e-12);

    DestroyMatrix(A);
}

static void test_complex_determinant_formula_based(void) {
    FieldInfo* fi = GetComplexFieldInfo();
    const Complex a[] = {
        {1, 1}, {2, 0},
        {0, 1}, {3, -1}
    };

    Matrix* A = create_complex_matrix_from_array(a, 2, fi);
    Complex expected = manual_complex_determinant(a, 2);
    Complex* det = (Complex*)CalculateDeterminant(A);

    assert_complex_eq(det, &expected, 1e-9);

    free(det);
    DestroyMatrix(A);
}

int main(void) {
    test_int_add_mul_2x2_formula_based();
    test_int_scalar_mul_formula_based();
    test_int_identity_and_zero_for_many_sizes();
    test_int_distributivity_3x3();
    test_int_determinant_formula_based();

    test_complex_add_mul_formula_based();
    test_complex_scalar_mul_formula_based();
    test_complex_determinant_formula_based();

    printf("OK\n");
    return 0;
}
