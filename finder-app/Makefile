CC ?= gcc
ifeq ($(CROSS_COMPILE),)
    TARGET := writer
else
    TARGET := writer_arm
    CC := $(CROSS_COMPILE)gcc
endif

OBJS := writer.o
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) writer_arm
