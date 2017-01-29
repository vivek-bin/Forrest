#define WINDOW_TITLE_PREFIX "Practice"
#define SHADERCODE(x) "#version 400\n" #x
#define SHADERCODEPRE(x) SHADERCODE(x)
#define RIGHT_KEY 0
#define LEFT_KEY 1
#define UP_KEY 2
#define DOWN_KEY 3
#define SPACE_KEY 4
#define TERRAIN_NUM_DIV_PTS 64
#define GROUND_SIZE 15.0f
#define NUMBER_OF_TREES 100
#define NUMBER_OF_ROCKS 40
#define LIGHT_POSITION 100.0, 100.0, 0.0

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "mat_funcs.h"
#include "cuboidClass.cpp"
GLuint shader_Prog::CurrentProg;

int CurrentWidth = 1000, CurrentHeight = 700, WindowHandle = 0;
unsigned FrameCount = 0;
GLfloat ViewMat[16],ProjMat[16],Camera[4],Centre[4],Up[4];
GLubyte ArrowKeys[5];
GLuint TreeTex,GrassTex,RockTex,FenceTex;

shader_Prog SimpleShader,TextureShader,TerrainShader;

base_Cuboid_Shape Tent,Tree,Rock,Ground,Fence;
shape_Object TreeObj[NUMBER_OF_TREES],RockObj[NUMBER_OF_ROCKS],TentObj[5],FenceObj[5],GroundObj;

GLfloat HeightMap[TERRAIN_NUM_DIV_PTS*TERRAIN_NUM_DIV_PTS];


const GLchar* SimpleVertShader = { SHADERCODEPRE(
 layout(location=0) in vec4 in_Position;
 layout(location=1) in vec4 in_Color;
 uniform mat4 VM;
 uniform mat4 P;
 out vec4 ex_Color;
 void main()
 {
     ex_Color=in_Color;
     gl_Position=P*VM*in_Position;
 }
)};

const GLchar* SimpleFragShader = { SHADERCODEPRE(
 in vec4 ex_Color;
 out vec4 out_Color;

 void main()
 {
     out_Color = ex_Color;
 }
)};

const GLchar* TexVertShader = { SHADERCODEPRE(
 layout(location=0) in vec4 in_Position;
 layout(location=1) in vec2 vt;
 layout(location=2) in vec3 vc_Normal;
 uniform mat4 V;
 uniform mat4 M;
 uniform mat4 P;
 out vec2 texture_coordinates;
 out vec3 fg_Normal;
 out vec4 fg_Position;
 void main()
 {
     fg_Normal=normalize(vec3(M*vec4(vc_Normal,0.0f)));
     texture_coordinates=vt;
     fg_Position=M*in_Position;
     gl_Position=P*V*fg_Position;
 }
)};

const GLchar* TexFragShader = { SHADERCODEPRE(
 in vec2 texture_coordinates;
 out vec4 frag_colour;
 uniform mat4 V;
 uniform sampler2D basic_texture;
 uniform vec4 transparent_colour=vec4(1.0f,1.0f,1.0f,1.0f);
 in vec3 fg_Normal;
 in vec4 fg_Position;

 void main () {
     frag_colour = texture(basic_texture,texture_coordinates);
     if(frag_colour == transparent_colour)
        discard;

     vec3 light_position_world = vec3 (LIGHT_POSITION);
     vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
     vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
     vec3 La = vec3 (0.3, 0.3, 0.2); // grey ambient colour
     // surface reflectance
     vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
     vec3 Kd = vec3 (1.0, 0.5, 0.0); // orange diffuse surface reflectance
     vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
     float specular_exponent = 100.0; // specular 'power'
     // ambient intensity
     vec3 Ia = La * Ka;
     // diffuse intensity
     vec3 direction_to_light = normalize ((light_position_world.xyz - fg_Position.xyz));
     vec3 Id = Ld * Kd * max(dot(direction_to_light, fg_Normal),0.0); // final diffuse intensity
     frag_colour *= vec4 (Id + Ia, 1.0);
 }
)};

const GLchar* HeightVertShader = { SHADERCODEPRE(
 layout(location=0) in float vc_Height;
 layout(location=2) in vec3 vc_Normal;
 uniform mat4 P;
 uniform mat4 V;
 uniform mat4 M;
 out vec2 texture_coordinates;
 out vec3 fg_Normal;
 out vec4 fg_Position;
 void main()
 {
     float r=2.0f*(gl_VertexID%TERRAIN_NUM_DIV_PTS)/(TERRAIN_NUM_DIV_PTS-1.0f) - 1.0f;
     float c=2.0f*(gl_VertexID/TERRAIN_NUM_DIV_PTS)/(TERRAIN_NUM_DIV_PTS-1.0f) - 1.0f;

     fg_Normal=normalize(vec3(M*vec4(vc_Normal,0.0f)));
     fg_Position=M*vec4(r,
                          //sin(r*10)*tan(c*10)*2,
                          vc_Height,
                          c,1.0f);
     texture_coordinates=vec2(r*5,c*5);
     gl_Position=P*V*fg_Position;
 }
)};

