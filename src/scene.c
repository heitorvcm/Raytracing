#include "scene.h"
#include <ctype.h>

#define SCAN(f, fmt, ...) { if(fscanf(f, fmt, __VA_ARGS__) <= 0) {} }

Vec3* ler_ppm(const char *raw_filename, int *w, int *h) {
    char filename[256];
    strncpy(filename, raw_filename, 255);
    filename[255] = '\0';
    int len = strlen(filename);
    while(len > 0 && isspace(filename[len-1])) { filename[len-1] = '\0'; len--; }

    printf("Carregando textura: [%s]\n", filename);
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("AVISO: Textura '%s' nao encontrada. Usando cor solida.\n", filename);
        return NULL;
    }

    char tipo[3];
    SCAN(f, "%2s", tipo);
    int c = fgetc(f);
    while (c == '#') { while (fgetc(f) != '\n'); c = fgetc(f); }
    ungetc(c, f);

    int max_val;
    if (fscanf(f, "%d %d %d", w, h, &max_val) != 3) { fclose(f); return NULL; }

    Vec3 *pixels = (Vec3*)calloc((*w) * (*h), sizeof(Vec3));
    if (!pixels) { fclose(f); return NULL; }

    if (tipo[1] == '3') {
        int r, g, b;
        for (int i = 0; i < (*w) * (*h); i++) {
            SCAN(f, "%d %d %d", &r, &g, &b);
            pixels[i].x = r / 255.0f; pixels[i].y = g / 255.0f; pixels[i].z = b / 255.0f;
        }
    } else if (tipo[1] == '6') {
        fgetc(f);
        unsigned char *buf = (unsigned char*)malloc(3 * (*w) * (*h));
        if (fread(buf, 3, (*w) * (*h), f) != (size_t)((*w) * (*h) * 3)) {};
        for (int i = 0; i < (*w) * (*h); i++) {
            pixels[i].x = buf[i*3]/255.0f; pixels[i].y = buf[i*3+1]/255.0f; pixels[i].z = buf[i*3+2]/255.0f;
        }
        free(buf);
    }
    fclose(f);
    return pixels;
}

void ler_opcionais(FILE *f, float *v1, float *v2) {
    int c;
    while (1) {
        c = fgetc(f);
        if (c == '\n' || c == EOF) { 
            return; 
        }
        if (!isspace(c)) { 
            ungetc(c, f);
            break; 
        }
    }
    
    if (v1 && fscanf(f, "%f", v1) == 1) {
        if (v2) {
             while (1) {
                c = fgetc(f);
                if (c == '\n' || c == EOF) return;
                if (!isspace(c)) { ungetc(c, f); break; }
            }
            fscanf(f, "%f", v2);
        }
    }
}

