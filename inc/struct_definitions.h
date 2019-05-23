#ifndef _REQUEST_RESPONSE_HH
#define _REQUEST_RESPONSE_HH

#include <sys/types.h>

/*
 * Request (client --> server)
 */
struct Request {
    char idUser[11]; //id utente
    char service[6]; //nome del servizio
    int clientPid; //pid del client
};

/*
 * Response (server --> client)
 */
struct Response {
    int key; //codice per l'uso del servizio
};


struct SharedItem {
    char idUser[11];
    int key;
    time_t timestamp;
};

#endif
