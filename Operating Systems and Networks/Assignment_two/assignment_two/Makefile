# Makefile for secure_server and secure_client

# -------------------------------------------------------------------
# Configurable variables
# -------------------------------------------------------------------
BOOST_INC    := E:/boost_1_87_0
CXX          := g++
CXXSTD       := -std=c++17
WARNINGS     := -Wall
OPTIM        := -O2

# -------------------------------------------------------------------
# Detect OS
# -------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
    # Windows settings
    EXE_EXT     := .exe
    SOCKET_LIB  := -lws2_32
    RM          := del /Q
    RM_OBJS     := del /Q *.o
else
    # Unix-like (Linux, macOS)
    EXE_EXT     := 
    SOCKET_LIB  :=
    RM          := rm -f
    RM_OBJS     := rm -f *.o
endif

# -------------------------------------------------------------------
# Targets
# -------------------------------------------------------------------
SERV_SRC     := secure_server.cpp
CLI_SRC      := secure_client.cpp

SERV_OBJ     := secure_server.o
CLI_OBJ      := secure_client.o

SERV_BIN     := secure_server$(EXE_EXT)
CLI_BIN      := secure_client$(EXE_EXT)

.PHONY: all clean clean_objs

all: $(SERV_BIN) $(CLI_BIN)

# -------------------------------------------------------------------
# Server
# -------------------------------------------------------------------
$(SERV_BIN): $(SERV_OBJ)
	$(CXX) $(CXXSTD) $(SERV_OBJ) $(SOCKET_LIB) -o $@

$(SERV_OBJ): $(SERV_SRC)
	$(CXX) $(CXXSTD) $(WARNINGS) $(OPTIM) -I $(BOOST_INC) -c $< -o $@

# -------------------------------------------------------------------
# Client
# -------------------------------------------------------------------
$(CLI_BIN): $(CLI_OBJ)
	$(CXX) $(CXXSTD) $(CLI_OBJ) $(SOCKET_LIB) -o $@

$(CLI_OBJ): $(CLI_SRC)
	$(CXX) $(CXXSTD) $(WARNINGS) $(OPTIM) -I $(BOOST_INC) -c $< -o $@

# -------------------------------------------------------------------
# Cleanup
# -------------------------------------------------------------------
clean: clean_objs
	$(RM) $(SERV_BIN) $(CLI_BIN)

clean_objs:
	$(RM_OBJS)
