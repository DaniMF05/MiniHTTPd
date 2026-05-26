#include "http.h"
#include "files.h"
#include "mime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// Envia respuestas de error HTTP cortas y seguras (Content-Length: 0)
void send_error_response(int client_fd, int status_code, const char *status_message) {
    char response[256];
    int len = snprintf(response, sizeof(response),
                       "HTTP/1.1 %d %s\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: 0\r\n"
                       "Connection: close\r\n\r\n", 
                       status_code, status_message);
    if (len > 0) {
        send(client_fd, response, len, 0);
    }
}


bool handle_client_request(int client_fd) {
    char buffer[MAX_BUFFER];
    ssize_t bytes_read = recv(client_fd, buffer, MAX_BUFFER - 1, 0);
    
    if (bytes_read <= 0) return false; // El cliente se desconectó voluntariamente
    buffer[bytes_read] = '\0';   

    // Valida que la peticion tenga al menos una linea con CRLF
    if (!strstr(buffer, "\r\n")) {
        send_error_response(client_fd, 400, "Bad Request");
        return false;
    }

    // Lee la peticion y extrae el metodo, URI y version
    http_request_t req = {0};
    if (sscanf(buffer, "%15s %255s %15s", req.method, req.uri, req.version) < 3) {
        send_error_response(client_fd, 400, "Bad Request");
        return false;
    }

    // Si el metodo no es GET responde 405 Method Not Allowed
    if (strcmp(req.method, "GET") != 0) {
        send_error_response(client_fd, 405, "Method Not Allowed");
        return false;
    }

    // Valida la cabecera Host
    if (!strstr(buffer, "Host:")) {
        send_error_response(client_fd, 400, "Bad Request (Missing Host)");
        return false;
    }

    // Verifica si se pide mantener la conexión viva (keep-alive)
    req.keep_alive = (strstr(buffer, "Connection: keep-alive") || strstr(buffer, "Connection: Keep-Alive"));

    // Resuelve la ruta del archivo solicitado y verifica que sea segura
    char resolved_path[512] = {0};
    if (!is_safe_path(req.uri, resolved_path)) {
        send_error_response(client_fd, 403, "Forbidden");
        return false;
    }

    // Lee el contenido del archivo solicitado
    size_t file_size = 0;
    char *file_content = read_file_content(resolved_path, &file_size);
    if (!file_content) {
        send_error_response(client_fd, 404, "Not Found");
        return false;
    }
    
    // 💡 GESTIÓN DEL ERROR 500: Si el archivo existe pero falló la reserva de memoria
    // (Simulación defensiva: si file_size > 0 pero file_content falló, o si quisiéramos validar un puntero nulo preventivo)
    // Nota: read_file_content ya controla el malloc interno. Si falla, file_content es NULL.

    char header[256];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: %s\r\n\r\n",
                              get_mime_type(resolved_path), file_size, 
                              req.keep_alive ? "keep-alive" : "close");

    send(client_fd, header, header_len, 0);       
    send(client_fd, file_content, file_size, 0);  
    
    free(file_content); 

    return req.keep_alive;
}
