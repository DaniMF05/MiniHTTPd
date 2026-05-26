
# MiniHTTPd (epoll) — servidor HTTP estático minimalista

- Creado por Joshua Daniel Menendez Farias

Este proyecto implementa un servidor HTTP **muy pequeño** (single-thread) para servir **archivos estáticos** desde el directorio `www/`, usando **sockets no bloqueantes** y **`epoll`** (Linux) para manejar múltiples clientes de forma eficiente.

## Características

- **I/O no bloqueante** + event loop con `epoll`.
- **Concurrencia**: múltiples conexiones en un único hilo (sin `fork()`/threads).
- **HTTP/1.1 básico**:
	- Soporta **solo `GET`**.
	- Valida cabecera **`Host:`**.
	- Maneja `Connection: keep-alive` (de forma simple).
- **Servidor de archivos estáticos** desde `./www`:
	- Mapea `/` → `/index.html`.
	- Lee archivos en modo binario (sirve HTML e imágenes).
- **Tipos MIME** por extensión: `.html/.htm`, `.css`, `.js`, `.png`, `.jpg/.jpeg` (resto: `application/octet-stream`).
- **Logs por stdout**: muestra conexiones, desconexiones y keep-alive.
- **Mitigación de directory traversal** mediante canonicalización con `realpath()`.

## Requisitos

- Linux (WSL Ubuntu también sirve). Este proyecto usa **`epoll`**, por lo que **no** compila/funciona igual en Windows nativo o macOS.
- `gcc` y `make`.

## Compilación

Desde la raíz del proyecto:

```bash
make
```

Para limpiar objetos y binario:

```bash
make clean
```

## Ejecución

Ejecuta el binario desde la **raíz del proyecto** (importante porque el docroot está codificado como `./www`):

```bash
./minihttpd
```

Salida esperada:

```text
Servidor MiniHTTPd corriendo en el puerto 8080...
```

Luego abre en el navegador:

- `http://localhost:8080/`

## Configuración (en código)

Este proyecto no expone flags por CLI; los parámetros principales están definidos como constantes:

- Puerto por defecto: `DEFAULT_PORT = 8080` en `include/server.h`.
- Backlog de `listen()`: `BACKLOG = 128` en `include/server.h`.
- Tamaño máximo del buffer de request: `MAX_BUFFER = 4096` en `include/http.h`.
- Máximo de eventos por `epoll_wait`: `MAX_EVENTS = 64` en `src/main.c`.
- Document root: `WEB_ROOT = "./www"` en `include/files.h`.

## Estructura del proyecto

```text
.
├── Makefile
├── include/
│   ├── files.h
│   ├── http.h
│   ├── mime.h
│   └── server.h
├── src/
│   ├── files.c
│   ├── http.c
│   ├── main.c
│   ├── mime.c
│   └── server.c
├── www/
│   ├── index.html
│   ├── image.png
│   └── style.css
└── obj/            (generado por make)
```

## Arquitectura y flujo

### Componentes

- **`src/main.c`**: inicializa el servidor, crea `epoll`, acepta conexiones y despacha solicitudes a `handle_client_request()`.
- **`src/server.c`**: crea el socket, hace `bind()`/`listen()` y pone FDs en modo no bloqueante.
- **`src/http.c`**: parsea una petición HTTP mínima y arma la respuesta (cabeceras + cuerpo).
- **`src/files.c`**: valida rutas (evita escapes del docroot) y lee archivos.
- **`src/mime.c`**: decide `Content-Type` por extensión.

### Ciclo `epoll`

1. Se crea el socket de servidor y se configura como **no bloqueante**.
2. Se crea una instancia `epoll`.
3. Se registra el FD del servidor con `EPOLLIN`.
4. En el loop:
	 - Si el evento viene del FD del servidor: `accept()` + registrar el cliente.
	 - Si el evento viene de un cliente: se procesa una petición con `handle_client_request()`.
5. Si el handler no devuelve keep-alive, se cierra el socket del cliente.

## Comportamiento HTTP

### Método

- **Solo** `GET`.
- Cualquier otro método responde `405 Method Not Allowed`.

### Request mínima válida

Debe existir al menos una línea con `\r\n`, y debe incluir cabecera `Host:`.

Ejemplo:

```http
GET / HTTP/1.1
Host: localhost


```

### Keep-Alive

El servidor interpreta keep-alive **solo** si encuentra exactamente:

- `Connection: keep-alive` o `Connection: Keep-Alive`

Si no está presente, responde con `Connection: close` y el proceso principal cierra el socket.

## Códigos de estado

- `200 OK`: archivo encontrado y servido correctamente.
- `400 Bad Request`: request mal formada (por ejemplo, sin CRLF, sin `Host:`, o línea inicial incompleta).
- `403 Forbidden`: ruta insegura o inválida (ver sección de seguridad).
- `404 Not Found`: no se pudo leer el archivo (por ejemplo, fallo al abrir/leer).
- `405 Method Not Allowed`: método distinto de `GET`.
- `500 Internal Server Error`: error inesperado (por ejemplo, fallo al asignar memoria).
