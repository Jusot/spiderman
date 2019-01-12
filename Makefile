DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.cpp)
OBJ = $(patsubst %.cpp, ${DIR_OBJ}/%.o, $(notdir ${SRC}))

TARGET = puef

BIN_TARGET = $(DIR_BIN)/$(TARGET)

CC = g++
CFLAGS = -g -Wall -std=c++11 -I$(DIR_INC)

${BIN_TARGET}: ${OBJ} | ${DIR_BIN}
	${CC} ${OBJ} -pthread -o $@

${DIR_OBJ}/%.o: ${DIR_SRC}/%.cpp | ${DIR_OBJ}
	${CC} ${CFLAGS} -c $< -o $@

${DIR_BIN}:
	mkdir $@

${DIR_OBJ}:
	mkdir $@

.PHONY: clean
clean:
	rm -rf ${DIR_OBJ}/*.o