#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "scene.h"
#include "algebra.h"
#include "render.h"

#define WIDTH 800
#define HEIGHT 600
#define EPSILON 0.001f
#define MAX_DEPTH 5      //limite de recursão


//calcula o vetor de reflexão R = I - 2(N.I)N
Vec3 reflect(Vec3 I, Vec3 N) {
    return v_sub(I, v_scale(N, 2.0f * v_dot(N, I)));
}

//gera um float aleatório entre 0 e 1
float randf() {
    return (float)rand() / (float)RAND_MAX;
}

//gera um ponto aleatório dentro de um disco unitário
void random_in_unit_disk(float *dx, float *dy) {
    float a = randf() * 2.0f * M_PI;
    float r = sqrt(randf());              
    *dx = r * cos(a);
    *dy = r * sin(a);
}

//lei de Snell
Vec3 refract(Vec3 I, Vec3 N, float eta) {
    float dot_ni = v_dot(N, I);
    float k = 1.0f - eta * eta * (1.0f - dot_ni * dot_ni);
    if (k < 0.0f) { Vec3 z = {0,0,0}; return z; } // TIR
    Vec3 a = v_scale(I, eta);
    Vec3 b = v_scale(N, eta * dot_ni + sqrt(k));
    return v_sub(a, b);
}


float intersect_sphere(Vec3 origin, Vec3 dir, Object *obj) {
    Vec3 oc = v_sub(origin, obj->geom.sphere.center);
    float a = v_dot(dir, dir);
    float b = 2.0f * v_dot(oc, dir);
    float c = v_dot(oc, oc) - (obj->geom.sphere.radius * obj->geom.sphere.radius);
    float disc = b*b - 4*a*c;
    if (disc < 0) return -1.0f;
    float t = (-b - sqrt(disc)) / (2.0f * a);
    if (t > EPSILON) return t;
    t = (-b + sqrt(disc)) / (2.0f * a);
    if (t > EPSILON) return t;
    return -1.0f;
}

float intersect_cylinder(Vec3 origin, Vec3 dir, Object *obj) {
    Vec3 base = obj->geom.cylinder.base;
    float radius = obj->geom.cylinder.radius;
    float height = obj->geom.cylinder.height;

    Vec3 o = v_sub(origin, base);
    float t_final = FLT_MAX;
    float a = dir.x * dir.x + dir.y * dir.y;
    
    if (fabs(a) > 1e-6) { 
        float b = 2.0f * (o.x * dir.x + o.y * dir.y);
        float c = (o.x * o.x + o.y * o.y) - (radius * radius);
        float disc = b*b - 4*a*c;

        if (disc >= 0) {
            float t0 = (-b - sqrt(disc)) / (2.0f * a);
            float t1 = (-b + sqrt(disc)) / (2.0f * a);

            if (t0 > EPSILON) {
                float z0 = o.z + t0 * dir.z;
                if (z0 >= 0.0f && z0 <= height) {
                    if (t0 < t_final) t_final = t0;
                }
            }
            if (t1 > EPSILON) {
                float z1 = o.z + t1 * dir.z;
                if (z1 >= 0.0f && z1 <= height) {
                    if (t1 < t_final) t_final = t1;
                }
            }
        }
    }

    if (fabs(dir.z) > 1e-6) { 
        float t_base = -o.z / dir.z;
        if (t_base > EPSILON && t_base < t_final) {
            Vec3 p = v_add(o, v_scale(dir, t_base));
            if (p.x*p.x + p.y*p.y <= radius*radius + EPSILON) { // Círculo no XY
                t_final = t_base;
            }
        }
        float t_top = (height - o.z) / dir.z;
        if (t_top > EPSILON && t_top < t_final) {
            Vec3 p = v_add(o, v_scale(dir, t_top));
            if (p.x*p.x + p.y*p.y <= radius*radius + EPSILON) {
                t_final = t_top;
            }
        }
    }
    return (t_final < FLT_MAX) ? t_final : -1.0f;
}

