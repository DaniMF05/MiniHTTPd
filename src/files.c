#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_safe_path(const char *uri, char *resolved_path) {
    char requested_path[512];
    char base_path[512];

    // Obtener la ruta absoluta de la carpeta raíz "www"
    if (!realpath(WEB_ROOT, base_path)) {
        perror("Error al resolver WEB_ROOT");
        return false;
    }

    // Mapear la raíz "/" por defecto a "/index.html"
    const char *target_uri = uri;
    if (strcmp(uri, "/") == 0) {
        target_uri = "/index.html";
    }
    snprintf(requested_path, sizeof(requested_path), "%s%s", WEB_ROOT, target_uri);

    //Resolver la ruta absoluta del archivo solicitado
    // Si devuelve NULL el archivo no existe en el sistema
    if (!realpath(requested_path, resolved_path)) {
        return false;
    }

    //Verificar si la ruta del archivo sigue estando 
    // dentro de la carpeta base "www".
    if (strncmp(resolved_path, base_path, strlen(base_path)) == 0) {
        return true; // Es totalmente seguro acceder
    }

    printf("Intento de Directory Traversal bloqueado: %s\n", resolved_path);
    return false;
}

char* read_file_content(const char *absolute_path, size_t *file_size) {
    // Abrimos en modo binario
    FILE *file = fopen(absolute_path, "rb");
    if (!file) {
        return NULL;
    }

    // Ir al final del archivo para calcular su tamaño total
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return NULL;
    }
    rewind(file); // Regresar al inicio del archivo

    // Asignar dinámicamente el buffer para almacenar el contenido del archivo
    char *buffer = malloc(size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    // Leer los bytes del archivo y guardarlos en el buffer
    size_t bytes_read = fread(buffer, 1, size, file);
    fclose(file);

    // Validar que se haya leído la totalidad del archivo de forma correcta
    if (bytes_read != (size_t)size) {
        free(buffer);
        return NULL;
    }

    *file_size = (size_t)size;
    return buffer;
}