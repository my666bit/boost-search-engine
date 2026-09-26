PARSER=parser
DUG=debug
HTTP_SERVER=http_server
cc=g++

.PHONY:all clean
all:$(PARSER) $(DUG) $(HTTP_SERVER)

$(PARSER):parser.cc
	$(cc) -o $@ $^ -I./cppjieba/include -lboost_system -lboost_filesystem -std=c++11
$(DUG):debug.cc
	$(cc) -o $@ $^ -I./cppjieba/include -ljsoncpp -std=c++11
$(HTTP_SERVER):http_server.cc
	$(cc) -o $@ $^ -I./cppjieba/include -ljsoncpp -std=c++11 -lpthread
clean:
	rm -f $(PARSER) $(DUG) $(HTTP_SERVER)
