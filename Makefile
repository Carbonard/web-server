CXX      = c++
CC       = cc
# CXXFLAGS = -Werror -Wall -Wextra
CFLAGS   = -Werror -Wall -Wextra


SERVER_NAMES = debug error main Server Server_clients WebServer
SRC_SERVER   = $(addprefix src/server/, $(addsuffix .cpp, $(SERVER_NAMES)))
OBJ_SERVER   = $(addprefix obj/server/, $(addsuffix .o, $(SERVER_NAMES)))

all: server cli

server: $(OBJ_SERVER)
	$(CXX) $(CXXFLAGS) $^ -o $@

obj/server/%.o: src/server/%.cpp | obj/server/
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/client/%.o: src/client/%.cpp | obj/client/
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/server/:
	@mkdir -p $@

obj/client/:
	@mkdir -p $@

cli: src/client/client.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -rf obj

fclean: clean
	@rm -f server cli

re: fclean all

.PHONY: all clean re
