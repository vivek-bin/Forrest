#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <stdio.h>


class base_Cuboid_Shape{
private:
    GLuint Vertices_VboID,Colors_VboID,Normals_VboID,Indices_VboID,TexCoord_VboID,Height_VboID;
    short VertexCount;
    GLuint VaoID;

public:
    float ObjCentre[4];

    base_Cuboid_Shape(){
        ObjCentre[0]=ObjCentre[1]=ObjCentre[2]=ObjCentre[3]=0.0f;

        Height_VboID=TexCoord_VboID=Vertices_VboID=Indices_VboID=Colors_VboID=Normals_VboID=VaoID=0;
        VertexCount=0;
    }

    void createVao(){
        glGenVertexArrays(1,&VaoID);
        glBindVertexArray(0);
    }

    void createHeightBuffer(GLfloat Data[],GLuint DataSize){
        glBindVertexArray(VaoID);
        glGenBuffers(1, &Height_VboID);
        glBindBuffer(GL_ARRAY_BUFFER, Height_VboID);
        glBufferData(GL_ARRAY_BUFFER, DataSize, Data, GL_STATIC_DRAW);
        glVertexAttribPointer(0,1,GL_FLOAT,GL_FALSE,0,0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void createVertexBuffer(GLfloat Data[],GLuint DataSize){
        glBindVertexArray(VaoID);
        glGenBuffers(1, &Vertices_VboID);
        glBindBuffer(GL_ARRAY_BUFFER, Vertices_VboID);
        glBufferData(GL_ARRAY_BUFFER, DataSize, Data, GL_STATIC_DRAW);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void createColorBuffer(GLfloat Data[],GLuint DataSize){
        glBindVertexArray(VaoID);
        glGenBuffers(1, &Colors_VboID);
        glBindBuffer(GL_ARRAY_BUFFER, Colors_VboID);
        glBufferData(GL_ARRAY_BUFFER, DataSize, Data, GL_STATIC_DRAW);
        glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void createTexCoordBuffer(GLfloat Data[],GLuint DataSize){
        glBindVertexArray(VaoID);
        glGenBuffers(1, &TexCoord_VboID);
        glBindBuffer(GL_ARRAY_BUFFER, TexCoord_VboID);
        glBufferData(GL_ARRAY_BUFFER, DataSize, Data, GL_STATIC_DRAW);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void createNormalBuffer(GLfloat Data[],GLuint DataSize){
        glBindVertexArray(VaoID);
        glGenBuffers(1, &Normals_VboID);
        glBindBuffer(GL_ARRAY_BUFFER, Normals_VboID);
        glBufferData(GL_ARRAY_BUFFER, DataSize, Data, GL_STATIC_DRAW);
        glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,0);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void createIndexBuffer(GLushort Data[],GLuint DataSize){
        VertexCount=DataSize/sizeof(Data[0]);
        glGenBuffers(1, &Indices_VboID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indices_VboID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, DataSize, Data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    GLuint getVaoID(){
        return VaoID;
    }

    void drawElements(GLenum mode){
        glBindVertexArray(VaoID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indices_VboID);
        glDrawElements(mode, VertexCount, GL_UNSIGNED_SHORT, NULL);
        glBindVertexArray(0);
    }

    void destroyVboVao(){
        glBindVertexArray(VaoID);
        if(Height_VboID != 0)
        {
            glDisableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glDeleteBuffers(1,&Height_VboID);
        }
        if(Vertices_VboID != 0)
        {
            glDisableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glDeleteBuffers(1,&Vertices_VboID);
        }
        if(Colors_VboID != 0)
        {
            glDisableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glDeleteBuffers(1,&Colors_VboID);
        }
        if(TexCoord_VboID != 0)
        {
            glDisableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glDeleteBuffers(1,&TexCoord_VboID);
        }
        if(Normals_VboID != 0)
        {
            glDisableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glDeleteBuffers(1,&Normals_VboID);
        }
        if(Indices_VboID != 0)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
            glDeleteBuffers(1,&Indices_VboID);
        }

        glBindVertexArray(0);
        glDeleteVertexArrays(1,&VaoID);
    }
};

class shape_Object{
public:
    base_Cuboid_Shape *Shape;
    float ModelMat[16],ObjCentre[4];

    shape_Object(){
        ObjCentre[0]=ObjCentre[1]=ObjCentre[2]=ObjCentre[3]=0.0f;
        ModelMat[0]=1.0f;  ModelMat[1]=0.0f;  ModelMat[2]=0.0f;  ModelMat[3]=0.0f;
        ModelMat[4]=0.0f;  ModelMat[5]=1.0f;  ModelMat[6]=0.0f;  ModelMat[7]=0.0f;
        ModelMat[8]=0.0f;  ModelMat[9]=0.0f;  ModelMat[10]=1.0f; ModelMat[11]=0.0f;
        ModelMat[12]=0.0f; ModelMat[13]=0.0f; ModelMat[14]=0.0f; ModelMat[15]=1.0f;
        Shape=NULL;
    }
};

class shader_Prog{
private:
    GLuint VertexShaderID,FragmentShaderID,ProgramID;
    static GLuint CurrentProg;

public:

    shader_Prog(){
        VertexShaderID=FragmentShaderID=ProgramID=0;
    }

    static GLuint getCurrentProg(){
        return shader_Prog::CurrentProg;
    }


    GLuint getProgramID(){
        return ProgramID;
    }

    GLuint getVertexShaderID(){
        return VertexShaderID;
    }

    GLuint getFragmentShaderID(){
        return FragmentShaderID;
    }

    void createShaders(char const * const VertPrg,char const * const FragPrg){
        GLint success = 0;
        VertexShaderID=glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VertexShaderID,1,&VertPrg,NULL);
        glCompileShader(VertexShaderID);
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
            fprintf(stdout,"error compiling vertex shader");
        FragmentShaderID=glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FragmentShaderID,1,&FragPrg,NULL);
        glCompileShader(FragmentShaderID);

        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
            fprintf(stdout,"error compiling frag shader");
        ProgramID=glCreateProgram();
        glAttachShader(ProgramID,VertexShaderID);
        glAttachShader(ProgramID,FragmentShaderID);

        glLinkProgram(ProgramID);
    }

    void createShaders(const GLuint VertPrg,char const * const FragPrg){
        GLint success = 0;
        VertexShaderID=VertPrg;

        FragmentShaderID=glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FragmentShaderID,1,&FragPrg,NULL);
        glCompileShader(FragmentShaderID);
        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
            fprintf(stdout,"error compiling frag shader");

        ProgramID=glCreateProgram();
        glAttachShader(ProgramID,VertexShaderID);
        glAttachShader(ProgramID,FragmentShaderID);

        glLinkProgram(ProgramID);
    }

    void createShaders(char const * const VertPrg,const GLuint FragPrg){
        GLint success = 0;
        VertexShaderID=glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VertexShaderID,1,&VertPrg,NULL);
        glCompileShader(VertexShaderID);
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
            fprintf(stdout,"error compiling vertex shader");

        FragmentShaderID=FragPrg;

        ProgramID=glCreateProgram();
        glAttachShader(ProgramID,VertexShaderID);
        glAttachShader(ProgramID,FragmentShaderID);

        glLinkProgram(ProgramID);
    }

    void destroyShaders(){
        GLenum ErrorValue = glGetError();

        glUseProgram(0);

        glDetachShader(ProgramID,FragmentShaderID);
        glDetachShader(ProgramID,VertexShaderID);
        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        glDeleteProgram(ProgramID);

        ErrorValue=glGetError();
        if(ErrorValue != GL_NO_ERROR)
        {
            fprintf(stderr,"ERROR : %s",gluErrorString(ErrorValue));
        }
    }

    static void disableProgram(){
        if(CurrentProg!=0)
        {
            glUseProgram(0);
            CurrentProg=0;
        }
    }

    void useProgram(){
        if(CurrentProg!=ProgramID)
        {
            glUseProgram(ProgramID);
            CurrentProg=ProgramID;
        }
    }

    void useProgram(const char* UName,float P[]){
        if(CurrentProg!=ProgramID)
        {
            glUseProgram(ProgramID);
            CurrentProg=ProgramID;
            glUniformMatrix4fv(glGetUniformLocation(CurrentProg,UName),1,GL_FALSE,P);
        }
    }

};


