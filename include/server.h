#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>

// Puerto por defecto si no se especifica uno
#define DEFAULT_PORT 8080
#define BACKLOG 128

// Inicializa el socket del servidor en el puerto indicado
int init_server_socket(int port);

// Configura un descriptor de archivo (socket) como no-bloqueante
int make_socket_non_blocking(int sfd);

#endif // SERVER_H