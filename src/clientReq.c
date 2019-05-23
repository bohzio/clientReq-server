#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "struct_definitions.h"
#include "errExit.h"

char *pathServerFifo ="/tmp/FIFOSERVER";
char *basePathClientFifo ="/tmp/CLIENTFIFO.";;

int clientFIFO, clientFIFO_extra;


int main (int argc, char *argv[]) {


    char pathClientFifo[25];
    sprintf(pathClientFifo,"%s%d", basePathClientFifo, getpid());

    printf("<CLient> Making FIFO...\n");
    // make a FIFO with the following permissions:
    // user:  read, write
    // group: write
    // other: no permission
    if (mkfifo(pathClientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        //errExit("mkfifo failed"); commento questa riga perchè fin che sviluppo non voglio che esca ma utilizzo quella già esistente

        printf("<Client> FIFO %s created!\n", pathClientFifo);




    struct Request request;

    printf("Hi, I'm ClientReq program!\nPuoi scegliere uno dei seguenti servizi:\n -stampa \n -salva \n -invia \n");

    printf("Inserisci id utente (max 10 caratteri): ");
    scanf("%s", request.idUser);

    printf("Inserisci per esteso il nome del servizion desiderato: ");
    scanf("%s", request.service);

    request.clientPid = getpid();

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


    printf("<Client> waiting for the key...\n");
    clientFIFO = open(pathClientFifo, O_RDONLY);
    if (clientFIFO == -1)
        errExit("open read-only failed");

    // Open an extra descriptor, so that the server does not see end-of-file
    // even if all clients closed the write end of the FIFO
    clientFIFO_extra = open(pathClientFifo, O_WRONLY);
    if (clientFIFO_extra == -1)
        errExit("open write-only failed");

    struct Response response;
    int bR = -1;

    do {
        printf("<Client> waiting for a Response...\n");
        // Read a request from the FIFO
        bR = read(clientFIFO, &response, sizeof(struct Response));

        // Check the number of bytes read from the FIFO
        if (bR == -1) {
            printf("<Client> it looks like the FIFO is broken\n");
        } else if (bR != sizeof(struct Response) || bR == 0)
            printf("<Client> it looks like I did not receive a valid response\n");
        else{
           printf("Il codice key è :%d\n", response.key);


        }

    } while (bR != -1);




    return 0;
}
