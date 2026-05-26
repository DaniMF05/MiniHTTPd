#include "mime.h"
#include <string.h>

const char* get_mime_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    
    // Si el archivo no tiene extensión, devolvemos un tipo binario genérico
    if (!ext) {
        return "application/octet-stream";
    }

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
        return "text/html";
    }
    if (strcmp(ext, ".css") == 0) {
        return "text/css";
    }
    if (strcmp(ext, ".js") == 0) {
        return "application/javascript";
    }
    if (strcmp(ext, ".png") == 0) {
        return "image/png";
    }
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        return "image/jpeg";
    }

    return "application/octet-stream";
}