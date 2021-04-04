
# COMPILER OPTIONS
CC							=	gcc-10
# throw error in case of any compile warnings:
CFLAGS					=	-c -Wall -Werror
debug : CFLAGS 	= -g -c -Wall -Werror
LDFLAGS					=
EXECUTABLE_NAME	= database

# DIRECTORIES
SRCDIR	=	src
BINDIR	=	bin
OBJDIR	=	$(BINDIR)/obj

# FILES
SOURCE_FILES 			= $(wildcard $(SRCDIR)/*.c)
EXECUTABLE_FILES	= $(EXECUTABLE_NAME:%=$(BINDIR)/%)
OBJECT_FILES			= $(SOURCE_FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# TEST OPTIONS
# Filter tests for specific example(s) using ONLY=my_test optional argument
EXAMPLE = $(ONLY)

build: $(EXECUTABLE_FILES)

clean:
	rm -r -f $(BINDIR)

test: build
	rspec spec/database.rb -e "$(EXAMPLE)"
	rspec spec/b_tree.rb -e "$(EXAMPLE)"

run: build
	bin/database spec/test.db

debug: build

.PHONY: build clean test

$(EXECUTABLE_FILES): $(OBJECT_FILES)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Build successful!"

$(OBJECT_FILES): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
