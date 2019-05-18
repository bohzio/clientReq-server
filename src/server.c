#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "errExit.h"
#include "request_response.h"

char *pathServerFifo ="/tmp/FIFOSERVER";
char *pathClietFifo ="/tmp/CLIENTFIFO";
int serverFIFO, serverFIFO_extra;


int main (int argc, char *argv[]) {

    printf("Hi, I'm Server program!\n");

    printf("<Server> Making FIFO...\n");
    // make a FIFO with the following permissions:
    // user:  read, write
    // group: write
    // other: no permission
    if (mkfifo(pathServerFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        //errExit("mkfifo failed"); commento questa riga perchè fin che sviluppo non voglio che esca ma utilizzo quella già esistente

    printf("<Server> FIFO %s created!\n", pathServerFifo);


    printf("<Server> waiting for a client...\n");
    serverFIFO = open(pathServerFifo, O_RDONLY);
    if (serverFIFO == -1)
        errExit("open read-only failed");

    // Open an extra descriptor, so that the server does not see end-of-file
    // even if all clients closed the write end of the FIFO
    serverFIFO_extra = open(pathServerFifo, O_WRONLY);
    if (serverFIFO_extra == -1)
        errExit("open write-only failed");



    struct Request request;
    int bR = -1;

    do {
        printf("<Server> waiting for a Request...\n");
        // Read a request from the FIFO
        bR = read(serverFIFO, &request, sizeof(struct Request));

        // Check the number of bytes read from the FIFO
        if (bR == -1) {
            printf("<Server> it looks like the FIFO is broken\n");
        } else if (bR != sizeof(struct Request) || bR == 0)
            printf("<Server> it looks like I did not receive a valid request\n");
        else{
            printf("%s\n", request.idUser);



            if(strcmp(request.service, "stampa") != 0){

            }else if(request.service == "salva"){

            }else if(request.service == "invia"){

            }else{
                printf("servizio non riconosciuto");
            }

            printf("%s\n", request.service);
        }

    } while (bR != -1);


    return 0;
}
