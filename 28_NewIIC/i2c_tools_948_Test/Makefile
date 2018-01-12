OBJS = $(SRC)DS90UB948_Test.o $(SRC)TestPattern.o
VPATH = ./include
CC = arm-linux-gcc
CFLAGS = 
SRC = source/
INCLUDE = ./include

DS90UB948_Test:$(OBJS)
	cd source && $(MAKE)
	$(CC) -o DS90UB948_Test $(OBJS) -I $(INCLUDE) 

.PHONY:clean

clean:
	-rm DS90UB948_Test $(OBJS)

