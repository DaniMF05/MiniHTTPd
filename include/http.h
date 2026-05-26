#ifndef HTTP_H
#define HTTP_H
#include <stdbool.h>

#define MAX_BUFFER 4096

// Estructura para almacenar los datos limpios de la petición
typedef struct {
    char method[16];
    char uri[256];
    char version[16];
    bool keep_alive;
} http_request_t;

// Procesa los datos crudos del socket, llena la estructura y maneja la respuesta
bool handle_client_request(int client_fd);

// Envia una respuesta de error HTTP (400, 403, 404, 405, 500)
void send_error_response(int client_fd, int status_code, const char *status_message);

#endif // HTTP_H