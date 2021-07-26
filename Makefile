CFLAGS=-g -O2 -Wall -Wextra -Isrc -DNDEBUG -lncurses

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
TESTABLE_SRC=$(filter-out src/main.c, $(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

PROG=pomodoro_curses
TARGET=./bin/$(PROG)

DESTDIR=$(HOME)/.local/bin

.PHONY: all tests clean check install uninstall

# The target build
all: tests $(TARGET)

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra
dev: all

$(TARGET): build $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

build:
	@mkdir -p bin

# The Unit Tests
$(TESTS): $(TESTABLE_SRC)
	$(CC) $(CFLAGS) -o $@ $(patsubst %,%.c,$(@)) $(TESTABLE_SRC)

tests: $(TESTS)
	sh ./tests/runtests.sh

# The cleaner
clean:
	rm -rf bin $(OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The checker
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)\
		|stpn?cpy|a?sn?printf|byte_)' $(SOURCES) || true


install:
	install -d $(DESTDIR)
	install -m 755 $(TARGET) $(DESTDIR)

uninstall:
	rm -f $(DESTDIR)/$(PROG)
