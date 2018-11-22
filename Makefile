###########################################
#Makefile for simple programs
###########################################
INC:=./include
SRC:=./src
BIN:=./bin
OBJ:=./obj

CC:=gcc
CC_FLAG:=-Wall -g -DRTSP_DEBUG -DSAVE_FILE_DEBUG
LD_FLAG:=-lpthread


SOURCES:=$(wildcard $(SRC)/*.c)
OBJS:=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))
TARGET:=rtspClient

$(TARGET):$(OBJS)
	$(CC) $(CC_FLAG) -I$(INC) -o $(BIN)/$@ $(OBJS) $(LD_FLAG)
	
all:$(OBJS)
$(OBJS):$(OBJ)/%.o:$(SRC)/%.c
	$(CC) $(CC_FLAG) -I$(INC) -c $< -o $@

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ)/*.o $(BIN)/*
