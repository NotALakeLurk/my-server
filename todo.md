# short-term: polish sockets stuff (easy)
- get rid of the start/stop mechanics
    - combine `HTTP_start_server` back into `HTTP_create_server` (perhaps
    refactoring names?).
- refactor `HTTP_*` functions to better fit `C`-style naming conventinos (=|:3)
    - look at `getaddrinfo` and `freeaddrinfo` for inspiration?
    - `C` likes "free" rather than "destroy", "create" may be fine?

# medium-term: start doing HTTP stuff
- use `<string.h>` functions to find headers in received HTTP request
- dynamically allocated string for `start-line` and `headers`
    - each part gets an index and length for its value?
- body gets separate dynamically allocated buffer?
    - pros: 
        + better separation
        + easier to write entire buffer to file?
        + easier to hand off separate allocation block to other processes when
        the header block may be deallocated for some reason?
    - cons:
        * not sure of cons yet

# medium-term: README and LICENSE
