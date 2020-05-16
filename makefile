CC=gcc
TARGET=mytrace

# single file, so intermediate .o files not needed
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(TARGET).c -o $(TARGET)

clean: 
	rm -rf $(TARGET)
