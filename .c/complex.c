#include <stdio.h>
#include <stdlib.h>
#include "complex.h"
#include <math.h>
Complex *CreateComplex(double real,double image) {
    Complex *c=(Complex*)malloc(sizeof(Complex));
    if (c==NULL){
        fprintf(stderr, "Память не может быть выделена в функции CreateComplex\n");
        exit(1);
    }
    (*c).real=real;
    (*c).image=image;
    return c;
}
void DestroyComplex(Complex *c){
    if (c==NULL){
        fprintf(stderr,"Невозможно удалить NULL-указатели\n");
        exit(1);
    }
    free(c);
}
Complex* AddComplex(const Complex *c1,const Complex* c2){
    if ((c1==NULL) || (c2==NULL)){
        fprintf(stderr,"Невозможно складывать NULL-указатели\n");
        exit(1);
    }
    return CreateComplex((*c1).real+(*c2).real, (*c1).image+(*c2).image);
}
Complex* ComplexScalarMul(const Complex* a, const double* scalar) {
    return CreateComplex(a->real * (*scalar), a->image * (*scalar));
}
Complex* MultiplyComplex(const Complex *c1,const Complex *c2){
    if((c1==NULL) || (c2==NULL)){
        fprintf(stderr,"Невозможно умножать NULL-указатели\n");
        exit(1);
    }
    return CreateComplex((*c1).real*(*c2).real-(*c1).image*(*c2).image, (*c1).real*(*c2).image+(*c1).image*(*c2).real);
}
Complex* DivideComplex(const Complex *c1,const Complex *c2){
    double denominator=((*c2).real*(*c2).real+(*c2).image*(*c2).image);
    if((c1==NULL) || (c2==NULL)){
        fprintf(stderr,"Невозможно делить NULL-указатели\n");
        exit(1);
    }
    if (denominator==0){
        fprintf(stderr,"Невозможно делить на 0\n");
        exit(1);
    }
    return CreateComplex(((*c1).real*(*c2).real + (*c1).image*(*c2).image)/(denominator), ((*c1).image*(*c2).real - (*c1).real*(*c2).image)/(denominator));
}
Complex* SubtractComplex(const Complex *c1,const Complex *c2){
    if((c1==NULL) || (c2==NULL)){
        fprintf(stderr,"Невозможно вычитать NULL-указатели\n");
        exit(1);
    }
    return CreateComplex((*c1).real-(*c2).real, (*c1).image-(*c2).image);
}
void PrintComplex(const Complex *c1){
    if ((c1==NULL)){
        fprintf(stderr,"Невозможно вывести NULL-указатель\n");
        exit(1);
    }
    if ((*c1).image>=0){
        printf("%.2f + %.2fi\n",(*c1).real,(*c1).image);
    }
    else{
        printf("%.2f - %.2fi\n",(*c1).real,-1*((*c1).image));
    }
}
double GetReal(const Complex *c1){
    return (*c1).real;
}
double GetImage(const Complex *c1){
    return (*c1).image;
}
void SetReal(Complex *c1, double r){
    (*c1).real=r;
}
void SetImage(Complex *c1,double i){
    (*c1).image=i;
}
Complex* SetComplex(Complex *c1,double r,double i){
    (*c1).real=r;
    (*c1).image=i;
    return c1;
}










