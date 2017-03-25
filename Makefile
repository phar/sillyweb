


LIBS=-lssl -lpthread -lcrypto
IDIR =/usr/local/opt/openssl/include
LDIR=/usr/local/opt/openssl/lib


server : buffer.o  mime.o client.o config.o  headers.o log.o main.o http.o percent.o server.o service.o  transfer.o transport.o util.o vhost.o
	gcc -o server buffer.o mime.o client.o config.o  http.o headers.o log.o main.o percent.o server.o service.o transfer.o  transport.o util.o vhost.o $(LIBS)


http.o : http.c
	gcc -c http.c -I$(IDIR) -L$(LDIR)

mime.o : mime.c
	gcc -c mime.c -I$(IDIR) -L$(LDIR)

buffer.o : buffer.c
	gcc -c buffer.c -I$(IDIR) -L$(LDIR)

client.o : client.o
	gcc -c client.c -I$(IDIR) -L$(LDIR)

config.o : config.o
	gcc -c config.c -I$(IDIR) -L$(LDIR)

headers.o : headers.c
	gcc -c headers.c -I$(IDIR) -L$(LDIR)

log.o : log.c
	gcc -c log.c -I$(IDIR) -L$(LDIR)

percent.o : server.c
	gcc -c percent.c -I$(IDIR) -L$(LDIR)

server.o : server.c
	gcc -c server.c -I$(IDIR) -L$(LDIR)

service.o : service.c
	gcc -c service.c -I$(IDIR) -L$(LDIR)

transfer.o : transfer.c
	gcc -c transfer.c -I$(IDIR) -L$(LDIR)

transport.o : transport.c
	gcc -c transport.c -I$(IDIR) -L$(LDIR)

util.o : util.c
	gcc -c util.c -I$(IDIR) -L$(LDIR)

vhost.o : vhost.c
	gcc -c vhost.c -I$(IDIR) -L$(LDIR)

main.o : main.c
	gcc -c main.c -I$(IDIR) -L$(LDIR)
clean :
	rm *.o
	rm server


