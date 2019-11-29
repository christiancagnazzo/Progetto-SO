# flags per la compilazione
CFLAGS = -std=c89 -Wpedantic
# target ovvero nome dell'eseguibile che si intende produrre
TARGET = master
TARGET1 = giocatore 
TARGET2 = pedina 
# object files necessari per produrre l'eseguibile
OBJ    = my_lib.o master.o
OBJ1   = my_lib.o giocatore.o
OBJ2    = my_lib.o pedina.o


$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $(TARGET)

$(TARGET1): $(OBJ1)
	$(CC) $(OBJ1) $(CFLAGS) -o $(TARGET1)

$(TARGET2): $(OBJ2)
	$(CC) $(OBJ2) $(CFLAGS) -o $(TARGET2)


all: $(TARGET) $(TARGET1) $(TARGET2)

clean:
	rm -f *.o $(TARGET) $(TARGET1) $(TARGET2) *~

run: $(TARGET) $(TARGET1) $(TARGET2)
	./$(TARGET)