float intersect_cone(Vec3 origin, Vec3 dir, Object *obj) {
    Vec3 basePos = obj->geom.cone.base;
    float radius = obj->geom.cone.radius;
    float height = obj->geom.cone.height;

    Vec3 o = v_sub(origin, basePos);
    float t_final = FLT_MAX;
    float rh = radius / height;
    float rh2 = rh * rh;

    //x^2 + y^2 = (r * (h-z)/h)^2
    float a = dir.x*dir.x + dir.y*dir.y - rh2 * dir.z*dir.z;
    float b = 2.0f * (o.x*dir.x + o.y*dir.y - rh2 * o.z*dir.z + rh2 * height * dir.z);
    float c = o.x*o.x + o.y*o.y - rh2 * (o.z - height)*(o.z - height);

    float disc = b*b - 4*a*c;

    if (fabs(a) > 1e-6 && disc >= 0) {
        float t0 = (-b - sqrt(disc)) / (2.0f * a);
        float t1 = (-b + sqrt(disc)) / (2.0f * a);

        if (t0 > EPSILON) {
            float z0 = o.z + t0 * dir.z;
            if (z0 >= 0.0f && z0 <= height && t0 < t_final) t_final = t0;
        }
        if (t1 > EPSILON) {
            float z1 = o.z + t1 * dir.z;
            if (z1 >= 0.0f && z1 <= height && t1 < t_final) t_final = t1;
        }
    }
    if (fabs(dir.z) > 1e-6) {
        float t_base = -o.z / dir.z;
        if (t_base > EPSILON && t_base < t_final) {
            Vec3 p = v_add(o, v_scale(dir, t_base));
            if (p.x*p.x + p.y*p.y <= radius*radius + EPSILON) {
                t_final = t_base;
            }
        }
    }
    return (t_final < FLT_MAX) ? t_final : -1.0f;
}

float intersect_polyhedron(Vec3 origin, Vec3 dir, Object *obj) {
    float t_near = -FLT_MAX;
    float t_far = FLT_MAX;
    for (int i = 0; i < obj->geom.poly.num_faces; i++) {
        Plane p = obj->geom.poly.faces[i];
        Vec3 n = {p.a, p.b, p.c};
        float denom = v_dot(n, dir);
        float num = -(v_dot(n, origin) + p.d);
        if (fabs(denom) < 1e-6) {
            if (num < 0) return -1.0f;
        } else {
            float t = num / denom;
            if (denom < 0) { if (t > t_near) t_near = t; }
            else { if (t < t_far) t_far = t; }
        }
    }
    if (t_near > t_far || t_far < 0) return -1.0f;
    return (t_near > EPSILON) ? t_near : ((t_far > EPSILON) ? t_far : -1.0f);
}

float intersect_object(Vec3 origin, Vec3 dir, Object *obj) {
    if (obj->type == OBJ_SPHERE) return intersect_sphere(origin, dir, obj);
    if (obj->type == OBJ_POLYHEDRON) return intersect_polyhedron(origin, dir, obj);
    if (obj->type == OBJ_CYLINDER) return intersect_cylinder(origin, dir, obj);
    if (obj->type == OBJ_CONE) return intersect_cone(origin, dir, obj);
    return -1.0f;
}

// Normais e Cores

