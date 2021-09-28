#ifndef EDITOR_SYNTAXHL_INCLUDE
#define EDITOR_SYNTAXHL_INCLUDE

void editorUpdateSyntax(struct erow *row);
int editorSyntaxToColor(int hl);
int is_separator(int c);
void editorSelectSyntaxHighlight(void);

#endif
