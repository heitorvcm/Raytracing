## Ray Tracer Distribuído (V1.0)

Este projeto implementa um Ray Tracer em C, utilizando técnicas de **Ray Tracing Distribuído** para gerar imagens 3D fotorrealistas a partir de um arquivo de configuração de cena.

---

## 1. Funcionalidades do Motor de Renderização

O motor de renderização suporta as seguintes capacidades de renderização fotorrealista e computação gráfica:

* **Geometria:** Suporte nativo para **Esferas**, **Poliedros Convexos**, **Cilindros** e **Cones**.
* **Iluminação:** Implementação do Modelo de Reflexão de **Phong** (Ambient, Diffuse, Specular) para iluminação local.
* **Ray Tracing Recursivo:** Simulação de superfícies transparentes ($K_t$) e reflexivas ($K_r$).
* **Texturização:** Suporte a cores sólidas, padrões procedurais (`checker`) e mapeamento de texturas planas (`texmap`).
* **Anti-aliasing:** Amostragem por pixel (`multisampling`) para suavizar bordas.

### Recursos de Ray Tracing Distribuído (DRT)

1.  **Sombras Suaves (Soft Shadows):** O programa utiliza amostragem de Monte Carlo sobre a área física da fonte de luz para gerar penumbra realista. A suavidade é controlada pelo parâmetro **Raio (R)** da luz.
2.  **Profundidade de Campo (Depth of Field - DoF):** É implementada a simulação de lentes com abertura ($\text{aperture}$) e distância focal ($\text{focal\_dist}$) variáveis, feita através de multisampling para obter foco seletivo.

---

## 2. Estrutura do Arquivo de Entrada (Parâmetros)

O arquivo de entrada é um texto simples que define a cena em blocos sequenciais obrigatórios.

1.  **Câmera:** Definida por Posição, Ponto de Visão, Vetor 'Up' e FOV. Os parâmetros de **Abertura ($\text{aperture}$)** e **Distância Focal ($\text{focal\_dist}$)** são definidos na mesma linha do FOV.

2.  **Luzes:** Cada luz é definida por Posição, Cor, Atenuação ($K_c, K_l, K_q$) e o **Raio (R)**. Se $R>0$, ativa a sombra suave.

3.  **Pigmentos:** Define cores sólidas, padrões `checker` ou `texmap` (mapeamento de textura).

4.  **Acabamentos (Finish):** Define propriedades de material (Phong) e de reflexão/refração ($K_r, K_t, \text{IOR}$).

5.  **Objetos:** Define o índice do Pigmento, o índice do Acabamento e o tipo (**`sphere`**, **`cylinder`**, **`cone`**, **`polyhedron`**), seguido dos parâmetros de geometria (centro, raio, altura, ou planos).

---

### 3. Instruções de Compilação e Execução

#### 3.1. Compilação
```bash
make
```
#### 3.2. Execução

O programa exige o arquivo de entrada e o nome do arquivo de saída (PPM). A resolução é opcional.
```bash
./renderer <arq_entrada> <arq_saida.ppm> [largura] [altura]
```
exemplo:
```bash
./renderer Arquivos/test1.in imagem.ppm 1280 960
```