## 📝 ARQUIVO: `README.md`

### Ray Tracer Distribuído (V1.0)

Este projeto implementa um Ray Tracer em C, utilizando técnicas de **Ray Tracing Distribuído** para gerar imagens 3D fotorrealistas a partir de um arquivo de configuração de cena.

---

### 1. Funcionalidades Implementadas (Features)

O motor de renderização suporta as seguintes capacidades, baseadas em Ray Tracing Distribuído:

* **Geometria:** Suporte nativo para **Esferas**, **Poliedros Convexos**, **Cilindros** e **Cones**.
* **Iluminação (Phong):** Implementação do Modelo de Reflexão de **Phong** (Ambiente, Difusa, Especular) para iluminação local.
* **Ray Tracing Recursivo:** Simulação de superfícies transparentes ($\text{K}_t$) e reflexivas ($\text{K}_r$).
* **Texturização:** Suporte a cores sólidas, padrões procedurais (`checker`) e mapeamento de texturas planas (`texmap`).
* **Anti-aliasing:** Amostragem por pixel (`multisampling`) para suavizar bordas.

#### Recursos de Ray Tracing Distribuído (DRT)

1.  **Sombras Suaves (Soft Shadows):** O programa utiliza amostragem de Monte Carlo sobre a área física da fonte de luz para gerar penumbra realista. A suavidade é controlada pelo parâmetro **Raio (R)** da luz.
2.  **Profundidade de Campo (Depth of Field - DoF):** É implementada a simulação de lentes com abertura ($\text{aperture}$) e distância focal ($\text{focal\_dist}$) variáveis. Isso é feito através de multisampling para obter foco seletivo.

---

### 2. Especificação do Arquivo de Entrada (Estrutura Normal)

O arquivo de entrada é um texto simples que define a cena em blocos sequenciais.

1. **Câmera:** Definida por Posição, Ponto de Visão, Vetor 'Up' e **FOV** (Field of View). Os parâmetros de **Abertura (Aperture)** e **Distância Focal (Focal\_Dist)** são definidos na mesma linha do FOV.

2. **Luzes:** Cada luz é definida por Posição, Cor, Atenuação ($\text{K}_c, \text{K}_l, \text{K}_q$) e o **Raio ($\text{R}$)**. Se $\text{R}>0$, ativa a sombra suave.

3. **Pigmentos:** Define cores sólidas, padrões `checker` ou `texmap` (mapeamento de textura).

4. **Acabamentos (Finish):** Define propriedades de material (Phong) e de reflexão/refração ($\text{K}_r, \text{K}_t, \text{IOR}$).

5. **Objetos:** Define o índice do Pigmento, o índice do Acabamento e o tipo (**`sphere`**, **`cylinder`**, **`cone`**, **`polyhedron`**), seguido dos parâmetros de geometria (centro, raio, altura, ou planos).

---

### 3. Instruções de Compilação e Execução

#### 3.1. Compilação
O projeto é compilado via `Makefile` e requer a biblioteca matemática (`-lm`).
```bash make

#### 3.2. Execução

O programa exige o arquivo de entrada e o nome do arquivo de saída (PPM). A resolução é opcional.

./renderer <arq_entrada> <arq_saida.ppm> [largura] [altura]

exemplo:

./renderer Arquivos/test1.in imagem.ppm 1280 960