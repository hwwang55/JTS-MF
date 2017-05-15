#Makefile begin
CC = g++
CFLAGS = -g -c -O3 -fopenmp
INCLUDE = 
LIBS = -lmysqlclient -I/usr/include/mysql/ -L/usr/lib/mysql
OBJ = ./obj
OBJECTS = $(OBJ)/database.o $(OBJ)/dbfactory.o $(OBJ)/converge.o $(OBJ)/item_update.o $(OBJ)/latent_feature_initialize.o $(OBJ)/load_data.o $(OBJ)/main.o $(OBJ)/user_update.o $(OBJ)/top_n_test.o

TARGETS = ./weiboMF

all:${TARGETS}

./weiboMF:$(OBJECTS)
	$(CC) -o $@ -fopenmp $(OBJECTS) $(INCLUDE) $(LIBS)

$(OBJECTS):$(OBJ)/%.o:%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	-rm $(OBJ)/*.o
