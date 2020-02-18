all: user server

user: user.c
	gcc user.c -o user -lpthread -std=c99

server: router.c
	gcc router.c -o router -std=c99
