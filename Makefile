OBJS = source/*.c
CC = gcc
INCLUDE_PATHS =  -I source/libs/png/include -I source/libs/io -I source/libs/glew/include -I source/libs/os
LIBRARY_PATHS =  -L source/libs/png/lib -L source/libs/io -L source/libs/glew/lib -L source/libs/os
COMPILER_FLAGS = -std=c89 -pedantic
LINKER_FLAGS = -lmingw32 -lio -los -lgdi32 -limm32 -lopengl32 -lglew32 -lpng
OBJ_NAME = main

all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -g -o $(OBJ_NAME)
