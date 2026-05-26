#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"
#include "http.h"

#define MAX_EVENTS 64

int main() {
    int server_fd, epoll_fd;
    struct epoll_event ev, events[MAX_EVENTS];

    // Se inicia el socket del servidor y se enlaza al puerto por defecto (8080)
    server_fd = init_server_socket(DEFAULT_PORT);
    if (server_fd == -1) {
        fprintf(stderr, "Fallo crítico al iniciar el socket del servidor.\n");
        return EXIT_FAILURE;
    }

    // Se hace el socket del servidor no-bloqueante
    if (make_socket_non_blocking(server_fd) == -1) {
        close(server_fd);
        return EXIT_FAILURE;
    }

    // Se crea la instancia de epoll
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Error en epoll_create1");
        close(server_fd);
        return EXIT_FAILURE;
    }


    // Se registra el socket del servidor en epoll para monitorear lecturas (EPOLLIN)
    ev.events = EPOLLIN; 
    ev.data.fd = server_fd; // Guardamos el FD para saber quién generó el evento
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("Error en epoll_ctl: server_fd");
        close(server_fd);
        close(epoll_fd);
        return EXIT_FAILURE;
    }

    printf("Servidor MiniHTTPd corriendo en el puerto %d...\n", DEFAULT_PORT);

    // BUCLE PRINCIPAL: Espera eventos de red y los maneja con epoll
    while (true) {

        // epoll_wait se bloquea de forma eficiente hasta que ocurra un evento en la red
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Error en epoll_wait");
            break; 
        }

        // Iterar por todos los descriptores de archivo que tienen novedades
        for (int i = 0; i < nfds; i++) {
            
            if (events[i].data.fd == server_fd) {
                
                // CASO A: Llegó una nueva conexión de un cliente al socket principal
                struct sockaddr in_addr;
                socklen_t in_len = sizeof(in_addr);
                
                int client_fd = accept(server_fd, &in_addr, &in_len);
                if (client_fd == -1) {
                    perror("Error en accept");
                    continue;
                }

                // Hacer el socket del cliente no-bloqueante
                if (make_socket_non_blocking(client_fd) == -1) {
                    close(client_fd);
                    continue;
                }

                // Registrar el nuevo cliente en epoll para leer sus peticiones HTTP
                ev.events = EPOLLIN | EPOLLET; // Usamos Edge-Triggered (EPOLLET) para rendimiento
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                    perror("Error en epoll_ctl: client_fd");
                    close(client_fd);
                }
                
                printf("CLIENTE CONECTADO || FD: %d\n", client_fd);

            } else {
                // CASO B: Un cliente ya conectado nos envió datos
                int client_fd = events[i].data.fd;
                
                // Le pasamos el control al módulo HTTP y nos dice si quiere persistencia
                bool mantener_vivo = handle_client_request(client_fd);
                
                if (!mantener_vivo) {
                    // Si no pidió keep-alive, lo sacamos de epoll y cerramos
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(client_fd);
                    printf("CLIENTE DESCONECTADO || FD: %d\n", client_fd);
                } else {
                    printf("CONEXIÓN PERSISTENTE || FD: %d\n", client_fd);
                }
            }
        }
    }

    // Limpieza al apagar el servidor
    close(server_fd);
    close(epoll_fd);
    return EXIT_SUCCESS;
}