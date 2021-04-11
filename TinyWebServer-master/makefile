CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  ./timer/timer_heap.cpp ./http/http_conn.cpp ./log/log.cpp ./CGImysql/sql_connection_pool.cpp  ./util/util.cpp webserver.cpp config.cpp ./redis/redis_conn.cpp ./epoller/epoller.cpp
		 $(CXX) -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient -lhiredis -std=c++1z

clean:
	rm  -r *.o
