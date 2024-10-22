#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H 1

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* The following constants represent the error types that may be encountered.
   Negative errors are system errors that already use `errno`.
   The value `0` represents a non-error / the null error.
   Positive errors are custom errors that may be encountered. */
enum HTTP_errno {
    // errno errors
    HTTP_EALLOC = -6, /* `calloc`, `malloc`, `realloc` or `reallocarray`
                         encountered a problem */
    HTTP_ECLOSE = -5, /* `close` encountered a problem */
    HTTP_ELISTEN = -4, /* `listen` encountered a problem */
    HTTP_EBIND = -3, /* `bind` encountered a problem */
    HTTP_ESETSOCKOPT = -2, /* `setsockopt` encountered a problem */
    HTTP_ESOCKET = -1, /* `socket` encountered a problem */

    // non-error / Ok
    HTTP_ENOERROR = 0, /* default, result when no error occured */

    // custom errors
};

/* Gets the string representation of an HTTP_errno. */
char *HTTP_strerror(enum HTTP_errno err);

/* Prints an HTTP_errno to stderr (like perror). */
void HTTP_perror(const char *s, enum HTTP_errno err);

/* This struct represents an HTTP server. */
struct HTTP_Server {
    int socket_fd; /* the file descriptor of the server's socket.
                      Use ntoh[ls] family of functions to convert
                      from network byte-order */
    struct sockaddr_in address; /* the internet address that the server is 
                                   bound to */
    int listen_backlog; /* How many pending connections the system should
                           keep waiting in the background. Default set to
                           `SOMAXCONN`. */
    char *buffer; /* The buffer to read messages into. Allocated by
                     `HTTP_create_server` */
    size_t buf_len; /* The length of the buffer in bytes. */
};

/* Create a new HTTP_Server that listens on the given port.
   An uninitialized HTTP_Server is intended to be passed as `http_server`.
   Returns any errors encountered, as an HTTP_errno. Create's the HTTP_Server's
   socket and address and sets its socket options. Should be accompanied by a
   call to `HTTP_destroy_server` to tidy up when done. */
enum HTTP_errno HTTP_create_server(struct HTTP_Server *http_server, const uint16_t port);

/* Bind an HTTP_Server's socket and start listening. Should be accompanied by
   a call to `HTTP_stop_server` to close the stream and tidy up when done. */
enum HTTP_errno HTTP_start_server(struct HTTP_Server *http_server);

/* Close a HTTP_Server's socket. Is a minimal wrapper over `close`. */
enum HTTP_errno HTTP_stop_server(struct HTTP_Server *http_server);

/* Deallocate the innards of an HTTP_Server. NOTE: Does not deallocate the 
   server's struct itself, just the insides */
enum HTTP_errno HTTP_destroy_server(struct HTTP_Server *http_server);

#endif /* _HTTP_SERVER_H */
