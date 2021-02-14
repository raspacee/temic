kilo: main.c
	$(CC) main.c editorBuffer.c editorFileIO.c editorFind.c editorInit.c editorInput.c editorOperations.c editorOutput.c editorSyntaxHL.c editorTerminal.c -o kilo -Wall -Wextra -pedantic
