#ifndef SCENE_H
#define SCENE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float a, b, c, d;
} Plane;

typedef struct {
    Vec3 eye;
    Vec3 center;
    Vec3 up;
    float fovY;

    float aperture;      //abertura da lente (0.0 = nitido)
    float focal_dist;
} Camera;

typedef struct {
    Vec3 position;
    Vec3 color;
    float k_const;
    float k_linear;
    float k_quad;
    float radius;        //tamanho da luz (0.0 = pontual)
} Light;

typedef enum { PIG_SOLID, PIG_CHECKER, PIG_TEXMAP } PigmentType;

typedef struct {
    PigmentType type;
    union {
        struct { Vec3 color; } solid;
        struct { Vec3 color1, color2; float size; } checker;
        struct { 
            char filename[256]; 
            float p0[4]; 
            float p1[4]; 
            int width;
            int height;
            Vec3 *pixels; 
        } texmap;
    } data;
} Pigment;

typedef struct {
    float ka, kd, ks, alpha;
    float kr, kt, ior;
} Finish;

typedef enum { OBJ_SPHERE, OBJ_POLYHEDRON, OBJ_CYLINDER, OBJ_CONE } ObjectType;

typedef struct {
    int pigment_id;
    int finish_id;
    ObjectType type;

    union {
        //esfera
        struct {
            Vec3 center;
            float radius;
        } sphere;

        //coliedro
        struct {
            int num_faces;
            Plane *faces; 
        } poly;

        //cilindro
        struct {
            Vec3 base;
            float radius;
            float height;
        } cylinder;

        //cone
        struct {
            Vec3 base;
            float radius;
            float height;
        } cone;
    } geom;
} Object;

typedef struct {
    Camera cam;
    int num_lights;   Light *lights;
    int num_pigments; Pigment *pigments;
    int num_finishes; Finish *finishes;
    int num_objects;  Object *objects;
} Scene;

Scene* carregar_cena(const char* caminho_arquivo);
void liberar_cena(Scene* scene);

#endif