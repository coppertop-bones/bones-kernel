# 1 "mmul2.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "mmul2.c" 2
int joe();
int sally(int, void *);
int fred;
 scalarProduct(double *rowA, double **B, int mA, int j) {

    double answer; int k; double a; double b;
    answer = 0;
    for (k=0; k < mA; k++){
        answer = answer + rowA[k] * B[k][j];
    }
    return 0;
}

 mmul(double **A, double **B, int nA, int mA, int mB, double **out) {

    int i; int j;
    for (i=0; i < nA; i++) {
        for (j=0; j < mB; j++) {
            out[i][j] = scalarProduct(A[i], B, mA, j);
        }
    }
    return 0;
}

 mmul2(double **A, double **B, int nA, int mA, int mB, double **out) {

    int i; int j; int k; double *rowA;
    for (i=0; i < nA; i++) {
        for (j=0; j < mB; j++) {
            out[i][j] = 0;
            rowA = A[i];
            for (k=0; k < mA; k++){
                out[i][j] = out[i][j] + rowA[k] * B[k][j];
            }
        }
    }
    return 0;
}
