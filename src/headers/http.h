#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H 1

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* The following constants represent the error types that may be encountered.
   Negative errors are system errors that already use `errno`.
   The value `0` represents a non-error / the null error.
   Positive errors are custom errors that may be encountered. */
enum HTTP_ErrType {
    // errno errors
    HTTP_ECLOSE = -5, /* `close` encountered a problem */
    HTTP_ELISTEN = -4, /* `listen` encountered a problem */
    HTTP_EALLOC = -1, /* `calloc`, `malloc`, `realloc` or `reallocarray`
                         encountered a problem */

    // non-error / Ok
    HTTP_ENOERROR = 0, /* default, result when no error occured */

    // custom errors
    HTTP_EAIERROR, /* `gaddrinfo` returned an error, access err_info.ai_error */
    HTTP_ESOCKET, /* failed to configure any sockets for server */
};

/* Provides additional information about the error depending on the ErrType */
union HTTP_ErrInfo {
    int errno_val; /* errno at the time of this error's creation */
    int ai_error; /* the error number returned by `getaddrinfo` */
};

/* A `tagged union` where `err_type` indicates which error occured, and
   `err_info` can be accessed for more information */
struct HTTP_Error {
    enum HTTP_ErrType err_type;
    union HTTP_ErrInfo err_info;
};

/* Gets the string representation of an HTTP_Error. */
const char *HTTP_strerror(const struct HTTP_Error err);

/* Prints an HTTP_Error to stderr (like perror). */
void HTTP_perror(const char *message, const struct HTTP_Error err);

/* A representation of an HTTP server. */
struct HTTP_Server {
    int socket_fd; /* the file descriptor of the server's socket. */
    int listen_backlog; /* How many pending connections the system should
                           keep waiting in the background. Default set to
                           `SOMAXCONN`. */
    char *buffer; /* The buffer to read messages into. Allocated by
                     `HTTP_create_server`. */
    size_t buf_len; /* The length of the buffer in bytes. */
};

/* Create a new HTTP_Server and listens on the given port.
   An uninitialized HTTP_Server is intended to be passed as `http_server`.
   Returns any errors encountered as an HTTP_Error. Should be accompanied by a
   call to `HTTP_destroy_server` to tidy up when done.
   `port` may be a protocol (such as "HTTP") or the string representation of a
   port number (such as "8080"), see `man getaddrinfo`. */
struct HTTP_Error HTTP_create_server(
    struct HTTP_Server *http_server,
    const char *port,
    const size_t buffer_size
);

/* Deallocate the innards of an HTTP_Server. NOTE: Does not deallocate the 
   server's struct itself, just the insides. */
struct HTTP_Error HTTP_destroy_server(struct HTTP_Server *http_server);

#endif /* _HTTP_SERVER_H */
