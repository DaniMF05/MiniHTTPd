#ifndef FILES_H
#define FILES_H

#include <stdbool.h>
#include <stddef.h>

#define WEB_ROOT "./www"

// Verifica si el archivo solicitado es seguro y está dentro de la carpeta www/
bool is_safe_path(const char *uri, char *resolved_path);

// Lee el contenido de un archivo seguro y devuelve su tamaño
char* read_file_content(const char *absolute_path, size_t *file_size);

#endif // FILES_H