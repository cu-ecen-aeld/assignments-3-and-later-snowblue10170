CC = $(CROSS_COMPILE)gcc

OBJS := aesdsocket.o
TARGET = aesdsocket
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f $(TARGET) *.o

