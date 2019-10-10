CC=gcc
CFLAGS= -v -c -std=c99
WEBSERVER_OBJECT=./objects/main.o
WEBSERVER_SOURCE=./src/main.c
OBJECTDIR=objects

webserver: ${WEBSERVER_OBJECT}
	${CC} ${WEBSERVER_OBJECT} -o webserver

${WEBSERVER_OBJECT}: ${WEBSERVER_SOURCE}
	${CC} ${CFLAGS} ${WEBSERVER_SOURCE} -o ${WEBSERVER_OBJECT}

clean:
	rm -rf ./objects/*.o webserver
