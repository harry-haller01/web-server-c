server: server.c
	gcc -o server server.c -lpthread
	./server $(PORT) $(DIR)

PORT ?= 8800
DIR ?= /