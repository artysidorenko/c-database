
# Compiler options
CC							=	gcc-10
CFLAGS					=	-c -Wall
LDFLAGS					=
EXECUTABLE_NAME	= database

# Directories
SRCDIR	=	src
BINDIR	=	bin
OBJDIR	=	$(BINDIR)/obj

# Files
SOURCE_FILES 			= $(wildcard $(SRCDIR)/*.c)
EXECUTABLE_FILES	= $(EXECUTABLE_NAME:%=$(BINDIR)/%)
OBJECT_FILES			= $(SOURCE_FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

build: $(EXECUTABLE_FILES)

clean:
	rm -r -f $(BIN)

test: build
	rspec spec/database.rb

.PHONY: build clean test

$(EXECUTABLE_FILES): $(OBJECT_FILES)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Build successful!"

$(OBJECT_FILES): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling $<
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
