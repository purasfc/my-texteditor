#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

struct termios orig_termios;

void die(const char *s) {
	perror(s); /* from stdio.h */
	exit(1); /* from stdlib.h */
}

void disableRawMode(void) {
		if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
			die("tcsetattr");
}

void enableRawMode(void) {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_cc[VMIN] = 0; /* read can return value with 0 byte */
	raw.c_cc[VTIME] = 1; /* timeout for returning from read. if time pass this value,
	then read() returns value*/

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int main(void) {
	enableRawMode();

	while(1) {
		char c = '\0';
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read"); /* read returns -1 on failure and return EAGAIN when there is no input or something. therefore, this condition express a real error situation */
		if (iscntrl(c)) {
			printf("%d\r\n", c);
		}
		else {
			printf("%d ('%c')\r\n", c, c);
		}
		if (c == 'q') break;
	}
	return 0;
}
