// manager.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    while (1) {
        // マネージャー側でコマンドの入力
        printf("Enter command to execute: ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0';  // 改行文字を削除

        // コマンドをエージェントに送信
        send(client_socket, command, strlen(command), 0);

        // エージェントからの結果を受信
        while (recv(client_socket, buffer, BUFFER_SIZE, 0) > 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            if (strcmp(buffer, "END_OF_RESULT") == 0) {
                break;
            }
            printf("%s\n", buffer);
        }

        printf("Command execution finished\n");
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    // ソケットの作成
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // ソケットにアドレスをバインド
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // リスニング
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connections...\n");
    addr_size = sizeof(client_addr);

    // クライアントからの接続を受け入れる
    while ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size)) >= 0) {
        printf("Connection accepted\n");
        if (fork() == 0) {  // 子プロセスを作成
            close(server_socket);  // 子プロセスでは親のリスニングソケットを閉じる
            handle_client(client_socket);
            exit(0);  // 子プロセスを終了
        }
        close(client_socket);  // 親プロセスではクライアントソケットを閉じる
    }

    if (client_socket < 0) {
        perror("Accept failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    close(server_socket);
    return 0;
}
