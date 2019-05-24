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

int clientFIFO, clientFIFO_extra;


int main(int argc, char *argv[]) {


    struct Request request;

    printf("Hi, I'm ClientReq program!\nPuoi scegliere uno dei seguenti servizi:\n -stampa \n -salva \n -invia \n");

    printf("Inserisci id utente (max 10 caratteri): ");
    scanf("%s", request.idUser);

    printf("Inserisci per esteso il nome del servizion desiderato: ");
    scanf("%s", request.service);

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


    // Step-4: The client opens its FIFO to get a Response
    clientFIFO = open(pathClientFifo, O_RDONLY);
    if (clientFIFO == -1)
        errExit("open read-only failed");


    // Step-5: The client reads a Response from the server
    struct Response response;
    if (read(clientFIFO, &response,
             sizeof(struct Response)) != sizeof(struct Response))
        errExit("read failed");


    // Step-6: The client prints the result on terminal
    printf("La key è : %d", response.key);


    // Step-7: The client closes its FIFO
    if (close(serverFIFO) != 0 || close(clientFIFO) != 0)
        errExit("close failed");

    // Step-8: The client removes its FIFO from the file system
    if (unlink(pathClientFifo) != 0)
        errExit("unlink failed");

    return 0;
}
