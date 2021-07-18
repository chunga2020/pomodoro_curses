CFLAGS=-g -O2 -Wall -Wextra -Isrc -DNDEBUG

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=./bin/pomodoro_curses

.PHONY: all tests clean check

# The target build
all: $(TARGET) tests

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra
dev: all

$(TARGET): build $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

build:
	@mkdir -p bin

# The Unit Tests
$(TESTS): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(patsubst %,%.c,$(@)) $(OBJECTS)

tests: $(TESTS)
	sh ./tests/runtests.sh

# The cleaner
clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The checker
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)\
		|stpn?cpy|a?sn?printf|byte_)' $(SOURCES) || true
