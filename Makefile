client: client.c
	gcc -o client client.c
	./client 4000 /home/dariel/Escritorio/web-serv-ult

server: server.c
	gcc -o server server.c
	./server 4000 /home/dariel/Escritorio/web-serv-ult