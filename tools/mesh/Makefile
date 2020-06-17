OBJS = ../../source/tools/mesh/*.c ../../source/game_renderer.c
CC = gcc
INCLUDE_PATHS =  -I ../../source/libs/png/include -I ../../source/libs/io -I ../../source/libs/glew/include -I ../../source/libs/os -I ../../source/libs/json
LIBRARY_PATHS =  -L ../../source/libs/png/lib -L ../../source/libs/io -L ../../source/libs/glew/lib -L ../../source/libs/os -L ../../source/libs/json
COMPILER_FLAGS = -std=c89 -pedantic
LINKER_FLAGS = -lmingw32 -lio -los -lgdi32 -limm32 -lopengl32 -lglew32 -lpng -ljson
OBJ_NAME = mesh

all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -g -o $(OBJ_NAME)