const GLchar* HeightFragShader = { SHADERCODEPRE(
 in vec2 texture_coordinates;
 out vec4 frag_colour;
 uniform mat4 V;
 uniform sampler2D basic_texture;
 uniform vec4 transparent_colour=vec4(1.0f,1.0f,1.0f,1.0f);
 in vec3 fg_Normal;
 in vec4 fg_Position;

 void main () {
     frag_colour = texture(basic_texture,texture_coordinates);
     if(frag_colour == transparent_colour)
        discard;

     vec3 light_position_world = vec3 (LIGHT_POSITION);
     vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
     vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
     vec3 La = vec3 (0.3, 0.3, 0.2); // grey ambient colour
     // surface reflectance
     vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
     vec3 Kd = vec3 (1.0, 0.5, 0.0); // orange diffuse surface reflectance
     vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
     float specular_exponent = 100.0; // specular 'power'
     // ambient intensity
     vec3 Ia = La * Ka;
     // diffuse intensity
     vec3 direction_to_light = normalize ((light_position_world.xyz - fg_Position.xyz));
     vec3 Id = Ld * Kd * max(dot(direction_to_light, fg_Normal),0.0); // final diffuse intensity
     frag_colour *= vec4 (Id + Ia, 1.0);
 }
)};

void initialize(int ,char* []);
void initWindow(int ,char* []);
void initializeData();
void timerFunction(int );
void idleFunction();
void renderFunction();
void resizeFunction(int ,int );
void cleanUp();
void updateViewData();
void createProjMat();
void arrowKeyDown(int ,int ,int );
void arrowKeyUp(int ,int ,int );
void createTent();
void createTree();
void createRock();
void createGround();
void createFence();
GLuint loadTexture(const char *,const GLuint,const GLuint);
float pointHeight(float ,float );
void loadHeightMap();
void genHeightMapNormals(float []);
void createHeightMap();
void genNormals(GLfloat const [],int ,GLushort const [],int ,GLfloat []);
float areaHeight(float,float,float);
void removeOverlaping();

int main(int argc,char* argv[]){

    srand(time(NULL));

    initialize(argc,argv);

    glutMainLoop();

    exit(EXIT_SUCCESS);
}

void initialize(int argc,char* argv[]){
    GLenum GlewErrorCheck;
    glewExperimental=GL_TRUE;
    initWindow(argc,argv);
    GlewErrorCheck=glewInit();
    if(GLEW_OK!=GlewErrorCheck)
    {
        fprintf(stderr,"ERROR:%s",glewGetErrorString(GlewErrorCheck));
        exit(EXIT_FAILURE);
    }

    fprintf(stdout,"OpenGL Version : %s",glGetString(GL_VERSION));
    initializeData();
    updateViewData();
    createProjMat();

    createGround();
    createTree();
    createTent();
    createRock();
    createFence();
    removeOverlaping();
    glEnable(GL_TEXTURE_2D);

    //SimpleShader.createShaders(SimpleVertShader,SimpleFragShader);
    TextureShader.createShaders(TexVertShader,TexFragShader);
    TerrainShader.createShaders(HeightVertShader,HeightFragShader);
    TreeTex=loadTexture("./images/tree.bmp",GL_CLAMP_TO_EDGE,GL_REPEAT);
    GrassTex=loadTexture("./images/grass.bmp",GL_REPEAT,GL_REPEAT);
    RockTex=loadTexture("./images/rock.bmp",GL_REPEAT,GL_REPEAT);
    FenceTex=loadTexture("./images/fence.bmp",GL_REPEAT,GL_CLAMP_TO_EDGE);
    glClearColor(0.68f,0.68f,1.0f,1.0f);
}

void createGround(){
    unsigned int i,j;
    GLushort Indices[2*(TERRAIN_NUM_DIV_PTS+1)*(TERRAIN_NUM_DIV_PTS-1)];
    GLfloat Normals[3*TERRAIN_NUM_DIV_PTS*TERRAIN_NUM_DIV_PTS];

    Indices[0]=0;
    for(i=0;i<TERRAIN_NUM_DIV_PTS;++i){
        Indices[2*i+1]=i;
        Indices[2*i+2]=i+TERRAIN_NUM_DIV_PTS;
    }
    Indices[(2*(TERRAIN_NUM_DIV_PTS+1))-1]=(2*TERRAIN_NUM_DIV_PTS)-1;
    for(i=2*(TERRAIN_NUM_DIV_PTS+1),j=0;
        i<(2*(TERRAIN_NUM_DIV_PTS+1)*(TERRAIN_NUM_DIV_PTS-1));
        ++i,++j)
    {   Indices[i]=Indices[j]+TERRAIN_NUM_DIV_PTS;   }

    GroundObj.Shape=&Ground;
    scaleMat(GroundObj.ModelMat,GROUND_SIZE,1.0f,GROUND_SIZE);
    genHeightMapNormals(Normals);
    Ground.createVao();
    Ground.createHeightBuffer(HeightMap,sizeof(HeightMap));
    Ground.createNormalBuffer(Normals,sizeof(Normals));
    Ground.createIndexBuffer(Indices, sizeof(Indices));
    glBindVertexArray(0);

}

