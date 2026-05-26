#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int init_server_socket(int port) {
    struct addrinfo hints, *result, *p;
    int socketfd, err;
    char port_str[6];

    // Convertir el puerto numérico a cadena para getaddrinfo
    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;        
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_flags = AI_PASSIVE;      

    // Llamada a getaddrinfo 
    err = getaddrinfo(NULL, port_str, &hints, &result);
    if (err != 0) {
        fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(err));
        return -1;
    }

    // Recorrer la lista devuelta por getaddrinfo hasta que funcione el bind
    for (p = result; p != NULL; p = p->ai_next) {
        socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socketfd == -1) {
            continue;
        }

        // Evita "Address already in use" al reiniciar el servidor rápidamente
        int optval = 1;
        setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        // Intentar enlazar el socket al puerto
        if (bind(socketfd, p->ai_addr, p->ai_addrlen) == 0) {
            break; // si se logra el bind, se sale del for
        }

        // Si falló el bind, se cierra este socket
        close(socketfd);
    }

    // Liberar la memoria dinámica asignada por getaddrinfo
    freeaddrinfo(result);

    // Poner el socket en modo escucha (pasivo)
    if (listen(socketfd, BACKLOG) == -1) {
        perror("Error en listen");
        close(socketfd);
        return -1;
    }

    return socketfd;
}

int make_socket_non_blocking(int socketfd) {
    int flags = fcntl(socketfd, F_GETFL, 0);
    
    // Añadir el flag de no-bloqueante
    flags |= O_NONBLOCK;
    if (fcntl(socketfd, F_SETFL, flags) == -1) {
        perror("Error al configurar flags con fcntl");
        return -1;
    }
    return 0;
}