Vec3 get_normal(Object *obj, Vec3 point) {
    float margin = EPSILON * 10.0f;

    if (obj->type == OBJ_SPHERE) {
        return v_normalize(v_sub(point, obj->geom.sphere.center));
    } 
    else if (obj->type == OBJ_CYLINDER) {
        Vec3 local = v_sub(point, obj->geom.cylinder.base);
        float h = obj->geom.cylinder.height;
        if (fabs(local.z) < margin) { Vec3 n={0,0,-1}; return n; } // Base
        if (fabs(local.z - h) < margin) { Vec3 n={0,0,1}; return n; } // Topo
        Vec3 n = {local.x, local.y, 0.0f}; return v_normalize(n);
    }
    else if (obj->type == OBJ_CONE) {
        Vec3 local = v_sub(point, obj->geom.cone.base);
        float h = obj->geom.cone.height;
        float r = obj->geom.cone.radius;

        if (fabs(local.z) < margin) { Vec3 n = {0, 0, -1}; return n; } // Base

        float rh = r/h;
        Vec3 n;
        n.x = local.x;
        n.y = local.y;
        n.z = rh * sqrt(local.x*local.x + local.y*local.y);
        return v_normalize(n);
    }
    else if (obj->type == OBJ_POLYHEDRON) {
        for (int i = 0; i < obj->geom.poly.num_faces; i++) {
            Plane p = obj->geom.poly.faces[i];
            if (fabs(p.a*point.x + p.b*point.y + p.c*point.z + p.d) < 0.1f) {
                Vec3 n = {p.a, p.b, p.c};
                return v_normalize(n);
            }
        }
    }
    Vec3 def = {0, 0, 1}; return def;
}

Vec3 get_pigment_color(Pigment *pig, Vec3 point) {
    if (pig->type == PIG_SOLID) return pig->data.solid.color;
    if (pig->type == PIG_CHECKER) {
        float s = pig->data.checker.size;
        int c = (int)(floor(point.x/s) + floor(point.y/s) + floor(point.z/s));
        return (c % 2 == 0) ? pig->data.checker.color1 : pig->data.checker.color2;
    }
    if (pig->type == PIG_TEXMAP) {
        if (!pig->data.texmap.pixels) { Vec3 err = {0.5,0.7,1.0}; return err; }
        float *p0 = pig->data.texmap.p0;
        float *p1 = pig->data.texmap.p1;
        float s_val = p0[0]*point.x + p0[1]*point.y + p0[2]*point.z + p0[3];
        float r_val = p1[0]*point.x + p1[1]*point.y + p1[2]*point.z + p1[3];
        s_val -= floor(s_val); r_val -= floor(r_val);
        int w = pig->data.texmap.width, h = pig->data.texmap.height;
        int u = (int)(s_val * w) % w; int v = (int)(r_val * h) % h;
        if (u < 0) u += w; 
        if (v < 0) v += h;
        
        return pig->data.texmap.pixels[v * w + u];
    }
    Vec3 err = {1, 0, 1}; return err;
}

//SOMBRAS SUAVES DISTRIBUTED RAY TRACING 

int is_shadowed_hard(Scene *s, Vec3 point, Vec3 light_dir, float light_dist) {
    Vec3 shadow_origin = v_add(point, v_scale(light_dir, EPSILON));
    for (int i = 0; i < s->num_objects; i++) {
        float t = intersect_object(shadow_origin, light_dir, &s->objects[i]);
        if (t > 0.0f && t < light_dist) return 1; // Bloqueado
    }
    return 0;
}

float shadow_intensity(Scene *s, Vec3 point, Light *l) {
    if (l->radius <= 0.0f) {
        Vec3 light_vec = v_sub(l->position, point);
        float dist = sqrt(v_dot(light_vec, light_vec));
        Vec3 light_dir = v_normalize(light_vec);
        return is_shadowed_hard(s, point, light_dir, dist) ? 0.0f : 1.0f;
    }

    int samples = 32; //qualidade da sombra
    int visible_count = 0;
    
    for (int i = 0; i < samples; i++) {
        Vec3 offset;
        float dx, dy, dz;
        do {
            dx = randf()*2-1; dy = randf()*2-1; dz = randf()*2-1;
        } while (dx*dx + dy*dy + dz*dz > 1.0f);
        
        offset.x = dx * l->radius;
        offset.y = dy * l->radius;
        offset.z = dz * l->radius;
        
        Vec3 target_pos = v_add(l->position, offset);
        Vec3 to_light = v_sub(target_pos, point);
        float dist = sqrt(v_dot(to_light, to_light));
        Vec3 dir = v_scale(to_light, 1.0f/dist);

        if (!is_shadowed_hard(s, point, dir, dist)) {
            visible_count++;
        }
    }
    return (float)visible_count / (float)samples;
}