void createFence(){
    GLfloat Vertices[]={
        -1.0f,-1.0f, 0.0f,1.0f,
        -1.0f, 1.0f, 0.0f,1.0f,
         1.0f,-1.0f, 0.0f,1.0f,
         1.0f, 1.0f, 0.0f,1.0f
  };
    GLushort Indices[]={
        2,1,0,
        1,2,3
    };
    GLfloat TexCoord[]={
        0.0f,1.0f,
        0.0f,0.0f,
        1.5f,1.0f,
        1.5f,0.0f
    };
    TexCoord[4]*=GROUND_SIZE;TexCoord[6]=TexCoord[4];

    GLfloat Normals[((sizeof(Vertices)/sizeof(GLfloat))*3)/4];

    genNormals(Vertices,sizeof(Vertices),Indices,sizeof(Indices),Normals);

    Fence.createVao();
    Fence.createVertexBuffer(Vertices, sizeof(Vertices));
    Fence.createTexCoordBuffer(TexCoord,sizeof(TexCoord));
    Fence.createNormalBuffer(Normals,sizeof(Normals));
    Fence.createIndexBuffer(Indices, sizeof(Indices));
    glBindVertexArray(0);

    FenceObj[0].Shape=&Fence;
    scaleMat(FenceObj[0].ModelMat,GROUND_SIZE,0.4f,0.1f);
    translateMat(FenceObj[0].ModelMat,0.0f,0.4f,-GROUND_SIZE);

    FenceObj[1].Shape=&Fence;
    scaleMat(FenceObj[1].ModelMat,GROUND_SIZE,0.4f,0.1f);
    translateMat(FenceObj[1].ModelMat,0.0f,0.4f,GROUND_SIZE);

    FenceObj[2].Shape=&Fence;
    scaleMat(FenceObj[2].ModelMat,GROUND_SIZE,0.4f,0.1f);
    rotateYMatrix(-90.0f,FenceObj[2].ModelMat);
    translateMat(FenceObj[2].ModelMat,GROUND_SIZE,0.4f,0.0f);

    FenceObj[3].Shape=&Fence;
    scaleMat(FenceObj[3].ModelMat,GROUND_SIZE,0.4f,0.1f);
    rotateYMatrix(90.0f,FenceObj[3].ModelMat);
    translateMat(FenceObj[3].ModelMat,-GROUND_SIZE,0.4f,0.0f);

}

void createTree(){

    GLfloat Vertices[]={
        -0.8f, -1.0f,0.0f, 1.0f,
        -0.57f,-1.0f,0.57f,1.0f,
        -0.48f, 0.6f,0.0f, 1.0f,
        -0.36f, 0.6f,0.36f,1.0f,
         0.0f,  1.0f,0.0f, 1.0f,


        -0.05f,-0.6f, 0.8f,1.0f,
        -0.05f,-0.35f,0.0f, 1.0f,
        -7.5f, -0.7f, 0.0f, 1.0f,
        -0.05f,-0.6f,-0.8f,1.0f,

        -0.05f,-0.6f+0.25f, 0.8f,1.0f,
        -0.05f,-0.35f+0.25f,0.0f, 1.0f,
        -6.8f, -0.7f+0.25f, 0.0f, 1.0f,
        -0.05f,-0.6f+0.25f,-0.8f,1.0f,

        -0.05f,-0.6f+0.5f, 0.8f,1.0f,
        -0.05f,-0.35f+0.5f,0.0f, 1.0f,
        -6.1f, -0.7f+0.5f, 0.0f, 1.0f,
        -0.05f,-0.6f+0.5f,-0.8f,1.0f,

        -0.05f,-0.6f+0.75f, 0.8f,1.0f,
        -0.05f,-0.35f+0.75f,0.0f, 1.0f,
        -5.4f, -0.7f+0.75f, 0.0f, 1.0f,
        -0.05f,-0.6f+0.75f,-0.8f,1.0f,

        -0.05f,-0.6f+1.0f, 0.8f,1.0f,
        -0.05f,-0.35f+1.0f,0.0f, 1.0f,
        -4.7f, -0.7f+1.0f, 0.0f, 1.0f,
        -0.05f,-0.6f+1.0f,-0.8f,1.0f,

        -0.05f,-0.6f+1.25f, 0.8f,1.0f,
        -0.05f,-0.35f+1.25f,0.0f, 1.0f,
        -4.0f, -0.7f+1.25f, 0.0f, 1.0f,
        -0.05f,-0.6f+1.25f,-0.8f,1.0f
    };

    GLushort Indices[]={
    0,1,2,
    1,3,2,
    2,3,4,

    5,6,7,
    6,7,8,

    9,10,11,
    10,11,12,

    13,14,15,
    14,15,16,

    17,18,19,
    18,19,20,

    21,22,23,
    22,23,24,

    25,26,27,
    26,27,28
    };

    GLfloat TexCoord[]={
    0.95f,0.0f,
    1.0f,0.0f,
    0.95f,4.0f,
    1.0f,4.0f,
    0.975f,0.5f,

    0.0f,0.125f,
    0.25f,0.0f,
    0.25f,0.7f,
    0.5f,0.125f,

    0.0f,0.125f,
    0.25f,0.0f,
    0.25f,0.7f,
    0.5f,0.125f,

    0.0f,0.125f,
    0.25f,0.0f,
    0.25f,0.7f,
    0.5f,0.125f,

    0.0f,0.125f,
    0.25f,0.0f,
    0.25f,0.7f,
    0.5f,0.125f,

    0.0f,0.125f,
    0.25f,0.0f,
    0.25f,0.7f,
    0.5f,0.125f,

    0.0f,0.125f,
    0.25f,0.0f,
    0.25f,0.7f,
    0.5f,0.125f
    };

    GLfloat Normals[((sizeof(Vertices)/sizeof(GLfloat))*3)/4];

    genNormals(Vertices,sizeof(Vertices),Indices,sizeof(Indices),Normals);

    Tree.createVao();
    Tree.createVertexBuffer(Vertices,sizeof(Vertices));
    Tree.createTexCoordBuffer(TexCoord,sizeof(TexCoord));
    Tree.createNormalBuffer(Normals,sizeof(Normals));
    Tree.createIndexBuffer(Indices,sizeof(Indices));
    glBindVertexArray(0);

    for(int i=0;i<NUMBER_OF_TREES;++i){
        float x,z,scale;
        TreeObj[i].Shape=&Tree;
        scale=(rand()&0x1f)-0x0f;
        scaleMat(TreeObj[i].ModelMat,0.075f+scale*0.001,1.0f+scale*0.01,0.075f+scale*0.001);
        x=((rand()&0xff)-0x7f)/10.0f;
        z=((rand()&0xff)-0x7f)/10.0f;
        translateMat(TreeObj[i].ModelMat,x,0.8f+scale*0.01+pointHeight(x,z),z);
    }
}

