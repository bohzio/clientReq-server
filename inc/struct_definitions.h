#ifndef _REQUEST_RESPONSE_HH
#define _REQUEST_RESPONSE_HH

#include <sys/types.h>

/*
 * Request (client --> server)
 */
struct Request {
    char idUser[11];
    char service[7];
    int clientPid;
};

/*
 * Response (server --> client)
 */
struct Response {
    int key;
};

struct SharedItem {
    char idUser[11];
    int key;
    time_t timestamp;
};

#endif
