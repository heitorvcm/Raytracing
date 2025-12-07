# Ray Tracer Distribuído (V1.0)

Este projeto é um traçador de raios (Ray Tracer) em C que implementa técnicas avançadas de renderização, como Ray Tracing Distribuído, para gerar imagens fotorrealistas.

---

## 1. Estrutura do Arquivo de Entrada (Cena)

O programa lê a configuração da cena a partir de um arquivo de texto, onde os parâmetros devem ser fornecidos em uma ordem específica.

### 1.1. Ordem dos Blocos

1. **Câmera** (4 linhas de parâmetros)
2. **Luzes** (3 parâmetros por luz)
3. **Pigmentos** (Definições de cor/textura)
4. **Acabamentos (Finish)** (Definições de material)
5. **Objetos** (Lista de formas)

---

### 1.2. Bloco 1: Câmera (5 Parâmetros)

| Parâmetro | Linha | Campos (X, Y, Z) | Descrição |
| :--- | :--- | :--- | :--- |
| **Eye** | 1 | X Y Z | Posição do observador no espaço. |
| **Center** | 2 | X Y Z | Ponto para onde a câmera está apontada. |
| **Up** | 3 | X Y Z | Vetor que define o "topo" da imagem. |
| **FOV, Aperture, Focal Dist** | 4 | FOV Aperture Focal\_Dist | **FOV:** Ângulo de visão (vertical). **Aperture:** Diâmetro da abertura da lente (para DoF). **Focal\_Dist:** Distância de foco (para DoF). |

| Parâmetro | Linha | Campos | Descrição |
| :--- | :--- | :--- | :--- |
| **Amostras** | 5 | N\_SAMPLES | Número de raios disparados por pixel (para AA, DoF e Soft Shadows). |

---

### 1.3. Bloco 2: Luzes

A primeira linha define o número de luzes (`N_LIGHTS`). Cada luz subsequente é definida por 7 parâmetros:

| Posição (X Y Z) | Cor (R G B) | Intensidade (1.0) | Raio (R) |
| :--- | :--- | :--- | :--- |
| X Y Z | R G B | K\_CONST K\_LIN K\_QUAD | R |

* **Raio (R):** Define o tamanho físico da luz. `R > 0` ativa **Sombras Suaves (Soft Shadows)**.

---

### 1.4. Bloco 3: Pigmentos (Texturas)

A primeira linha define o número de pigmentos (`N_PIGMENTS`).

| Tipo | Cor/Textura | Parâmetros |
| :--- | :--- | :--- |
| `solid` | R G B | Cor sólida. |
| `checker` | R1 G1 B1, R2 G2 B2, Size | Padrão xadrez com duas cores e tamanho. |
| `texmap` | `nome_arq.ppm` | Mapeamento de textura de imagem. |
| **(Texmap Extra)** | Linha 1 | P1\_X P1\_Y P1\_Z P1\_W | Coordenadas de mapeamento (ajuste de escala/orientação). |
| **(Texmap Extra)** | Linha 2 | P2\_X P2\_Y P2\_Z P2\_W | Coordenadas de mapeamento. |

---

### 1.5. Bloco 4: Acabamentos (Finish)

A primeira linha define o número de acabamentos (`N_FINISHES`). Cada acabamento usa o modelo de Phong expandido:

| Ka | Kd | Ks | Alpha | Kr | Kt | IOR |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Reflexão Ambiente | Reflexão Difusa | Reflexão Especular | Brilho (Exponencial) | Coef. de Reflexão | Coef. de Transmissão | Índice de Refração |

* **Transparência:** Para um objeto transparente (vidro), use **`Kt`** alto (ex: `0.95`) e **`IOR`** (ex: `1.5`).
* **Reflexão:** Para um objeto espelhado, use **`Kr`** alto (ex: `0.95`).

---

### 1.6. Bloco 5: Objetos

A primeira linha define o número de objetos (`N_OBJECTS`).

| Pigment Index | Finish Index | Tipo | Geometria (Parâmetros) |
| :--- | :--- | :--- | :--- |
| P\_IDX | F\_IDX | `sphere` | Center\_X Y Z Radius |
| P\_IDX | F\_IDX | `polyhedron` | N\_PLANES |
| | | | Plano 1: A B C D |
| | | | ... N Planos |
| P\_IDX | F\_IDX | `cylinder` | Base\_X Y Z Radius Height |
| P\_IDX | F\_IDX | `cone` | Base\_X Y Z Radius Height |

---

## 2. Instruções de Compilação e Execução

### 2.1. Compilação

O projeto requer a biblioteca matemática (`-lm`) e deve ser compilado via `Makefile`:

```bash
make