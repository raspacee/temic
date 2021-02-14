#ifndef EDITOR_INPUT_INCLUDE
#define EDITOR_INPUT_INCLUDE

void editorProcessKeypress(void);
void editorMoveCursor(int key);
char *editorPrompt(char *prompt, void (*callback)(char *, int));

#endif