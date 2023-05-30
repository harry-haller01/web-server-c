server: server.c
	gcc -o server server.c
	./server $(PORT) $(DIR)

PORT ?= 8800
DIR ?= /home/dariel/Escritorio/web-serv-ult