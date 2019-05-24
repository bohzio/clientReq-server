#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/sem.h>
#include <semaphore.h>

#include "errExit.h"
#include "struct_definitions.h"
#include "shared_memory.h"

char *pathServerFifo = "/tmp/FIFOSERVER";
char *basePathClientFifo = "/tmp/CLIENTFIFO.";
char *fileFtokPath = "/tmp/fileFTOK";
int serverFIFO, serverFIFO_extra;

int totalEntries = 10;

int projid = 123;

struct SharedItem *entries;
int shmid;
int semid;

pid_t pid;

/**
 * Function that check if the service is ok
 * @param service
 * @return 0 if is ok 1 if is wrong
 */
int checkRequestService(char *service)
{
    if ((strncmp(service, "stampa", strlen("stampa")) == 0 || strncmp(service, "salva", strlen("salva")) == 0 || strncmp(service, "invia", strlen("invia")) == 0))
    {
        return 0;
    }
    return 1;
}


void sigHandler()
{

    // detach the shared memory segment
    printf("<Server> detaching the shared memory segment...\n");
    free_shared_memory(entries);

    // remove the shared memory segment
    printf("<Server> removing the shared memory segment[%d]...\n",shmid);
    remove_shared_memory(shmid);

    printf("<Server> remove file ftok\n");
    if (unlink(fileFtokPath) == -1){
        errExit("Remove of file ftok failed");
    }

    kill(pid, SIGTERM);

    exit(0);


}

void sendResponse(struct Request *request)
{

    int cont = 0;
    char pathClientFifo[25];
    sprintf(pathClientFifo, "%s%d", basePathClientFifo, request->clientPid);

    printf("%s\n", request->idUser);

    if (checkRequestService(request->service) == 0)
    {
        // Step-2: The client opens the server's FIFO to send a Request
        printf("<Server> opening FIFO %s...\n", pathClientFifo);
        int clientFIFO = open(pathClientFifo, O_WRONLY);
        if (clientFIFO == -1)
            errExit("open failed");


        semOp(semid, 0 , -1);

        for (int i = 0; i < totalEntries; i++)
        {
            if (entries[i].idUser[0] == 0)
            {
                //E' vuoto o è stato cancellato da processo keyserver
                //Setto il contatore a i così poi verrà scritto in quell'elemento
                cont = i;
                break;
            }
        }

        int timeStamp = time(0);

        strcpy(entries[cont].idUser, request->idUser);
        entries[cont].timestamp = timeStamp;

        int stampa = 3;
        int invia = 7;
        int salva = 11;
        int key = 0;

        if (strcmp(request->service, "stampa") == 0)
        {
            key = timeStamp + stampa;
        }
        else if (strcmp(request->service, "salva") == 0)
        {
            key = timeStamp + salva;
        }
        else if (strcmp(request->service, "invia") == 0)
        {
            key = timeStamp + invia;
        }

        struct Response response;

        entries[cont].key = key;
        response.key = key;

        semOp(semid, 0 , 1);

        // Step-3: The Server sends a Response through the client's FIFO
        printf("<Server> sending ... \n");
        if (write(clientFIFO, &response,
                  sizeof(struct Response)) != sizeof(struct Response))
            errExit("write failed");
    }
}


void setSignalHandler(){
    // set of signals (N.B. it is not initialized!)
    sigset_t mySet;
    // initialize mySet to contain all signals
    sigfillset(&mySet);
    // remove SITERM from mySet
    sigdelset(&mySet, SIGTERM);
    // blocking all signals but SIGTERM
    sigprocmask(SIG_SETMASK, &mySet, NULL);

    // set the function sigHandler as handler for the signal SIGINT
    if (signal(SIGTERM, sigHandler) == SIG_ERR)
        errExit("change signal handler failed");
}



key_t getKey(){
    open(fileFtokPath,O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

    return ftok(fileFtokPath,projid);
}


void sigHandlerChild(){
    printf("<Subprocess child> Killed\n");
    exit(0);
}

void child(){
    if(signal(SIGTERM, sigHandlerChild) == SIG_ERR)
        errExit("change signal handler child failed\n");


    while (1)
    {
        sleep(30);
        printf("<Subprocess child> checking shared memory\n");
        fflush(stdout); //todo studiare come funziona il fflush di preciso


        semOp(semid, 0 , -1);
        time_t now = time(0);
        for (int i = 0; i < totalEntries; i++)
        {

            if (entries[i].idUser[0] != 0)
            {

                double timeDiff = difftime(now, entries[i].timestamp);
                if (timeDiff >= 60)
                { // 5 minuti = 60 * 5, metto 1 minuto se non non mi passa più
                    //devo eliminare la entry
                    //settando il primo byte a 0 non viene più riconosciuta come una stringa
                    //e posso facilmente identificarla come cancellata
                    entries[i].idUser[0] = 0;
                    entries[i].key = 0;
                    entries[i].timestamp = 0;
                }

                printf("%s,%d,%ld\n", entries[i].idUser, entries[i].key, entries[i].timestamp);
            }
        }
        semOp(semid, 0 ,1);
    }
}


void openFIFO(){
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
}


int main(int argc, char *argv[])
{

    printf("Hi, I'm Server program!\n");

    setSignalHandler();

    key_t key = getKey();

    // creo segmento memoria condiviso
    printf("<Server> allocating a shared memory segment\n");

    shmid = alloc_shared_memory(key, sizeof(struct SharedItem *) * totalEntries); //alloco spazio per 10 SharedItem
    //me lo prendo
    entries = (struct SharedItem *)get_shared_memory(shmid, 0);


    semid = semget(key, 1,IPC_CREAT |  S_IRUSR | S_IWUSR);
    if(semid == -1)
        errExit("semget failed");

    // Initialize the semaphore set
    unsigned short semInitVal[] = {1};
    union semun arg;
    arg.array = semInitVal;

    if (semctl(semid, 0 /*ignored*/, SETALL, arg) == -1)
        errExit("semctl SETALL failed");



    //lancio processo figlio keymanager
    pid = fork();
    if (pid == -1)
        printf("subprocess keymanager not created\n");
    else if (pid == 0)
    {
        printf("<Server> creates child\n");
        child();
    }


    openFIFO();

    int bR = -1;

    struct Request request;
    do
    {
        printf("<Server> waiting for a Request...\n");
        // Read a request from the FIFO
        bR = read(serverFIFO, &request, sizeof(struct Request));

        // Check the number of bytes read from the FIFO
        if (bR == -1)
        {
            printf("<Server> it looks like the FIFO is broken\n");
        }
        else if (bR != sizeof(struct Request) || bR == 0)
            printf("<Server> it looks like I did not receive a valid request\n");
        else
        {
            sendResponse(&request);
        }

    } while (bR != -1);

    return 0;
}
