#ifndef COMPLEX_H
#define COMPLEX_H

typedef struct {
    double real;
    double image;
} Complex;

Complex* CreateComplex(double real, double image);
void DestroyComplex(Complex* c);
Complex* AddComplex(const Complex* c1, const Complex* c2);
Complex* MultiplyComplex(const Complex* c1, const Complex* c2);
Complex* DivideComplex(const Complex* c1, const Complex* c2);
Complex* SubtractComplex(const Complex* c1, const Complex* c2);
Complex* ComplexScalarMul(const Complex* a, const double* scalar);
void PrintComplex(const Complex* c1);
double GetReal(const Complex* c1);
double GetImage(const Complex* c1);
void SetReal(Complex* c1, double r);
void SetImage(Complex* c1, double i);
Complex* SetComplex(Complex* c1, double r, double i);

#endif
