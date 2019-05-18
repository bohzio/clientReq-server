
CFLAGS=-Wall -std=gnu99 -DDEBUG -g
INCLUDES=-I./inc

# list source code files for server executable
SERVER_SRCS=src/errExit.c src/server.c


# list source code files for client executable
CLIENT_SRCS=src/errExit.c src/clientReq.c

##------------------------------------------------------------------------------
## DO NOT TOUCH BELOW THIS LINE!
##------------------------------------------------------------------------------
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)

all: server clientReq

server: $(SERVER_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

clientReq: $(CLIENT_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

.c.o:
	@echo "Compiling: "$<
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean

clean:
	@rm -f src/*.o server clientReq
	@echo "Removed object files and executables..."