void createRock(){
    GLfloat Vertices[]={
        -1.0f,-1.0f, 0.0f,1.0f,
        -0.4f,-1.0f, 1.0f,1.0f,
         0.4f,-1.0f, 1.0f,1.0f,
         1.0f,-1.0f, 0.0f,1.0f,
         0.3f,-1.0f,-0.7f,1.0f,
         0.0f,-1.0f,-1.0f,1.0f,
        -0.3f,-1.0f,-0.7f,1.0f,

        -0.6f, 0.8f, 0.4f,1.0f,
         0.0f, 1.0f, 0.6f,1.0f,
         0.7f, 0.8f, 0.0f,1.0f,
         0.0f, 1.0f,-0.5f,1.0f,
        -0.6f, 0.9f,-0.4f,1.0f
    };

    GLushort Indices[]={
        11,7,0,
        0,6,11,
        6,10,11,
        6,5,10,
        10,5,4,
        4,9,10,
        3,9,4,
        9,3,2,
        2,8,9,
        8,2,1,
        1,7,8,
        7,1,0,
        11,8,7,
        11,9,8,
        11,10,9
    };

    GLfloat TexCoord[]={
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,0.0f,
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,1.0f,
        1.0f,0.0f,

        1.0f,0.0f,
        0.0f,0.0f,
        1.0f,1.0f,
        0.0f,0.0f,
        0.0f,1.0f,
    };

    GLfloat Normals[((sizeof(Vertices)/sizeof(GLfloat))*3)/4];

    genNormals(Vertices,sizeof(Vertices),Indices,sizeof(Indices),Normals);

    Rock.createVao();
    Rock.createVertexBuffer(Vertices, sizeof(Vertices));
    Rock.createTexCoordBuffer(TexCoord,sizeof(TexCoord));
    Rock.createNormalBuffer(Normals,sizeof(Normals));
    Rock.createIndexBuffer(Indices, sizeof(Indices));
    glBindVertexArray(0);

    for(int i=0;i<NUMBER_OF_ROCKS;++i){
        float x,z,scale,xzscale,yscale;
        RockObj[i].Shape=&Rock;
        scale=(rand()&0x1f)-0x0f;
        xzscale=0.6f+scale*0.04f;
        yscale=0.3f+scale*0.015f;
        scaleMat(RockObj[i].ModelMat,xzscale,yscale,xzscale);
        x=((rand()&0xff)-0x7f)/10.0f;
        z=((rand()&0xff)-0x7f)/10.0f;
        translateMat(RockObj[i].ModelMat,x,yscale+areaHeight(x,z,xzscale)-0.1f,z);
    }
}

void createTent(){
    GLfloat Vertices[]={
        -1.0f,-1.0f,1.0f,1.0f,
        -1.0f,1.0f,0.0f,1.0f,
        1.0f,1.0f,0.0f,1.0f,
        1.0f,-1.0f,1.0f,1.0f,
        -1.0f,-1.0f,-1.0f,1.0f,
        1.0f,-1.0f,-1.0f,1.0f
    };
    GLushort Indices[]={
        3,1,0,
        3,2,1,
        1,2,4,
        5,4,2
    };
    GLfloat TexCoord[]={
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,1.0f,
        1.0f,0.0f,
        0.0f,0.0f,
        1.0f,0.0f
    };

    GLfloat Normals[((sizeof(Vertices)/sizeof(GLfloat))*3)/4];
    genNormals(Vertices,sizeof(Vertices),Indices,sizeof(Indices),Normals);

    Tent.createVao();
    Tent.createVertexBuffer(Vertices, sizeof(Vertices));
    Tent.createTexCoordBuffer(TexCoord,sizeof(TexCoord));
    Tent.createNormalBuffer(Normals,sizeof(Normals));
    Tent.createIndexBuffer(Indices, sizeof(Indices));
    glBindVertexArray(0);
    TentObj[0].Shape=&Tent;
    scaleMat(TentObj[0].ModelMat,0.6f,0.3f,0.4f);
    translateMat(TentObj[0].ModelMat,0.0f,0.3f,0.0f);
}

