#include <stdio.h>
#include <http.h>

#include <sys/socket.h> // temporary
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

int main(int argc, char **argv) {
    struct HTTP_Server server;

    HTTP_create_server(&server)
    /*
    struct HTTP_Server server;
    enum HTTP_errno err;

    err = HTTP_create_server(&server, 8080u);

    err = HTTP_start_server(&server);

    // ????????????
    
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    
    #define BUFFER_SIZE 1024
    char buffer[BUFFER_SIZE];

    // accept an incoming connection
    for (;;) {
        int client_sockfd = accept(
            server.socket_fd,
            (struct sockaddr*)&client_address,
            &client_len
        );
        if (client_sockfd == -1) {
            perror("error: failed to accept connection");
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "server: received connection with %X\r\n", ntohl(client_address.sin_addr.s_addr));

        ssize_t bytes_read = recv(client_sockfd, &buffer, BUFFER_SIZE, 0);
        switch (bytes_read) {
        case 0:
            close(client_sockfd);
            continue;
        case -1:
            perror("error: failed to recv data");
            close(client_sockfd);
            HTTP_destroy_server(&server);
            exit(EXIT_FAILURE);
        default:
            break;
        }
        
        // print received message
        for (ssize_t i = 0; i < bytes_read; ++i)
            fputc(buffer[i], stdout);

        char message[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf=8\r\n\r\n<!DOCTYPE html><html><head><title>test</title></head><body><h1>no, lol</h1></body></html>\r\n";
        
        send(client_sockfd, &message, sizeof(message), 0);

        close(client_sockfd);
    }

    err = HTTP_stop_server(&server);
    err = HTTP_destroy_server(&server);

    return EXIT_SUCCESS;
    */
}
