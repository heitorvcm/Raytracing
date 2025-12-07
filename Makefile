
TARGET   = renderer
CC       = gcc
CFLAGS   = -Wall -Wextra -g -O2 -Iinclude
LDFLAGS  = -lm
SRC_DIR  = src
OBJ_DIR  = obj
INC_DIR  = include

SRCS := $(wildcard $(SRC_DIR)/*.c)

OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linkando o executável: $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Sucesso! Execute com ./$(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compilando: $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	@echo "Limpando arquivos compilados..."
	rm -rf $(OBJ_DIR) $(TARGET) output.ppm

run: $(TARGET)
	./$(TARGET) cena.txt

.PHONY: all clean run