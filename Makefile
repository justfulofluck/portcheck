CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -O2
LDFLAGS = -lncurses -lmenu -lpanel -lpthread

TARGET = portcheck
SOURCES = main.c scanner.c history.c tui.c
HEADERS = scanner.h history.h tui.h
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) *.json

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)/usr/bin/$(TARGET)
	install -D -m 644 $(TARGET).1 $(DESTDIR)/usr/share/man/man1/$(TARGET).1

uninstall:
	rm -f $(DESTDIR)/usr/bin/$(TARGET)
	rm -f $(DESTDIR)/usr/share/man/man1/$(TARGET).1

deb: $(TARGET)
	dpkg-buildpackage -b -us -uc

help:
	@echo "PortCheck Makefile"
	@echo "=================="
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the application (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  install   - Install the application"
	@echo "  uninstall - Uninstall the application"
	@echo "  deb       - Build Debian package"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Usage:"
	@echo "  make           - Build"
	@echo "  make clean     - Clean build"
	@echo "  sudo make install - Install system-wide"
