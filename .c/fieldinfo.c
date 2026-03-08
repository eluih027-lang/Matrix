#include "fieldinfo.h"
#include "complex.h"
static FieldInfo* INT_FIELD_INFO=NULL;
static FieldInfo* COMPLEX_FIELD_INFO=NULL;

void AddInt(void* result, const void* a, const void* b) {
    *(int*)result = *(const int*)a + *(const int*)b;
}
void MulInt(void* result, const void* a, const void* b) {
    *(int*)result = *(const int*)a * *(const int*)b;
}
void ScalarMulInt(void* result, const void* a, const void* scalar) {
    *(int*)result = *(const int*)a * *(const int*)scalar;
}
void AddictComplex(void* result, const void* a, const void* b) {
    Complex* r = (Complex*)result;
    Complex* c1 = (Complex*)a;
    Complex* c2 = (Complex*)b;
    (*r).real = (*c1).real + (*c2).real;
    (*r).image = (*c1).image + (*c2).image;
}

void MulComplex(void* result, const void* a, const void* b) {
    Complex* r = (Complex*)result;
    Complex* c1 = (Complex*)a;
    Complex* c2 = (Complex*)b;
    double real = (*c1).real * (*c2).real - (*c1).image * (*c2).image;
    double imag = (*c1).real * (*c2).image + (*c1).image * (*c2).real;
    (*r).real = real;
    (*r).image = imag;
}

void ScalarMulComplex(void* result, const void* a, const void* scalar) {
    Complex* r = (Complex*)result;
    Complex* c = (Complex*)a;
    double s = *(const double*)scalar;
    (*r).real = (*c).real * s;
    (*r).image = (*c).image * s;
}
FieldInfo* GetIntFieldInfo(){
    if (!INT_FIELD_INFO){
        INT_FIELD_INFO=(FieldInfo*)malloc(sizeof(FieldInfo));
        if (INT_FIELD_INFO==NULL){
            fprintf(stderr,"Невозможно выделить память для целого числа\n");
            exit(1);
        }
        (*INT_FIELD_INFO).type=INT_TYPE;
        (*INT_FIELD_INFO).elementSize=sizeof(int);
        (*INT_FIELD_INFO).add = AddInt;
        (*INT_FIELD_INFO).mul = MulInt;
        (*INT_FIELD_INFO).scalarMul = ScalarMulInt;
    }
    return INT_FIELD_INFO;
}
FieldInfo* GetComplexFieldInfo(){
    if (!COMPLEX_FIELD_INFO){
        COMPLEX_FIELD_INFO=(FieldInfo*)malloc(sizeof(FieldInfo));
        if (COMPLEX_FIELD_INFO==NULL){
            fprintf(stderr,"Невозможно выделить память для комплексного числа\n");
            exit(1);
        }
        (*COMPLEX_FIELD_INFO).type=COMPLEX_TYPE;
        (*COMPLEX_FIELD_INFO).elementSize=sizeof(Complex);
        (*COMPLEX_FIELD_INFO).add = AddictComplex;
        (*COMPLEX_FIELD_INFO).mul = MulComplex;
        (*COMPLEX_FIELD_INFO).scalarMul = ScalarMulComplex;
    }
    return COMPLEX_FIELD_INFO;
}
void FreeFieldInfo() {
    if (INT_FIELD_INFO) {
        free(INT_FIELD_INFO);
        INT_FIELD_INFO = NULL;
    }
    if (COMPLEX_FIELD_INFO) {
        free(COMPLEX_FIELD_INFO);
        COMPLEX_FIELD_INFO = NULL;
    }
}
int FieldInfoComparator(const FieldInfo *a,const FieldInfo *b){
    if (a==NULL || b==NULL){
        return 0;
    }
    return (((*a).type==(*b).type) && ((*a).elementSize==(*b).elementSize));
}