// RAY TRACER 

Vec3 trace_ray(Scene *s, Vec3 origin, Vec3 dir, int depth) {
    if (depth > MAX_DEPTH) { Vec3 k={0,0,0}; return k; }

    float closest_t = FLT_MAX;
    int closest_idx = -1;

    for (int i = 0; i < s->num_objects; i++) {
        float t = intersect_object(origin, dir, &s->objects[i]);
        if (t > EPSILON && t < closest_t) {
            closest_t = t;
            closest_idx = i;
        }
    }

    if (closest_idx == -1) { Vec3 bg={0.05, 0.05, 0.05}; return bg; }

    Object *obj = &s->objects[closest_idx];
    Pigment *pig = &s->pigments[obj->pigment_id];
    Finish *fin = &s->finishes[obj->finish_id];

    Vec3 hit_point = v_add(origin, v_scale(dir, closest_t));
    Vec3 normal = get_normal(obj, hit_point);
    Vec3 view = v_neg(dir);

    float eta;
    Vec3 proper_normal;
    if (v_dot(dir, normal) < 0) { eta = 1.0f/fin->ior; proper_normal = normal; }
    else { eta = fin->ior; proper_normal = v_neg(normal); }

    Vec3 final_color = {0,0,0};
    Vec3 pigment_color = get_pigment_color(pig, hit_point);
    float opacity = 1.0f - fin->kt; 
    if (opacity < 0.0f) opacity = 0.0f;

    // ambiente
    if (s->num_lights > 0) {
        Vec3 amb = v_mul(pigment_color, s->lights[0].color);
        final_color = v_add(final_color, v_scale(amb, fin->ka * opacity));
    }

    // luzes com ou sem a Sombra Suave)
    for (int i = 1; i < s->num_lights; i++) {
        Light *l = &s->lights[i];
        
        float visibility = shadow_intensity(s, hit_point, l);
        
        if (visibility > 0.0f) {
            Vec3 l_vec = v_sub(l->position, hit_point);
            float dist = sqrt(v_dot(l_vec, l_vec));
            Vec3 l_dir = v_scale(l_vec, 1.0f/dist);
            float att = 1.0f / (l->k_const + l->k_linear*dist + l->k_quad*dist*dist);
            
            // difusa
            float diff = fmaxf(0.0f, v_dot(proper_normal, l_dir));
            Vec3 diffuse = v_scale(v_mul(pigment_color, l->color), fin->kd * diff * opacity * visibility);

            // especular PHONG
            Vec3 r_light = v_normalize(reflect(v_neg(l_dir), proper_normal));
            float RdotV = fmaxf(0.0f, v_dot(r_light, view));
            float spec = powf(RdotV, fin->alpha);
            Vec3 specular = v_scale(l->color, fin->ks * spec * visibility);

            final_color = v_add(final_color, v_scale(v_add(diffuse, specular), att));
        }
    }

    //reflexão
    if (fin->kr > 0.0f) {
        Vec3 r_dir = v_normalize(reflect(dir, proper_normal));
        Vec3 r_pos = v_add(hit_point, v_scale(r_dir, EPSILON));
        Vec3 r_col = trace_ray(s, r_pos, r_dir, depth + 1);
        final_color = v_add(final_color, v_scale(r_col, fin->kr));
    }

    //refração
    if (fin->kt > 0.0f) {
        Vec3 t_dir = refract(dir, proper_normal, eta);
        if (v_dot(t_dir, t_dir) > 0.0001f) {
            t_dir = v_normalize(t_dir);
            Vec3 t_pos = v_add(hit_point, v_scale(t_dir, EPSILON));
            Vec3 t_col = trace_ray(s, t_pos, t_dir, depth + 1);
            
            float tint = 0.1f; 
            Vec3 w = {1,1,1};
            Vec3 soft;
            soft.x = pigment_color.x*tint + w.x*(1-tint);
            soft.y = pigment_color.y*tint + w.y*(1-tint);
            soft.z = pigment_color.z*tint + w.z*(1-tint);

            Vec3 filtered = v_mul(t_col, soft);
            final_color = v_add(final_color, v_scale(filtered, fin->kt));
        }
    }

    if(final_color.x > 1) final_color.x = 1;
    if(final_color.y > 1) final_color.y = 1;
    if(final_color.z > 1) final_color.z = 1;
    return final_color;
}

