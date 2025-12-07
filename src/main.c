#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "scene.h"
#include "render.h"

int main(int argc, char *argv[]){
    if (argc < 3) {
        fprintf(stderr, "Erro: Argumentos obrigatorios ausentes.\n");
        fprintf(stderr, "Uso: %s <arq_entrada> <arq_saida> [largura] [altura]\n", argv[0]);
        fprintf(stderr, "Padrao: 800x600\n");
        return 1;
    }

    srand(time(NULL));
    char *input_file = argv[1];
    char *output_file = argv[2];
    
    int width = 800;
    int height = 600;
    if (argc == 5) {
        width = atoi(argv[3]);
        height = atoi(argv[4]);
        if (width <= 0 || height <= 0) {
            width = 800;
            height = 600;
        }

    Scene *scene = carregar_cena(input_file);

    if (!scene) {
        fprintf(stderr, "Erro ao carregar a cena do arquivo %s.\n", input_file);
        return 1;
    }

    renderizar(scene, output_file, width, height);
    liberar_cena(scene);

    return 0;
}
}