void initWindow(int argc,char* argv[]){
    glutInit(&argc,argv);
    glutInitContextVersion(4,0);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glutInitWindowSize(CurrentWidth,CurrentHeight);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE);

    WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX);

    if(WindowHandle < 1)
    {
        fprintf(stderr, "UNABLE TO CREATE WINDOW");
        exit(-1);
    }

    glEnable(GL_DEPTH_TEST);

    glutSpecialFunc(arrowKeyDown);
    glutSpecialUpFunc(arrowKeyUp);
    glutIdleFunc(idleFunction);
    glutDisplayFunc(renderFunction);
    glutReshapeFunc(resizeFunction);
    glutCloseFunc(cleanUp);
    glutTimerFunc(0,timerFunction,0);
}

void keyCheck(){
    float temp[16];
    float RotateSpeed,MoveSpeed;
    RotateSpeed=3.0f;
    MoveSpeed=0.1f;
    if(ArrowKeys[0]==1)
    {
        initMat(temp);
        translateMat(temp,-Camera[0],-Camera[1],-Camera[2]);
        rotateYMatrix(-RotateSpeed,temp);
        translateMat(temp,Camera[0],Camera[1],Camera[2]);

        multiplyMatMV(temp,Centre,4);
    }
    if(ArrowKeys[1]==1)
    {
        initMat(temp);
        translateMat(temp,-Camera[0],-Camera[1],-Camera[2]);
        rotateYMatrix(RotateSpeed,temp);
        translateMat(temp,Camera[0],Camera[1],Camera[2]);

        multiplyMatMV(temp,Centre,4);
    }
    if(ArrowKeys[2]==1)
    {
        distance3D(Camera,Centre,temp);
        normalizeVector(temp);
        Camera[0]+=(temp[0]*MoveSpeed);
        Camera[1]+=(temp[1]*MoveSpeed);
        Camera[2]+=(temp[2]*MoveSpeed);
        Centre[0]+=(temp[0]*MoveSpeed);
        Centre[1]+=(temp[1]*MoveSpeed);
        Centre[2]+=(temp[2]*MoveSpeed);
        Camera[1]=Centre[1]=0.5f+pointHeight(Camera[0],Camera[2])*1.3f;
    }
    if(ArrowKeys[3]==1)
    {
        distance3D(Camera,Centre,temp);
        normalizeVector(temp);
        Camera[0]-=(temp[0]*MoveSpeed);
        Camera[1]-=(temp[1]*MoveSpeed);
        Camera[2]-=(temp[2]*MoveSpeed);
        Centre[0]-=(temp[0]*MoveSpeed);
        Centre[1]-=(temp[1]*MoveSpeed);
        Centre[2]-=(temp[2]*MoveSpeed);
        Camera[1]=Centre[1]=0.5f+pointHeight(Camera[0],Camera[2])*1.3f;
    }
    updateViewData();
}

void arrowKeyUp(int key,int x,int y){
    if(key == GLUT_KEY_RIGHT)
        {ArrowKeys[0]=0;}
    else if(key == GLUT_KEY_LEFT)
        {ArrowKeys[1]=0;}
    else if(key == GLUT_KEY_UP)
        {ArrowKeys[2]=0;}
    else if(key == GLUT_KEY_DOWN)
        {ArrowKeys[3]=0;}
}

void arrowKeyDown(int key,int x,int y){
    if(key == GLUT_KEY_RIGHT)
        {ArrowKeys[0]=1;ArrowKeys[1]+=0;}
    else if(key == GLUT_KEY_LEFT)
        {ArrowKeys[1]=1;ArrowKeys[0]+=0;}
    else if(key == GLUT_KEY_UP)
        {ArrowKeys[2]=1;ArrowKeys[3]+=0;}
    else if(key == GLUT_KEY_DOWN)
        {ArrowKeys[3]=1;ArrowKeys[2]+=0;}
}

void idleFunction(){
    //glutPostRedisplay();
}

void renderFunction(){
    GLint MLocation;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if(GroundObj.Shape!=NULL){
        TerrainShader.useProgram("P",ProjMat);
        glBindTexture (GL_TEXTURE_2D, GrassTex);
        glUniformMatrix4fv(glGetUniformLocation(shader_Prog::getCurrentProg(),"V"),1,GL_FALSE,ViewMat);
        glUniformMatrix4fv(glGetUniformLocation(shader_Prog::getCurrentProg(),"M"),1,GL_FALSE,GroundObj.ModelMat);
        GroundObj.Shape->drawElements(GL_TRIANGLE_STRIP);
    }

    TextureShader.useProgram("P",ProjMat);

    glUniformMatrix4fv(glGetUniformLocation(shader_Prog::getCurrentProg(),"V"),1,GL_FALSE,ViewMat);
    MLocation=glGetUniformLocation(shader_Prog::getCurrentProg(),"M");
    glBindTexture (GL_TEXTURE_2D, FenceTex);
    for(int i=0;i<5;i++){
        if(FenceObj[i].Shape!=NULL){
            glUniformMatrix4fv(MLocation,1,GL_FALSE,FenceObj[i].ModelMat);
            FenceObj[i].Shape->drawElements(GL_TRIANGLES);
        }
    }
    glBindTexture(GL_TEXTURE_2D,RockTex);
    //MLocation=glGetUniformLocation(shader_Prog::getCurrentProg(),"M");
    for(int i=0;i<NUMBER_OF_ROCKS;++i){
        if(RockObj[i].Shape!=NULL){
            glUniformMatrix4fv(MLocation,1,GL_FALSE,RockObj[i].ModelMat);
            RockObj[i].Shape->drawElements(GL_TRIANGLES);
        }
    }

    //TextureShader.useProgram("P",ProjMat);
    //MLocation=glGetUniformLocation(shader_Prog::getCurrentProg(),"M");
    glBindTexture (GL_TEXTURE_2D, TreeTex);
    for(int i=0;i<NUMBER_OF_TREES;i++){
        if(TreeObj[i].Shape!=NULL){
            for(int b=0;b<15;b++){
                glUniformMatrix4fv(MLocation,1,GL_FALSE,TreeObj[i].ModelMat);
                TreeObj[i].Shape->drawElements(GL_TRIANGLES);
                //rotate piece about centre of tree
                float TempX=TreeObj[i].ModelMat[12];
                float TempZ=TreeObj[i].ModelMat[14];
                translateMat(TreeObj[i].ModelMat,-TempX,0,-TempZ);
                rotateYMatrix(360/15,TreeObj[i].ModelMat);
                translateMat(TreeObj[i].ModelMat,TempX,0,TempZ);
            }
        }
    }
    glutSwapBuffers();

    shader_Prog::disableProgram();
    ++FrameCount;
}