// RENDERIZAÇÃO

void renderizar(Scene *s, const char *output_file, int width, int height) {
    printf("Renderizando cena para [%s]... (%dx%d)\n", output_file, width, height);

    Vec3 w_cam = v_normalize(v_sub(s->cam.eye, s->cam.center));
    Vec3 u_cam = v_normalize(v_cross(s->cam.up, w_cam));
    Vec3 v_cam = v_cross(w_cam, u_cam);

    float aspect = (float)width / height;
    float theta = s->cam.fovY * (M_PI / 180.0f);
    float half_h = tan(theta / 2.0f);
    float half_w = aspect * half_h;

    //amostras por pixel (Anti-aliasing)
    int samples = 4;

    FILE *f_ppm = fopen(output_file, "w"); 
    if (!f_ppm) {
        fprintf(stderr, "Erro fatal: Nao foi possivel criar o arquivo de saida PPM [%s].\n", output_file);
        return;
    }

    fprintf(f_ppm, "P3\n%d %d\n255\n", width, height);

    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            
            Vec3 pixel_color = {0, 0, 0};

            for (int k = 0; k < samples; k++) {
                
                float s_c = (x + randf()) / width;
                float t_c = (y + randf()) / height;
                
                float world_x = (s_c * 2.0f - 1.0f) * half_w;
                float world_y = (t_c * 2.0f - 1.0f) * half_h;

                Vec3 pixel_target = v_add(v_scale(u_cam, world_x), v_scale(v_cam, world_y));
                pixel_target = v_sub(pixel_target, w_cam); 

                // Câmera / DoF
                Vec3 origin = s->cam.eye;
                Vec3 dir = v_normalize(pixel_target);

                if (s->cam.aperture > 1e-6) {
                    Vec3 focal_point = v_add(origin, v_scale(dir, s->cam.focal_dist));
                    float dx, dy;
                    random_in_unit_disk(&dx, &dy);
                    Vec3 lens = v_add(v_scale(u_cam, dx * s->cam.aperture), v_scale(v_cam, dy * s->cam.aperture));
                    origin = v_add(s->cam.eye, lens);
                    dir = v_normalize(v_sub(focal_point, origin));
                }

                Vec3 sample_col = trace_ray(s, origin, dir, 0); 
                pixel_color = v_add(pixel_color, sample_col);
            }

            // Média e Conversão para 0-255
            pixel_color = v_scale(pixel_color, 1.0f / samples);

            int ir = (int)(255.99 * pixel_color.x);
            int ig = (int)(255.99 * pixel_color.y);
            int ib = (int)(255.99 * pixel_color.z);
            
            if(ir>255) ir=255; if(ig>255) ig=255; if(ib>255) ib=255;
            fprintf(f_ppm, "%d %d %d ", ir, ig, ib);
        }
        fprintf(f_ppm, "\n");
        if (y % 60 == 0) printf("\rRenderizando... [%d/%d]", height - y, height);
    }

    fclose(f_ppm);
}