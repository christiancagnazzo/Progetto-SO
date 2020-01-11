# flags per la compilazione
CFLAGS = -std=c89 -Wpedantic
# eseguibile
TARGET = master
TARGET1 = giocatore 
TARGET2 = pedina 
# object files necessari per produrre l'eseguibile
OBJ    = my_lib.o master.c
OBJ1   = my_lib.o giocatore.c
OBJ2    = my_lib.o pedina.c

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -g -O0 -o  $(TARGET)

$(TARGET1): $(OBJ1)
	$(CC) $(OBJ1) $(CFLAGS) -g -O0 -o  $(TARGET1)

$(TARGET2): $(OBJ2)
	$(CC) $(OBJ2) $(CFLAGS) -g -O0 -o  $(TARGET2)


all: $(TARGET) $(TARGET1) $(TARGET2)

clean:
	rm -f *.o $(TARGET) $(TARGET1) $(TARGET2) *~

run: $(TARGET) $(TARGET1) $(TARGET2)
	./$(TARGET)
