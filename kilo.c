/*** includes ***/
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
struct editorConfig {
	struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/

void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s); /* from stdio.h */
	exit(1); /* from stdlib.h */
}

void disableRawMode(void) {
		if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
			die("tcsetattr");
}

void enableRawMode(void) {
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = E.orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_cc[VMIN] = 0; /* read can return value with 0 byte */
	raw.c_cc[VTIME] = 1; /* timeout for returning from read. if time pass this value,
	then read() returns value*/

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey(void) {
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

/*** output ***/

void editorDrawRows(void) {
	int y;
	for (y=0; y<24; y++) {
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}
 
void editorRefreshScrean(void) {
	write(STDOUT_FILENO, "\x1b[2J", 4); /* 4 -> writing four bytes. \x1b -> escape character, or 27 in decimal. = writing escape sequence to the terminal. escape sequence always start from an escape character (27) followed by [ character. J -> Erase In Dispay. Escape sequence cmmands take arguments, which come before the command. In this case, the argument is 2, -> clear the <screen>. <esc>[1J would clear the screen up to where the cursor is, and <esc>[0J */
	write(STDOUT_FILENO, "\x1b[H", 3); /* reposition to the top of the terminal */
	
	editorDrawRows();
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress(void) {
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[2J", 3);
			exit(0);
			break;
	}
}

int main(void) {
	enableRawMode();

	while(1) {
		editorRefreshScrean();
		editorProcessKeypress();
	}
	return 0;
}
