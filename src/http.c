#include <sys/socket.h>
#include <unistd.h> // close
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h> // only included for strerror, *ugh*
    
// normal errno errors are negative for convenience!
// non-errors are 0
// custom errors are positive
enum HTTP_ErrType {
    // errno errors
    HTTP_EMALLOC = -6,
    HTTP_ECLOSE = -5,
    HTTP_ELISTEN = -4,
    HTTP_EBIND = -3,
    HTTP_ESETSOCKOPT = -2,
    HTTP_ESOCKET = -1,

    // non-error / Ok
    HTTP_ENOERROR = 0, // default, result when no error occured

    // special errors
    HTTP_EAIERROR
};

union HTTP_ErrInfo {
    int errno_val; // errno at the time of this error's creation
    int ai_error; // the error number returned by `getaddrinfo`
};

// a `tagged union` where `err_type` indicates which member of `err_info` can
// be accessed for more information
struct HTTP_Error {
    enum HTTP_ErrType err_type;
    union HTTP_ErrInfo err_info;
};

// not exposed in api
// the strings for our errors
char *HTTP_ENOERROR_STRING = "No error occured";

// get the string representation of an error
const char *HTTP_strerror(const struct HTTP_Error err) {
    if (err.err_type < 0) // error is of the `errno` variety
        return strerror(err.err_info.errno_val);

    switch (err.err_type) {
        case HTTP_ENOERROR: // handle non-errors
            return HTTP_ENOERROR_STRING;
        case HTTP_EAIERROR: // handle `getaddressinfo` errors
            return gai_strerror(err.err_info.ai_error);
        break;
    }
}

// print an HTTP_errno to stderr (like perror)
void HTTP_perror(const char *message, const struct HTTP_Error err) {
    fprintf(stderr, "%s: %s", message, HTTP_strerror(err));
}

struct HTTP_Server {
    int socket_fd;

    struct addrinfo *addrinfo; // the current address of the server (result of
                               // `getaddrinfo`, is linked list, may be invalid)
    struct addrinfo *_addrinfo_free_ptr; // pointer to free the result linked-
                                         // list of `getaddrinfo`

    int listen_backlog;

    char *buffer;
    size_t buf_len;
};

const int HTTP_SETSOCKOPT_ENABLE = 1; // int for setsockopt to get pointed to

