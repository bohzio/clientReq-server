#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "request_response.h"
#include "errExit.h"

char *pathServerFifo ="/tmp/FIFOSERVER";
char *pathClietFifo ="/tmp/CLIENTFIFO";;

int main (int argc, char *argv[]) {

    struct Request request;

    printf("Hi, I'm ClientReq program!\nPuoi scegliere uno dei seguenti servizi:\n -stampa \n -salva \n -invia \n");

    printf("Inserisci id utente (max 10 caratteri): ");
    scanf("%s", request.idUser);

    printf("Inserisci per esteso il nome del servizion desiderato: ");
    scanf("%s", request.service);


    // Step-2: The client opens the server's FIFO to send a Request
    printf("<Client> opening FIFO %s...\n", pathServerFifo);
    int serverFIFO = open(pathServerFifo, O_WRONLY);
    if (serverFIFO == -1)
        errExit("open failed");


    // Step-3: The client sends a Request through the server's FIFO
    printf("<Client> sending ... \n");
    if (write(serverFIFO, &request,
              sizeof(struct Request)) != sizeof(struct Request))
        errExit("write failed");






    return 0;
}
