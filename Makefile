#CC=arm-linux-gnueabihf-gcc
CC=g++
app:  ev_thread_server_process ev_client_process
#app: client_process server_process
#app:  ev_server_process ev_client_process
ev_thread_server_process:websocket_common.o ev_thread_server_main.o
	$(CC) -O3 -o $@ $^ -lpthread -lcrypto -lssl -levent
ev_client_process:websocket_common.o ev_client_main.o
	$(CC) -O3 -o $@ $^ -lpthread -lcrypto -lssl -levent 
ev_server_process:websocket_common.o ev_server_main.o
	$(CC) -O3 -o $@ $^ -lpthread -lcrypto -lssl -levent
client_process:websocket_common.o client_main.o
	$(CC) -O3 -o $@ $^ -lpthread -lcrypto -lssl -levent
server_process:websocket_common.o server_main.o
	$(CC) -O3 -o $@ $^ -lpthread -lcrypto -lssl -levent
ev_client_process_d:websocket_common_d.o ev_client_main_d.o
	$(CC) -O3 -o $@ $^ -lpthread -lcrypto -lssl -levent -g 
ev_server_process_d:websocket_common_d.o ev_server_main_d.o
	$(CC) -O3 -o $@ $^ -lpthread -lcrypto -lssl -levent -g
ev_thread_server_process_d:websocket_common_d.o ev_thread_server_main_d.o
	$(CC) -O3 -o $@ $^ -lpthread -lcrypto -lssl -levent -g
%.o:%.c
	$(CC) $< -c
%.o:%.cpp
	$(CC) $< -c
%_d.o:%.c
	$(CC) $< -c -g -o $@
%_d.o:%.cpp
	$(CC) $< -c -g -o $@
.PHONY:clean
clean:
	@rm -rf client_process* server_process* ev_client_process* ev_server_process* ev_thread_server_process* *.o 
debug:ev_thread_server_process_d ev_client_process_d
