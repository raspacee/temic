
CC=cc

#	-g	adds debugging information to the executable
# -Wall and -Wextra turns on more warnings for the program
# -pedantic warns if the code violates ISO C
CFLAGS=-Wall -Wextra -pedantic -g

# the build target executable
TARGET=temic

$(TARGET): main.c editorBuffer.c editorFileIO.c editorFind.c editorInit.c editorInput.c editorOperations.c editorOutput.c editorSyntaxHL.c editorTerminal.c
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	$(RM) $(TARGET)