#!/bin/bash
gcc ./request.h ./my_queue.h ./netfileserver.h ./netfileserver.c ./my_queue.c -o test_server -lpthread -std=gnu99 -g

