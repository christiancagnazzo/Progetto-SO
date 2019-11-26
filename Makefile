# flags per la compilazione
CFLAGS = -std=c89 -Wpedantic
# target ovvero nome dell'eseguibile che si intende produrre
TARGET = master
TARGET1 = giocatore 
# object files necessari per produrre l'eseguibile
OBJ    = my_lib.o settings.o master.o
OBJ1   = my_lib.o settings.o giocatore.o

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $(TARGET)

$(TARGET1): $(OBJ1)
	$(CC) $(OBJ1) $(CFLAGS) -o $(TARGET1)

all: $(TARGET) $(TARGET1)

clean:
	rm -f *.o $(TARGET) $(TARGET1) *~

run: $(TARGET) $(TARGET1)
	./$(TARGET)
