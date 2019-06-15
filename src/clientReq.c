#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "struct_definitions.h"
#include "errExit.h"

char *pathServerFifo = "/tmp/FIFOSERVER";
char *basePathClientFifo = "/tmp/CLIENTFIFO.";;

int clientFIFO;


int main(int argc, char *argv[]) {

    //The client opens the server's FIFO
    printf("<Client> opening server FIFO %s...\n", pathServerFifo);
    fflush(stdout);

    int serverFIFO = open(pathServerFifo, O_WRONLY);
    if (serverFIFO == -1)
        errExit("open server fifo failed. It seems that the server is down");


    struct Request request;

    printf("Hi, I'm ClientReq program!\n");

    printf("Inserisci id utente (max 10 caratteri): ");
    scanf(" %10s", request.idUser);
    int c;
    while((c = getchar()) != '\n' && c != EOF); //discard extra input character



    printf("Puoi scegliere uno dei seguenti servizi:\n -stampa \n -salva \n -invia \n");

    printf("Inserisci per esteso il nome del servizion desiderato: ");
    scanf("%6s", request.service);
    while((c = getchar()) != '\n' && c != EOF); //discard extra input character

    request.clientPid = getpid();


    char pathClientFifo[25];
    sprintf(pathClientFifo, "%s%d", basePathClientFifo, getpid());

    printf("<CLient> Making FIFO...\n");
    // make a FIFO with the following permissions:
    // user:  read, write
    // group: write
    // other: no permission
    if (mkfifo(pathClientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        //errExit("mkfifo failed"); commento questa riga perchè fin che sviluppo non voglio che esca ma utilizzo quella già esistente

        printf("<Client> FIFO %s created!\n", pathClientFifo);


    // Step-3: The client sends a Request through the server's FIFO
    printf("<Client> sending ... \n");
    if (write(serverFIFO, &request,
              sizeof(struct Request)) != sizeof(struct Request))
        errExit("write failed");


    // Step-4: The client opens its FIFO to get a Response
    clientFIFO = open(pathClientFifo, O_RDONLY);
    if (clientFIFO == -1)
        errExit("open read-only failed");


    // Step-5: The client reads a Response from the server
    struct Response response;
    if (read(clientFIFO, &response,
             sizeof(struct Response)) != sizeof(struct Response))
        errExit("read failed");


    printf("L'utente è : %s\n",request.idUser);
    printf("La key è : %d \n", response.key);


    // Step-7: The client closes its FIFO
    if (close(serverFIFO) != 0 || close(clientFIFO) != 0)
        errExit("close failed");

    // Step-8: The client removes its FIFO from the file system
    if (unlink(pathClientFifo) != 0)
        errExit("unlink failed");

    return 0;
}
