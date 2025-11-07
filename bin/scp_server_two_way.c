#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void create_scp_message(char *buffer, const char *type, int id, const char *payload) {
    sprintf(buffer, "SCP/1.1 | %s | id=%d | %s", type, id, payload);
}

void parse_scp_message(const char *buffer, char *type, int *id, char *payload) {
    sscanf(buffer, "SCP/1.1 | %s | id=%d | %[^\n]", type, id, payload);
}

unsigned __stdcall receive_thread(void *socket_desc) {
    SOCKET client = *(SOCKET *)socket_desc;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        printf("[SERVER] RECV: %s\n", buffer);
        char msg_type[50], payload[BUFFER_SIZE];
        int msg_id;
        parse_scp_message(buffer, msg_type, &msg_id, payload);

        if (strcmp(msg_type, "HELLO") == 0) {
            char ack[BUFFER_SIZE];
            create_scp_message(ack, "ACK", msg_id, "CONNECTION_OK");
            send(client, ack, strlen(ack), 0);
            printf("[SERVER] SEND: %s\n", ack);
        } else if (strcmp(msg_type, "MSG") == 0) {
            printf("Client says: %s\n", payload);
            char ack[BUFFER_SIZE];
            create_scp_message(ack, "ACK", msg_id, "MSG_RECEIVED");
            send(client, ack, strlen(ack), 0);
        }
    }
    closesocket(client);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server, client;
    struct sockaddr_in server_addr, client_addr;
    int c;
    HANDLE thread;

    printf("-------------------------------------------\n");
    printf("  SCP SERVER v1.1 (Windows Dev-C++)\n");
    printf("-------------------------------------------\n\n");

    WSAStartup(MAKEWORD(2, 2), &wsa);
    server = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server, 1);

    printf("?? Server listening on port %d...\n", PORT);
    c = sizeof(struct sockaddr_in);
    client = accept(server, (struct sockaddr *)&client_addr, &c);

    printf("? Client connected!\n\n");

    thread = (HANDLE)_beginthreadex(0, 0, receive_thread, (void *)&client, 0, 0);

    while (1) {
        char msg[BUFFER_SIZE];
        printf("You: ");
        fgets(msg, BUFFER_SIZE, stdin);
        msg[strcspn(msg, "\n")] = 0;

        char scp_msg[BUFFER_SIZE];
        create_scp_message(scp_msg, "MSG", rand() % 1000, msg);
        send(client, scp_msg, strlen(scp_msg), 0);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
