#ifndef EDITOR_BUFFER_INCLUDE
#define EDITOR_BUFFER_INCLUDE

void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

#endif