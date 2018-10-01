#CC=arm-linux-gnueabihf-gcc	
CC=g++
app:  ev_httpd_server_process ev_client_process 
#app:  ev_thread_server_process ev_client_process 
#app: client_process server_process
#app:  ev_server_process ev_client_process
ev_httpd_server_process:ev_websocket.o ev_httpd.o evbase_threadpool.o Threadpool.o ev_httpd_test.o
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent
ev_thread_server_process:websocket_common.o ev_thread_server_main.o evbase_threadpool.o Threadpool.o 
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent
ev_thread_server_process_d:websocket_common_d.o ev_thread_server_main_d.o evbase_threadpool_d.o Threadpool_d.o 
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent -g
ev_client_process:websocket_common.o ev_client_main.o 
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent 
ev_server_process:websocket_common.o ev_server_main.o h
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent
client_process:websocket_common.o client_main.o 
	$(CC) -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent
server_process:websocket_common.o server_main.o 
	$(CC) -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent
ev_client_process_d:websocket_common_d.o ev_client_main_d.o 
	$(CC) -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent -g 
ev_server_process_d:websocket_common_d.o ev_server_main_d.o 
	$(CC) -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent -g
%.o:%.c config.h
	$(CC) $^ -c -Wall
%.o:%.cpp cnfig.h
	$(CC) $^ -c -o $@ -Wall
%_d.o:%.c cnfig.h
	$(CC) $^ -c -g -o $@ -Wall
%_d.o:%.cpp cnfig.h
	$(CC) $^ -c -g -o $@ -Wall
.PHONY:clean
clean:
	@rm -rf client_process* server_process* ev_client_process* ev_server_process* ev_httpd_server_process* ev_thread_server_process* *.o 
debug:ev_thread_server_process_d ev_client_process_d ev_thread_server_process_d	