void removeOverlaping(){
    for(int i=0;i<NUMBER_OF_TREES;++i){
        for(int j=0;j<NUMBER_OF_ROCKS;++j){
            float *TMat=TreeObj[i].ModelMat;
            float *RMat=RockObj[j].ModelMat;
            float distsq,rt,rr;
            distsq=((RMat[12]-TMat[12])*(RMat[12]-TMat[12]))+((RMat[14]-TMat[14])*(RMat[14]-TMat[14]));
            rt=(TMat[0]+TMat[10])/2;
            rr=(RMat[0]+RMat[10])/2;

            if(distsq<rt*rt+rr*rr){
                initMat(TMat);
                TMat[0]=TMat[5]=TMat[10]=TMat[15]=0.0f;
                break;
            }
        }
    }
}

void resizeFunction(int Width, int Height){
    CurrentWidth=Width;
    CurrentHeight=Height;
    glViewport(0,0,CurrentWidth,CurrentHeight);
}

void cleanUp(){
    SimpleShader.destroyShaders();
    Tent.destroyVboVao();
    Tree.destroyVboVao();
    Rock.destroyVboVao();
    Ground.destroyVboVao();
    Fence.destroyVboVao();
}

void timerFunction(int n){
    if(n==1)
    {
        char temp[120];
        sprintf(temp,"%s %d x %d  %d FPS",WINDOW_TITLE_PREFIX,
                CurrentWidth,CurrentHeight,FrameCount*20);
        glutSetWindowTitle(temp);

    }
    keyCheck();
    glutTimerFunc(50,timerFunction,1);
    FrameCount=0;
    glutPostRedisplay();
}

void initializeData(){
    initMat(ViewMat);
    initMat(ProjMat);
    ArrowKeys[0]=ArrowKeys[1]=ArrowKeys[2]=ArrowKeys[3]=0;
    Camera[0]=0.0f;Camera[1]=0.5f;Camera[2]=0.7f;Camera[3]=1.0f;
    Centre[0]=0.0f;Centre[1]=0.5f;Centre[2]=0.0f;Centre[3]=1.0f;
    Up[0]=0.0f;    Up[1]=1.0f;    Up[2]=0.0f;    Up[3]=0.0f;
    loadHeightMap();
}

void updateViewData(){
    float R[16],T[16],TempForward[4],TempRight[4];

    normalizeVector(Up);
    initMat(T);
    T[12]=-Camera[0]; T[13]=-Camera[1]; T[14]=-Camera[2];

    distance3D(Camera,Centre,TempForward);
    normalizeVector(TempForward);
    crossProduct(TempForward,Up,TempRight);
    normalizeVector(TempRight);

    initMat(R);
    R[0]=TempRight[0];R[1]=Up[0];R[2]=-TempForward[0];
    R[4]=TempRight[1];R[5]=Up[1];R[6]=-TempForward[1];
    R[8]=TempRight[2];R[9]=Up[2];R[10]=-TempForward[2];

    initMat(ViewMat);
    multiplyMatMM(T,ViewMat,4,ViewMat);
    multiplyMatMM(R,ViewMat,4,ViewMat);
}

void createProjMat(){
    //projection
    float Near,Far,Fov,Aspect,Range,Sx,Sy,Sz,Pz;
    Near=0.1f; // clipping plane
    Far = 100.0f; // clipping plane
    Fov = 67.0f * M_PI/180.0f; // convert 67 degrees to radians
    Aspect = (float)CurrentWidth / (float)CurrentHeight; // aspect ratio
    // matrix components
    Range = tan (Fov * 0.5f) * Near;
    Sx = Near / (Range * Aspect);
    Sy = Near / Range;
    Sz = -(Far + Near) / (Far - Near);
    Pz = -(2.0f * Far * Near) / (Far - Near);

    initMat(ProjMat);
    ProjMat[0]=Sx;
    ProjMat[5]=Sy;
    ProjMat[10]=Sz;
    ProjMat[11]=-1;
    ProjMat[14]=Pz;
    ProjMat[15]=0;
}

