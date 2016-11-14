#ifndef MAT_FUNCS_INCLUDE
#define MAT_FUNCS_INCLUDE
void multiplyMatMM(float mat1[],float mat2[],int n,float result[]);
void translateMat(float m[],float x,float y,float z);
void rotateXMatrix(float angle,float m[]);
void rotateYMatrix(float angle,float m[]);
void rotateZMatrix(float angle,float m[]);
void initMat(float mat[]);
void multiplyMatMV(float mat[],float v[],int n);
void scaleMat(float m[],float sx,float sy,float sz);
void normalizeVector(float v[]);
void crossProduct(float v1[],float v2[],float fin[]);
void distance3D(float v1[],float v2[],float dist[]);
void copyMat(float Src[],float Dest[],int Size);
#endif // MAT_FUNCS_INCLUDE