// create the server's socket and address and set its socket options
struct HTTP_Error HTTP_create_server(
    struct HTTP_Server *http_server,
    const uint16_t port,
    const size_t buffer_size
) {

    struct HTTP_Error error; // create an error struct to return
    memset(&error, 0, sizeof error); // zero out struct

    // a lot of this information came from
    // https://beej.us/guide/bgnet/html//index.html
    
    http_server->listen_backlog = SOMAXCONN; // backlog for `listen` call
    
    // set server to be at 0.0.0.0:<given port>
    struct addrinfo hints, *res_addr;
    memset(&hints, 0, sizeof hints); // zero out struct
    
    hints.ai_family = AF_UNSPEC; // either ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM; // stream for tcp
    hints.ai_flags = AI_PASSIVE; // want to bind a socket to accept connections
    
    //http_server->address.sin_family = AF_INET;
    //http_server->address.sin_addr.s_addr = htonl(INADDR_ANY); // should be 0.0.0.0 (broadcast/any)
    //http_server->address.sin_port = htons(port);
    
    status = getaddrinfo(NULL, "8080", &hints, &res_addr);
    if (status != 0) {
        // return error
        error.err_type = HTTP_EAIERROR;
        error.err_info.ai_error = status;
        return error;
    }
    // keep in mind that the returned addrinfo may not contain a valid address
    http_server->addrinfo = res_addr;
    http_server->_addrinfo_free_ptr = res_addr;

    // configure the socket for the server
    
    // taken from `man getaddrinfo`:
    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */
    // in this case, we're not giving much error other than failed to configure
    // socket, unfortunately
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        http_server->socket_fd = socket(
            rp->ai_family,
            rp->ai_socktype,
            rp->ai_protocol
        );
        if (http_server->socket_fd == -1)
            continue;

        // set socket options
        // REUSEADDR: 
        if (setsockopt(
            http_server->socket_fd,
            SOL_SOCKET,
            SO_REUSEADDR, // make optional?
            &HTTP_SETSOCKOPT_ENABLE,
            sizeof(HTTP_SETSOCKOPT_ENABLE)
        ) == -1) {
            close(http_server->socket_fd);
            continue;
        }
 
        if (bind(http_server->socket_fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */
 
        close(http_server->socket_fd);
    }
 
    freeaddrinfo(result);           /* No longer needed */

    // create a socket for the server
    http_server->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (http_server->socket_fd == -1) {
        error.err_type = HTTP_ESOCKET;
        error.err_info.errno_val = errno;
        return error;
    }

    // allocate the read buffer
    http_server->buf_len = buffer_size;
    http_server->buffer = malloc(buffer_size); // buffer_size is length of buffer in bytes
    
    if (http_server->buffer == NULL) {
        error.err_type = HTTP_EMALLOC;
        error.err_info.errno_val = errno;
        return error;
    }

    // return HTTP_ENOERROR
    return error;
}

/* Bind an HTTP_Server's socket and start listening. Should be accompanied by
   a call to `HTTP_stop_server` to close the stream and tidy up when done. */
struct HTTP_Error HTTP_start_server(struct HTTP_Server *http_server) {
    // TODO: create socket if it is null (if server is restarting)
    // HTTP_create_socket(http_server) as a general solution might be better?

    struct HTTP_Error error; // create an error struct to return
    memset(&error, 0, sizeof error); // zero out struct
    
    // finally bind the socket
    if (bind(
        http_server->socket_fd,
        (struct sockaddr*)&http_server->address,
        sizeof(http_server->address) // i'm pretty sure sizeof works here because
                                     // we know the type of http_server->address
    ) == -1) {
        error.err_type = HTTP_EBIND;
        error.err_info.errno_val = errno;
        return error;
    }

    // start listening
    if (listen(http_server->socket_fd, http_server->listen_backlog) == -1) {
        error.err_type = HTTP_ELISTEN;
        error.err_info.errno_val = errno;
        return error;
    }

    // success, most outputs go through *http_server
    return error;
}

/* Close an HTTP_Server's socket. Is a minimal wrapper over `close`. */
struct HTTP_Error HTTP_stop_server(struct HTTP_Server *http_server) {
    struct HTTP_Error error; // create an error struct to return
    memset(&error, 0, sizeof error); // zero out struct

    int err = close(http_server->socket_fd);
    if (
        err == -1 && // if: there was an error, and
        errno != EBADF // socket_fd existed
    ) { // return that err (otherwise it's expected/okay)
        error.err_type = HTTP_ECLOSE;
        error.err_info.errno_val = errno;
        return error;
    } 

    return error;
}

/* Deallocate the innards of an HTTP_Server. NOTE: Does not deallocate the 
   server's struct itself, just the insides */
struct HTTP_Error HTTP_destroy_server(struct HTTP_Server *http_server) {
// this will be more important once more logic comes in
    struct HTTP_Error error; // create an error struct to return
    memset(&error, 0, sizeof error); // zero out struct

    // TODO: call HTTP_stop_server if necessary.
    error = HTTP_stop_server(http_server);
    if (error.err_type != HTTP_ENOERROR)
        return error;

    // deallocate the previously allocated buffer
    free(http_server->buffer);

    // deallocate the addrinfo linked list provided by `getaddrinfo`
    freeaddrinfo(http_server->_addrinfo_free_ptr);

    return error;
}
/*
    #define BUFFER_SIZE 1024
    char buffer[BUFFER_SIZE];

    // accept an incoming connection
    for (;;) {
        int client_sockfd = accept(
            server_sockfd,
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
            close(server_sockfd);
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

    return EXIT_SUCCESS;
*/
