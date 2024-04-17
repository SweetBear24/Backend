#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_PENDING 10
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("Received message from client: %s\n", buffer);
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int port = 0; // Номер свободного порта

    // Создание TCP сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настройка серверного адреса
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(0); // Присвоение любого свободного порта

    // Привязка сокета к адресу и порту
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Получение назначенного порта
    getsockname(server_socket, (struct sockaddr *)&server_addr, &client_addr_len);
    port = ntohs(server_addr.sin_port);
    printf("Server listening on port %d\n", port);

    // Начало прослушивания сокета
    if (listen(server_socket, MAX_PENDING) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Основной цикл сервера
    while (1) {
        // Принятие входящего подключения
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        // Создание дочернего процесса для обработки клиента
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            close(client_socket);
            continue;
        } else if (pid == 0) { // Дочерний процесс
            close(server_socket); // Закрываем дескриптор серверного сокета в дочернем процессе
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        } else { // Родительский процесс
            close(client_socket); // Закрываем дескриптор клиентского сокета в родительском процессе
        }
    }

    close(server_socket);

    return 0;
}
