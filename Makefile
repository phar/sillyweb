

CFLAGS=-ggdb
LIBS=-lssl -lpthread -lcrypto
IDIR =/usr/local/opt/openssl/include
LDIR=/usr/local/opt/openssl/lib


server : buffer.o  client.o config.o  headers.o log.o main.o percent.o server.o service.o  transfer.o transport.o util.o vhost.o mime.o
	gcc $(CFLAGS)  -o server mime.o buffer.o  client.o config.o  headers.o log.o main.o percent.o server.o service.o transfer.o  transport.o util.o vhost.o $(LIBS)

buffer.o : buffer.c
	gcc $(CFLAGS)  -c buffer.c -I$(IDIR) -L$(LDIR)

mime.o : mime.c
	gcc $(CFLAGS)  -c mime.c -I$(IDIR) -L$(LDIR)

client.o : client.o
	gcc $(CFLAGS)  -c client.c -I$(IDIR) -L$(LDIR)

config.o : config.o
	gcc $(CFLAGS)  -c config.c -I$(IDIR) -L$(LDIR)

headers.o : headers.c
	gcc $(CFLAGS)  -c headers.c -I$(IDIR) -L$(LDIR)

log.o : log.c
	gcc $(CFLAGS)  -c log.c -I$(IDIR) -L$(LDIR)

percent.o : server.c
	gcc $(CFLAGS)  -c percent.c -I$(IDIR) -L$(LDIR)

server.o : server.c
	gcc $(CFLAGS)  -c server.c -I$(IDIR) -L$(LDIR)

service.o : service.c
	gcc $(CFLAGS)  -c service.c -I$(IDIR) -L$(LDIR)

transfer.o : transfer.c
	gcc $(CFLAGS)  -c transfer.c -I$(IDIR) -L$(LDIR)

transport.o : transport.c
	gcc $(CFLAGS)  -c transport.c -I$(IDIR) -L$(LDIR)

util.o : util.c
	gcc $(CFLAGS)  -c util.c -I$(IDIR) -L$(LDIR)

vhost.o : vhost.c
	gcc $(CFLAGS)  -c vhost.c -I$(IDIR) -L$(LDIR)

main.o : main.c
	gcc $(CFLAGS)  -c main.c -I$(IDIR) -L$(LDIR)
clean :
	rm *.o
	rm server


