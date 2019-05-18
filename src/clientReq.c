#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "request_response.h"
#include "errExit.h"

char *path2ServerFIFO = "/tmp/fifo_server";
char *baseClientFIFO = "/tmp/fifo_client.";

int main (int argc, char *argv[]) {
    char idUser[10]; //salvo l'id dell'utente
    char service[5]; //salvo il servizio
    printf("Hi, I'm ClientReq program!\nPuoi scegliere uno dei seguenti servizi:\n -stampa \n -salva \n -invia \n");

    printf("Inserisci id utente (max 10 caratteri): ");
    scanf("%9s", idUser);

    printf("Inserisci per esteso il nome del servizion desiderato: ");
    scanf("%9s", service);







    return 0;
}
