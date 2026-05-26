# Variables del compilador y banderas
CC = gcc

CFLAGS = -Wall -Wextra -std=gnu99 -Iinclude -g

# Directorios de la estructura del proyecto
SRC_DIR = src
OBJ_DIR = obj
BIN = minihttpd

# Detección automática de archivos fuente y objetos
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Regla principal (compila el binario)
all: $(BIN)

# Enlace del binario final
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	@echo "Servidor MiniHTTPd compilado con éxito. Ejecuta ./$(BIN) para iniciar."

# Compilación de archivos objeto (.o) individuales
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Crear la carpeta de objetos temporales si no existe
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Limpieza de archivos compilados
clean:
	rm -rf $(OBJ_DIR) $(BIN)
	@echo "Limpieza completada."

# Evitar conflictos con archivos que se llamen igual que las reglas
.PHONY: all clean