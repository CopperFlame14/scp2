#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void create_scp_message(char *buffer, const char *type, int id, const char *payload) {
    sprintf(buffer, "SCP/1.1 | %s | id=%d | %s", type, id, payload);
}

void parse_scp_message(const char *buffer, char *type, int *id, char *payload) {
    sscanf(buffer, "SCP/1.1 | %s | id=%d | %[^\n]", type, id, payload);
}

unsigned __stdcall receive_thread(void *socket_desc) {
    SOCKET sock = *(SOCKET *)socket_desc;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;
        printf("[CLIENT] RECV: %s\n", buffer);
    }
    closesocket(sock);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server_addr;
    HANDLE thread;

    printf("-------------------------------------------\n");
    printf("  SCP CLIENT v1.1 (Windows Dev-C++)\n");
    printf("-------------------------------------------\n\n");

    WSAStartup(MAKEWORD(2, 2), &wsa);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed.\n");
        return 1;
    }

    printf("? Connected to SCP Server.\n\n");

    thread = (HANDLE)_beginthreadex(0, 0, receive_thread, (void *)&sock, 0, 0);

    // Send HELLO message
    char hello_msg[BUFFER_SIZE];
    create_scp_message(hello_msg, "HELLO", 1, "Client_Connected");
    send(sock, hello_msg, strlen(hello_msg), 0);

    while (1) {
        char msg[BUFFER_SIZE];
        printf("You: ");
        fgets(msg, BUFFER_SIZE, stdin);
        msg[strcspn(msg, "\n")] = 0;

        if (strcmp(msg, "bye") == 0) {
            char bye[BUFFER_SIZE];
            create_scp_message(bye, "BYE", rand() % 1000, "Disconnecting");
            send(sock, bye, strlen(bye), 0);
            break;
        }

        char scp_msg[BUFFER_SIZE];
        create_scp_message(scp_msg, "MSG", rand() % 1000, msg);
        send(sock, scp_msg, strlen(scp_msg), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