Scene* carregar_cena(const char* caminho_arquivo) {
    FILE *f = fopen(caminho_arquivo, "r");
    if (!f) { printf("Erro: Arquivo '%s' nao encontrado.\n", caminho_arquivo); return NULL; }

    Scene *scene = (Scene*)calloc(1, sizeof(Scene));
    char buffer[256];

    //câmera
    SCAN(f, "%f %f %f", &scene->cam.eye.x, &scene->cam.eye.y, &scene->cam.eye.z);
    SCAN(f, "%f %f %f", &scene->cam.center.x, &scene->cam.center.y, &scene->cam.center.z);
    SCAN(f, "%f %f %f", &scene->cam.up.x, &scene->cam.up.y, &scene->cam.up.z);
    SCAN(f, "%f", &scene->cam.fovY);
    
    scene->cam.aperture = 0.0f; scene->cam.focal_dist = 1.0f;
    ler_opcionais(f, &scene->cam.aperture, &scene->cam.focal_dist);

    //luzes
    SCAN(f, "%d", &scene->num_lights);
    scene->lights = (Light*)calloc(scene->num_lights, sizeof(Light));
    
    for (int i = 0; i < scene->num_lights; i++) {
        SCAN(f, "%f %f %f", &scene->lights[i].position.x, &scene->lights[i].position.y, &scene->lights[i].position.z);
        SCAN(f, "%f %f %f", &scene->lights[i].color.x, &scene->lights[i].color.y, &scene->lights[i].color.z);
        SCAN(f, "%f %f %f", &scene->lights[i].k_const, &scene->lights[i].k_linear, &scene->lights[i].k_quad);
        
        scene->lights[i].radius = 0.0f;
        ler_opcionais(f, &scene->lights[i].radius, NULL);
    }

    //pigmentos
    SCAN(f, "%d", &scene->num_pigments);
    printf("DEBUG: Lendo %d pigmentos...\n", scene->num_pigments); // Debug
    scene->pigments = (Pigment*)calloc(scene->num_pigments, sizeof(Pigment));
    for (int i = 0; i < scene->num_pigments; i++) {
        SCAN(f, "%s", buffer);
        if (strcmp(buffer, "solid") == 0) {
            scene->pigments[i].type = PIG_SOLID;
            SCAN(f, "%f %f %f", &scene->pigments[i].data.solid.color.x, &scene->pigments[i].data.solid.color.y, &scene->pigments[i].data.solid.color.z);
        } else if (strcmp(buffer, "checker") == 0) {
            scene->pigments[i].type = PIG_CHECKER;
            SCAN(f, "%f %f %f", &scene->pigments[i].data.checker.color1.x, &scene->pigments[i].data.checker.color1.y, &scene->pigments[i].data.checker.color1.z);
            SCAN(f, "%f %f %f", &scene->pigments[i].data.checker.color2.x, &scene->pigments[i].data.checker.color2.y, &scene->pigments[i].data.checker.color2.z);
            SCAN(f, "%f", &scene->pigments[i].data.checker.size);
        } else if (strcmp(buffer, "texmap") == 0) {
            scene->pigments[i].type = PIG_TEXMAP;
            SCAN(f, "%s", scene->pigments[i].data.texmap.filename);
            for(int k=0; k<4; k++) SCAN(f, "%f", &scene->pigments[i].data.texmap.p0[k]);
            for(int k=0; k<4; k++) SCAN(f, "%f", &scene->pigments[i].data.texmap.p1[k]);
            
            scene->pigments[i].data.texmap.pixels = ler_ppm(
                scene->pigments[i].data.texmap.filename,
                &scene->pigments[i].data.texmap.width,
                &scene->pigments[i].data.texmap.height
            );
        }
    }

    //acabamentos
    SCAN(f, "%d", &scene->num_finishes);
    scene->finishes = (Finish*)calloc(scene->num_finishes, sizeof(Finish));
    for (int i = 0; i < scene->num_finishes; i++) {
        SCAN(f, "%f %f %f %f", &scene->finishes[i].ka, &scene->finishes[i].kd, &scene->finishes[i].ks, &scene->finishes[i].alpha);
        SCAN(f, "%f %f %f", &scene->finishes[i].kr, &scene->finishes[i].kt, &scene->finishes[i].ior);
    }

    //objetos
    SCAN(f, "%d", &scene->num_objects);
    scene->objects = (Object*)calloc(scene->num_objects, sizeof(Object));
    
    for (int i = 0; i < scene->num_objects; i++) {
        SCAN(f, "%d %d %s", &scene->objects[i].pigment_id, &scene->objects[i].finish_id, buffer);
        
        if (strcmp(buffer, "sphere") == 0) {
            scene->objects[i].type = OBJ_SPHERE;
            SCAN(f, "%f %f %f %f", 
                &scene->objects[i].geom.sphere.center.x, 
                &scene->objects[i].geom.sphere.center.y, 
                &scene->objects[i].geom.sphere.center.z, 
                &scene->objects[i].geom.sphere.radius);
        } 
        else if (strcmp(buffer, "cylinder") == 0) {
            scene->objects[i].type = OBJ_CYLINDER;
            SCAN(f, "%f %f %f %f %f", 
                &scene->objects[i].geom.cylinder.base.x,
                &scene->objects[i].geom.cylinder.base.y,
                &scene->objects[i].geom.cylinder.base.z,
                &scene->objects[i].geom.cylinder.radius,
                &scene->objects[i].geom.cylinder.height);
        }
        else if (strcmp(buffer, "cone") == 0) {
            scene->objects[i].type = OBJ_CONE;
            SCAN(f, "%f %f %f %f %f", 
                &scene->objects[i].geom.cone.base.x,
                &scene->objects[i].geom.cone.base.y,
                &scene->objects[i].geom.cone.base.z,
                &scene->objects[i].geom.cone.radius,
                &scene->objects[i].geom.cone.height);
        }
        else if (strcmp(buffer, "polyhedron") == 0) {
            scene->objects[i].type = OBJ_POLYHEDRON;
            int n_faces;
            SCAN(f, "%d", &n_faces);
            scene->objects[i].geom.poly.num_faces = n_faces;
            scene->objects[i].geom.poly.faces = (Plane*)malloc(sizeof(Plane) * n_faces);
            for(int j=0; j<n_faces; j++) {
                SCAN(f, "%f %f %f %f", 
                    &scene->objects[i].geom.poly.faces[j].a,
                    &scene->objects[i].geom.poly.faces[j].b,
                    &scene->objects[i].geom.poly.faces[j].c,
                    &scene->objects[i].geom.poly.faces[j].d);
            }
        }
    }

    fclose(f);
    return scene;
}

void liberar_cena(Scene* scene) {
    if (!scene) return;
    if (scene->lights) free(scene->lights);
    if (scene->pigments) {
        for(int i=0; i<scene->num_pigments; i++) {
            if(scene->pigments[i].type == PIG_TEXMAP && scene->pigments[i].data.texmap.pixels)
                free(scene->pigments[i].data.texmap.pixels);
        }
        free(scene->pigments);
    }
    if (scene->finishes) free(scene->finishes);
    if (scene->objects) {
        for(int i=0; i<scene->num_objects; i++) {
            if(scene->objects[i].type == OBJ_POLYHEDRON)
                free(scene->objects[i].geom.poly.faces);
        }
        free(scene->objects);
    }
    free(scene);
}