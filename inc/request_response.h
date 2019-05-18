#ifndef _REQUEST_RESPONSE_HH
#define _REQUEST_RESPONSE_HH

#include <sys/types.h>

/*
 * Request (client --> server)
 */
struct Request {
    char idUser[11]; //id utente
    char service[6]; //nome del servizio
};

/*
 * Response (server --> client)
 */
struct Response {
    char hash[20]; //codice per l'uso del servizio
};

#endif
