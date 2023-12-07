CC = gcc
CFLAGS = -g -w
LIBS = `pkg-config --cflags --libs gtk+-3.0` -rdynamic
TARGET = system_monitor

all: $(TARGET)

$(TARGET): system_monitor.c
	$(CC) $(CFLAGS) -o $(TARGET) system_monitor.c $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGET)
