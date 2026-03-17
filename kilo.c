/*** includes ***/
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
struct editorConfig {
	int screenrows;
	int screencols;
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

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;
	/* ioctl(), TIOCGWINSZ, struct winsize come from sys/ioctl.h */
	/* int ioctl(int fd, unsigned long request, *buffer) */
	/* TIOCGWINSZ: request to get window size */
	/* struct winsize{
	 * unsigned short ws_row;
	 * ws_col:
	 * ws_xpixel:
	 * ws_ypixel
	 * }*/
	if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
		editorReadKey();
		return -1;
	}
	else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}
/*** output ***/

void editorDrawRows(void) {
	int y;
	for (y=0; y<E.screenrows; y++) {
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
/*** init ***/

void initEditor(void) {
	if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main(void) {
	enableRawMode();
	initEditor();

	while(1) {
		editorRefreshScrean();
		editorProcessKeypress();
	}
	return 0;
}
