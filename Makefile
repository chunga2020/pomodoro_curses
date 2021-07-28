CFLAGS=-g -O2 -Wall -Wextra -Isrc -DNDEBUG -lncurses

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
TESTABLE_SRC=$(filter-out src/main.c, $(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

PROG=pomodoro_curses
TARGET=./bin/$(PROG)

MANDIR=$(HOME)/man/man1
DOCDIR=doc
MANPAGE=$(PROG).1
DESTDIR=$(HOME)/.local/bin

.PHONY: all tests clean check install uninstall

# The target build
all: tests $(TARGET)

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra -lncurses
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
	@echo -n Creating $(DESTDIR)...
	@install -d $(DESTDIR)
	@echo " Done."
	@echo -n Installing $(PROG) to $(DESTDIR)...
	@install -m 755 $(TARGET) $(DESTDIR)
	@echo " Done."
	@echo -n Creating local \`man\` directory...
	@install -d $(MANDIR)
	@echo " Done."
	@echo -n Moving manpage to $(MANDIR)...
	@cp $(DOCDIR)/$(MANPAGE) $(MANDIR)
	@echo " Done."
	@echo -n Compressing manpage...
	@gzip -f $(MANDIR)/$(MANPAGE)
	@echo " Done."
	@echo -n Updating \`man\` database...
	@mandb -q
	@echo " Done."
	@echo
	@echo Installation complete.

uninstall:
	@echo -n Removing $(DESTDIR)/$(PROG)...
	@rm -f $(DESTDIR)/$(PROG)
	@echo " Done."
	@echo -n Removing \`man\` page...
	@rm -f $(MANDIR)/$(MANPAGE).gz
	@echo " Done."
	@echo -n "Updating \`man\` database..."
	@mandb -q
	@echo " Done."
	@echo
	@echo "Uninstall complete."
