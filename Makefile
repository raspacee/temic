temic: main.c editorBuffer.c editorFileIO.c editorFind.c editorInit.c editorInput.c editorOperations.c editorOutput.c editorSyntaxHL.c editorTerminal.c
	$(CC) main.c editorBuffer.c editorFileIO.c editorFind.c editorInit.c editorInput.c editorOperations.c editorOutput.c editorSyntaxHL.c editorTerminal.c -o temic -Wall -Wextra -pedantic
