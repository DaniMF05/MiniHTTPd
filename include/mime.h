#ifndef MIME_H
#define MIME_H

// Devuelve el tipo MIME basado en la extensión del archivo (ej: "text/html")
const char* get_mime_type(const char *filename);

#endif // MIME_H