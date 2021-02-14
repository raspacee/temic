// includes
#include <stdarg.h>

#include "data.h"
#include "editorInput.h"
#include "editorOutput.h"
#include "editorFileIO.h"
#include "editorInit.h"
#include "editorTerminal.h"

// init
int main(int argc, char *argv[])
{
	enableRawMode();
	initEditor();

	if (argc >= 2)
		editorOpen(argv[1]);

	editorSetStatusMessage("HELP: CTRL-S = Save | CTRL-F = Find | CTRL-Q = Quit");

	while (1) {
		editorRefreshScreen();

		editorProcessKeypress();
	}
}