GLuint loadTexture(const char * path,const GLuint Params,const GLuint Paramt){
    GLuint tex=0;
    int x,y,comp;
    unsigned char* image;
    image = stbi_load(path,&x,&y,&comp,STBI_rgb_alpha);
    if(!image){
        fprintf(stdout,"error loading image");
    }
    glGenTextures (1, &tex);
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, tex);
    glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,x,y,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Params);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Paramt);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    STBI_FREE(image);
    return tex;
}

void loadHeightMap(){
    std::ifstream HMfile("./data/heightmap.txt");

    for(int i=0; HMfile>>HeightMap[i]; ++i);

    HMfile.close();
}

float pointHeight(float x,float y){
    int r,c;
    float t1,t2,h;
    x+=GROUND_SIZE;
    x*=TERRAIN_NUM_DIV_PTS/(2*GROUND_SIZE);
    y+=GROUND_SIZE;
    y*=TERRAIN_NUM_DIV_PTS/(2*GROUND_SIZE);
    r=(int)x;
    c=(int)y;
    x-=r;y-=c;
    t1=(1.0f-x)*HeightMap[c*TERRAIN_NUM_DIV_PTS+r]+x*HeightMap[c*TERRAIN_NUM_DIV_PTS+r+1];
    t2=(1.0f-x)*HeightMap[(c+1)*TERRAIN_NUM_DIV_PTS+r]+x*HeightMap[(c+1)*TERRAIN_NUM_DIV_PTS+r+1];
    h=(1.0f-y)*t1+y*t2;
    return h;
}

float areaHeight(float x,float y,float radius){
    int i,j,r,num;
    float minimum,average;
    minimum=9999.9f;
    average=0.0f;
    num=0;
    r=(radius*TERRAIN_NUM_DIV_PTS/(2*GROUND_SIZE));
    x+=GROUND_SIZE;
    x*=TERRAIN_NUM_DIV_PTS/(2*GROUND_SIZE);
    y+=GROUND_SIZE;
    y*=TERRAIN_NUM_DIV_PTS/(2*GROUND_SIZE);
    i=(int)x;
    j=(int)y;

    for(int a=r;a>0;--a){
        float h;
        h=HeightMap[(j+r-a)*TERRAIN_NUM_DIV_PTS+i+a];
        if(h<minimum)minimum=h;
        ++num;average+=h;

        h=HeightMap[(j+a)*TERRAIN_NUM_DIV_PTS+i+a-r];
        if(h<minimum)minimum=h;
        ++num;average+=h;

        h=HeightMap[(j+a-r)*TERRAIN_NUM_DIV_PTS+i-a];
        if(h<minimum)minimum=h;
        ++num;average+=h;

        h=HeightMap[(j-a)*TERRAIN_NUM_DIV_PTS+i+r-a];
        if(h<minimum)minimum=h;
        ++num;average+=h;
    }
    average/=num;
    return minimum*0.8f+average*0.2f;
}

