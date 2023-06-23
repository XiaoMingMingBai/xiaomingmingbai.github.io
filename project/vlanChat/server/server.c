
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <ws2tcpip.h>
// #include <arpa/inet.h>
#include <windows.h>
#pragma comment(lib, "wsock32.lib")
#include <unistd.h>

#define PORT 12345
#define MAX_CLIENTS 10

int main()
{
    // 创建socket对象
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("创建socket对象失败");
        exit(EXIT_FAILURE);
    }

    // 设置socket选项
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("设置socket选项失败");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // 绑定socket到指定端口
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("绑定socket失败");
        exit(EXIT_FAILURE);
    }

    // 监听socket
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("监听socket失败");
        exit(EXIT_FAILURE);
    }

    printf("等待客户端连接...\n");

    int client_sockets[MAX_CLIENTS];
    int num_clients = 0;

    while (1)
    {
        // 接收客户端连接请求
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1)
        {
            perror("接收客户端连接失败");
            exit(EXIT_FAILURE);
        }

        printf("客户端 %s:%d 连接成功\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 向客户端发送欢迎消息
        const char *welcome_message = "欢迎来到局域网聊天室！\n";
        if (send(client_socket, welcome_message, strlen(welcome_message), 0) == -1)
        {
            perror("向客户端发送消息失败");
            exit(EXIT_FAILURE);
        }

        client_sockets[num_clients++] = client_socket;

        // 接收客户端发送的消息
        char buffer[1024];
        int n;

        while (1)
        {
            n = recv(client_socket, buffer, sizeof(buffer), 0);
            if (n == -1)
            {
                perror("接收消息失败");
                exit(EXIT_FAILURE);
            }
            else if (n == 0)
            {
                printf("客户端 %s:%d 已经退出\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                close(client_socket);
                break;
            }
            else
            {
                buffer[n] = '\0';
                printf("客户端 %s:%d 说：%s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

                // 将消息广播给所有客户端
                for (int i = 0; i < num_clients; i++)
                {
                    if (client_sockets[i] != client_socket)
                    {
                        if (send(client_sockets[i], buffer, n, 0) == -1)
                        {
                            perror("向客户端发送消息失败");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        }
    }

    // 关闭服务器socket
    close(server_socket);

    return 0;
}