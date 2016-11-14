#include <math.h>
#include "mat_funcs.h"

void multiplyMatMM(float mat1[],float mat2[],int n,float result[]){
    int i,j,k;
    float temp[16];
    for(i=0;i<16;++i)
        temp[i]=0.0f;
    for(i=0;i<n;++i)
    {
        for(j=0;j<n;++j)
        {
            for(k=0;k<n;++k)
            {
                temp[i*n+k]+=mat1[j*n+k]*mat2[i*n+j];
            }
        }
    }
    for(i=0;i<n*n;i++)
        result[i]=temp[i];
}

void translateMat(float m[],float x,float y,float z){
    float mat[16];

    initMat(mat);
    mat[12]=x;mat[13]=y; mat[14]=z;

    multiplyMatMM(mat,m,4,m);
}

void scaleMat(float m[],float sx,float sy,float sz){
    float mat[16];

    initMat(mat);
    mat[0]=sx;mat[5]=sy; mat[10]=sz;

    multiplyMatMM(mat,m,4,m);
}

void rotateXMatrix(float angle,float m[]){
    float mat[16],c,s;
    angle=angle*M_PI/180.0f;
    c=cos(angle);
    s=sin(angle);

    initMat(mat);
    mat[5]=c;   mat[6]=s;
    mat[9]=-s;mat[10]=c;

    multiplyMatMM(mat,m,4,m);
}

void rotateYMatrix(float angle,float m[]){
    float mat[16],c,s;
    angle=angle*M_PI/180.0f;
    c=cos(angle);
    s=sin(angle);

    initMat(mat);
    mat[0]=c; mat[2]=-s;
    mat[8]=s; mat[10]=c;

    multiplyMatMM(mat,m,4,m);
}

void rotateZMatrix(float angle,float m[]){
    float mat[16],c,s;
    angle=angle*M_PI/180.0f;
    c=cos(angle);
    s=sin(angle);

    initMat(mat);
    mat[0]=c;   mat[1]=s;
    mat[4]=-s;mat[5]=c;

    multiplyMatMM(mat,m,4,m);
}

void initMat(float mat[]){
    mat[0]=1;  mat[1]=0;  mat[2]=0;  mat[3]=0;
    mat[4]=0;  mat[5]=1;  mat[6]=0;  mat[7]=0;
    mat[8]=0;  mat[9]=0;  mat[10]=1; mat[11]=0;
    mat[12]=0; mat[13]=0; mat[14]=0; mat[15]=1;
}

void multiplyMatMV(float mat[],float v[],int n){
    float temp[4];
    short i,j;
    for(i=0;i<n;++i)
    {
        temp[i]=0.0f;
        for(j=0;j<n;++j)
            temp[i]+=mat[j*n+i]*v[j];
    }
    for(i=0;i<n;++i)
        v[i]=temp[i];
}

void normalizeVector(float v[]){
    float mag;
    mag = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if(mag<0.0001)return;
    mag = (float)sqrt(mag);
    v[0]/=mag; v[1]/=mag; v[2]/=mag;
}

void crossProduct(float v1[],float v2[],float result[]){
    result[0]=v1[1]*v2[2]-v1[2]*v2[1];
    result[1]=v1[2]*v2[0]-v1[0]*v2[2];
    result[2]=v1[0]*v2[1]-v1[1]*v2[0];
}

void distance3D(float v1[],float v2[],float dist[]){
    dist[0]=v2[0]-v1[0];
    dist[1]=v2[1]-v1[1];
    dist[2]=v2[2]-v1[2];
    dist[3]=v2[3]-v1[3];
}

void copyMat(float Src[],float Dest[],int Size){
    for(Size=Size*Size-1;Size>=0;--Size)
        Dest[Size]=Src[Size];
}
