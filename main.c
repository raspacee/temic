// includes
#include <stdarg.h>
#include <stdbool.h>

#include "data.h"
#include "editorInput.h"
#include "editorOutput.h"
#include "editorFileIO.h"
#include "editorInit.h"
#include "editorTerminal.h"
#include "editorOperations.h"

extern struct editorConfig E;

// init
int main(int argc, char *argv[])
{
	enableRawMode();
	initEditor();

	if (argc >= 2)
		editorOpen(argv[1]);
	else {
		editorInsertNewline(false);
		E.filemode = INSERT_MODE;
	}

	editorSetStatusMessage("HELP: CTRL-S = Save | CTRL-F = Find | CTRL-Q = Quit");

	while (1) {
		editorRefreshScreen();

		editorProcessKeypress();
	}
}