void genHeightMapNormals(float NormalMat[]){
    int i,j;
    float Dir[8][4];
    float temp[4],N[4];

    for(i=0;i<TERRAIN_NUM_DIV_PTS;i++){
        for(j=0;j<TERRAIN_NUM_DIV_PTS;j++){
            N[0]=0.0f;N[1]=0.0f;N[2]=0.0f;N[3]=0.0f;
            Dir[0][0]=-1.0f;Dir[0][2]= 0.0f;Dir[0][3]= 0.0f;
            Dir[1][0]=-1.0f;Dir[1][2]= 1.0f;Dir[1][3]= 0.0f;
            Dir[2][0]= 0.0f;Dir[2][2]= 1.0f;Dir[2][3]= 0.0f;
            Dir[3][0]= 1.0f;Dir[3][2]= 1.0f;Dir[3][3]= 0.0f;
            Dir[4][0]= 1.0f;Dir[4][2]= 0.0f;Dir[4][3]= 0.0f;
            Dir[5][0]= 1.0f;Dir[5][2]=-1.0f;Dir[5][3]= 0.0f;
            Dir[6][0]= 0.0f;Dir[6][2]=-1.0f;Dir[6][3]= 0.0f;
            Dir[7][0]=-0.0f;Dir[7][2]=-1.0f;Dir[7][3]= 0.0f;

            Dir[0][1]=HeightMap[(i==0?0:i-1)*TERRAIN_NUM_DIV_PTS+j];
            Dir[1][1]=HeightMap[(i==0?0:i-1)*TERRAIN_NUM_DIV_PTS+(j==0?0:j-1)];
            Dir[2][1]=HeightMap[i*TERRAIN_NUM_DIV_PTS+(j==0?0:j-1)];
            Dir[3][1]=HeightMap[(i==TERRAIN_NUM_DIV_PTS-1?TERRAIN_NUM_DIV_PTS-1:i+1)*TERRAIN_NUM_DIV_PTS+(j==0?0:j-1)];
            Dir[4][1]=HeightMap[(i==TERRAIN_NUM_DIV_PTS-1?TERRAIN_NUM_DIV_PTS-1:i+1)*TERRAIN_NUM_DIV_PTS+j];
            Dir[5][1]=HeightMap[(i==TERRAIN_NUM_DIV_PTS-1?TERRAIN_NUM_DIV_PTS-1:i+1)*TERRAIN_NUM_DIV_PTS+(j==TERRAIN_NUM_DIV_PTS-1?TERRAIN_NUM_DIV_PTS-1:j+1)];
            Dir[6][1]=HeightMap[i*TERRAIN_NUM_DIV_PTS+(j==TERRAIN_NUM_DIV_PTS-1?TERRAIN_NUM_DIV_PTS-1:j+1)];
            Dir[7][1]=HeightMap[(i==0?0:i-1)*TERRAIN_NUM_DIV_PTS+(j==TERRAIN_NUM_DIV_PTS-1?TERRAIN_NUM_DIV_PTS-1:j+1)];

            Dir[0][1]-=HeightMap[i*TERRAIN_NUM_DIV_PTS+j];
            Dir[1][1]-=HeightMap[i*TERRAIN_NUM_DIV_PTS+j];
            Dir[2][1]-=HeightMap[i*TERRAIN_NUM_DIV_PTS+j];
            Dir[3][1]-=HeightMap[i*TERRAIN_NUM_DIV_PTS+j];
            Dir[4][1]-=HeightMap[i*TERRAIN_NUM_DIV_PTS+j];
            Dir[5][1]-=HeightMap[i*TERRAIN_NUM_DIV_PTS+j];
            Dir[6][1]-=HeightMap[i*TERRAIN_NUM_DIV_PTS+j];
            Dir[7][1]-=HeightMap[i*TERRAIN_NUM_DIV_PTS+j];

            crossProduct(Dir[0],Dir[1],temp);
            normalizeVector(temp);
            N[0]+=temp[0];N[1]+=temp[1];N[2]+=temp[2];

            crossProduct(Dir[1],Dir[2],temp);
            normalizeVector(temp);
            N[0]+=temp[0];N[1]+=temp[1];N[2]+=temp[2];

            crossProduct(Dir[2],Dir[3],temp);
            normalizeVector(temp);
            N[0]+=temp[0];N[1]+=temp[1];N[2]+=temp[2];

            crossProduct(Dir[3],Dir[4],temp);
            normalizeVector(temp);
            N[0]+=temp[0];N[1]+=temp[1];N[2]+=temp[2];

            crossProduct(Dir[4],Dir[5],temp);
            normalizeVector(temp);
            N[0]+=temp[0];N[1]+=temp[1];N[2]+=temp[2];

            crossProduct(Dir[5],Dir[6],temp);
            normalizeVector(temp);
            N[0]+=temp[0];N[1]+=temp[1];N[2]+=temp[2];

            crossProduct(Dir[6],Dir[7],temp);
            normalizeVector(temp);
            N[0]+=temp[0];N[1]+=temp[1];N[2]+=temp[2];

            crossProduct(Dir[7],Dir[0],temp);
            normalizeVector(temp);
            N[0]+=temp[0];N[1]+=temp[1];N[2]+=temp[2];

            normalizeVector(N);

            NormalMat[3*(i*TERRAIN_NUM_DIV_PTS+j)+0]=N[0];
            NormalMat[3*(i*TERRAIN_NUM_DIV_PTS+j)+1]=N[1];
            NormalMat[3*(i*TERRAIN_NUM_DIV_PTS+j)+2]=N[2];

            //ofile<<N[0]<<" "<<N[1]<<" "<<N[2]<<" "<<0.0f<<" ";
            //if(j==35)
            //std::cout<<N[0]<<" "<<N[1]<<" "<<N[2]<<" "<<0.0f<<std::endl;
        }
        //ofile<<std::endl;
    }
    //ofile.close();

}

void createHeightMap(){
    int x,y,comp;
    unsigned char* image;
    image = stbi_load("./data/heights.bmp",&x,&y,&comp,STBI_rgb_alpha);
    int i,j;

    for(i=0;i<x;++i){
        for(j=0;j<y;++j){
            HeightMap[(i*x+j)]=image[4*(i*x+j)]/200.0f;
        }
    }
}

void genNormals(GLfloat const Vertices[],int VerticesSize,GLushort const Indices[],int IndicesSize,GLfloat Normals[]){
    int NumIndices=IndicesSize/sizeof(GLushort);
    int NumNormals=(VerticesSize/sizeof(GLfloat))/4;        //4components per vertex

    GLfloat *TriangleNormals=new GLfloat[(NumIndices*3)/3];  //3vertices per triangle, 3 components per normal

    for(int i=0;i<NumIndices;i+=3){
        GLfloat TempVec1[4],TempVec2[4];
        distance3D(&Vertices[Indices[i+0]*4],&Vertices[Indices[i+1]*4],TempVec1);
        distance3D(&Vertices[Indices[i+0]*4],&Vertices[Indices[i+2]*4],TempVec2);
        crossProduct(TempVec1,TempVec2,TriangleNormals+i);
        normalizeVector(TriangleNormals+i);
    }
    for(int i=0;i<NumNormals*3;++i){
        Normals[i]=0;
    }
    for(int i=0;i<NumIndices;++i){
        Normals[Indices[i]*3+0]+=TriangleNormals[(i/3)*3+0];
        Normals[Indices[i]*3+1]+=TriangleNormals[(i/3)*3+1];
        Normals[Indices[i]*3+2]+=TriangleNormals[(i/3)*3+2];
    }
    for(int i=0;i<NumNormals;++i){
        normalizeVector(&Normals[i*3]);
    }

    delete[] TriangleNormals;
}
