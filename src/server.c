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

#define MAXENTRIES 100
#define MUTEX 0

#define STAMPA 1
#define SALVA 2
#define INVIA 3

int projectId = 123;

struct SharedItem *entries;
int shmid;
int semid;

pid_t pid;

/**
 * Function that check if the service is ok
 * @param service
 * @return 0 ok, 1 wrong
 */
int checkRequestService(char *service) {
    if ((strncmp(service, "stampa", strlen("stampa")) == 0 || strncmp(service, "salva", strlen("salva")) == 0 ||
         strncmp(service, "invia", strlen("invia")) == 0)) {
        return 0;
    }
    return 1;
}

/**
 * Handler of the parent process
 */
void sigParentHandler() {

    // detach the shared memory segment
    printf("<Server> detaching the shared memory segment...\n");
    free_shared_memory(entries);

    // remove the shared memory segment
    printf("<Server> removing the shared memory segment[%d]...\n", shmid);
    remove_shared_memory(shmid);

    printf("<Server> remove file ftok\n");
    if (unlink(fileFtokPath) == -1) {
        errExit("Remove of file ftok failed");
    }

    kill(pid, SIGTERM);

    exit(0);


}


int genereteKeyService(int timestamp,char *service){

    int key = timestamp % 10000; //get the last 4 digits
    key = (key + rand() % 1000 + 1); //sum up a random number

    int serviceId = -1;

    if (strcmp(service, "stampa") == 0) {
        serviceId = STAMPA;
    } else if (strcmp(service, "salva") == 0) {
        serviceId = SALVA;
    } else if (strcmp(service, "invia") == 0) {
        serviceId = INVIA;
    }

    int i=1;
    while (i<=key){
        i*=10;
    }



    return serviceId * i + key;
}

void sendResponse(struct Request *request) {

    int cont = 0;
    char pathClientFifo[25];
    sprintf(pathClientFifo, "%s%d", basePathClientFifo, request->clientPid);

    printf("%s\n", request->idUser);

    if (checkRequestService(request->service) == 0) {
        // Step-2: The client opens the server's FIFO to send a Request
        printf("<Server> opening FIFO %s...\n", pathClientFifo);
        int clientFIFO = open(pathClientFifo, O_WRONLY);
        if (clientFIFO == -1)
            errExit("open failed");


        semOp(semid, MUTEX, -1);

        for (int i = 0; i < MAXENTRIES; i++) {
            if (entries[i].idUser[0] == 0) {
                //E' vuoto o è stato cancellato da processo keyserver
                //Setto il contatore a i così poi verrà scritto in quell'elemento
                cont = i;
                break;
            }
        }

        int timeStamp = time(0);

        strcpy(entries[cont].idUser, request->idUser);
        entries[cont].timestamp = timeStamp;

        int key = genereteKeyService(timeStamp, request->service);


        struct Response response;

        entries[cont].key = key;
        response.key = key;

        semOp(semid, MUTEX, 1);

        // Step-3: The Server sends a Response through the client's FIFO
        printf("<Server> sending ... \n");
        if (write(clientFIFO, &response,
                  sizeof(struct Response)) != sizeof(struct Response))
            errExit("write failed");
    }
}


void setSignalHandler() {
    // set of signals not initialized
    sigset_t mySet;
    // initialize mySet to contain all signals
    sigfillset(&mySet);
    // remove SIGTERM from mySet
    sigdelset(&mySet, SIGTERM);
    // blocking all signals but SIGTERM
    sigprocmask(SIG_SETMASK, &mySet, NULL);

    // set the function sigParentHandler as handler for the signal SIGTERM
    if (signal(SIGTERM, sigParentHandler) == SIG_ERR)
        errExit("change signal handler failed");
}


/**
 * Function that return a key with ftok
 * @return key_t key
 */
key_t getKey() {
    open(fileFtokPath, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    return ftok(fileFtokPath, projectId);
}

/**
 * Signal handler of the child
 */
void sigHandlerChild() {
    printf("<Subprocess child> Killed\n");
    exit(0);
}


/**
 * Function that contains the code of the child
 */
void child() {

    if (signal(SIGTERM, sigHandlerChild) == SIG_ERR)
        errExit("change signal handler child failed\n");


    while (1) {
        sleep(30);
        printf("<Subprocess child> checking shared memory\n");
        fflush(stdout); //todo studiare come funziona il fflush di preciso


        semOp(semid, MUTEX, -1);
        time_t now = time(0);
        for (int i = 0; i < MAXENTRIES; i++) {

            if (entries[i].idUser[0] != 0) {

                double timeDiff = difftime(now, entries[i].timestamp);

                if (timeDiff >= 60) {  //todo put 60 * 5 in production
                    entries[i].idUser[0] = 0;
                    entries[i].key = 0;
                    entries[i].timestamp = 0;
                }

                printf("%s,%d,%ld\n", entries[i].idUser, entries[i].key, entries[i].timestamp);
            }
        }
        semOp(semid, MUTEX, 1);
    }
}


void openFIFO() {
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


int createSemaphoreSet(key_t key) {
    semid = semget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1)
        errExit("semget failed");


    union semun arg;
    //set semaphore value to 1
    arg.val = 1;

    //initialize the 0-th semaphore to 1
    if (semctl(semid, 0, SETVAL, arg) == -1)
        errExit("semctl SETVAL failed");

    return semid;
}


int main(int argc, char *argv[]) {

    srand(time(0));

    printf("Hi, I'm Server program!\n");

    setSignalHandler();

    key_t key = getKey();

    printf("<Server> allocating a shared memory segment\n");

    shmid = alloc_shared_memory(key, sizeof(struct SharedItem *) * MAXENTRIES);

    entries = (struct SharedItem *) get_shared_memory(shmid, 0);

    semid = createSemaphoreSet(key);


    //lancio processo figlio keymanager
    pid = fork();
    if (pid == -1)
        printf("subprocess keymanager not created\n");
    else if (pid == 0) {
        printf("<Server> creates child\n");
        child();
    }


    openFIFO();

    int bR = -1;

    struct Request request;
    do {
        printf("<Server> waiting for a Request...\n");
        // Read a request from the FIFO
        bR = read(serverFIFO, &request, sizeof(struct Request));

        // Check the number of bytes read from the FIFO
        if (bR == -1) {
            printf("<Server> it looks like the FIFO is broken\n");
        } else if (bR != sizeof(struct Request) || bR == 0)
            printf("<Server> it looks like I did not receive a valid request\n");
        else {
            sendResponse(&request);
        }

    } while (bR != -1);

    return 0;
}
