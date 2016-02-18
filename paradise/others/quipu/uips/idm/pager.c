#include <stdio.h>
#include <varargs.h>
#include <sys/types.h>
#include <termios.h>
#include "quipu/util.h"
#include "demanifest.h"

extern int lines, cols;

static int numOK;
static int pagerNumber = -1;
static int wrapLines = FALSE;

int lineno;
int discardInput;

pagerOn(number)
int number;
{
	lineno = 0;
	discardInput = FALSE;
	pagerNumber = -1;
	if (number == NUMBER_ALLOWED)
		numOK = TRUE;
	else if (number == NUMBER_NOT_ALLOWED)
		numOK = FALSE;
}

linewrapOn() {
	wrapLines = TRUE;
}

linewrapOff() {
	wrapLines = FALSE;
}

isWrapOn() {
	return wrapLines;
}

pageprint(va_alist)
va_dcl /* no ; */
{
	va_list ap;
	char buf[BUFSIZ];
	int i, c;
	static int charsInLine = 0;

	va_start (ap);
	if (discardInput == TRUE)
		return;
	_asprintf (buf, NULLCP, ap);
	for (i = 0; buf[i] != '\0'; i++) {
		if (buf[i] == '\n') {
			charsInLine = 0;
			(void) putchar(buf[i]);
			lineno++;
			if (lineno >= lines - 1) {
				putPagePrompt();
				c = getPagerInput();
				switch(c) {
				case '\n':
					lineno--;
					break;
				case ' ':
					lineno = 1;
					break;
				default:
					break;
				}
				if ((pagerNumber == -1) && ((c == VEOF) || (c == VKILL)))
					(void) kill(getpid(), 2); /* let the signal handling sort it out */
				if (((pagerNumber == -1)&& (c == 'q')) || (pagerNumber != -1)) {
					discardInput = TRUE;
					break;
				}
			}
		} else
			/* optionally discard chars that would cause line wrapping */
			if (isWrapOn() ||
					(charsInLine < (cols - 1))) {
				(void) putchar(buf[i]);
				charsInLine++;
				if (isWrapOn()) {
					if (charsInLine == cols) {
						charsInLine = 0;
						lineno++;
					}
				}
			}
	}
	va_end(ap);
}

putPagePrompt() {
	writeInverse("SPACE for next screen; q to quit pager");
	if (numOK == TRUE)
		writeInverse("; or the number of the entry");
	writeInverse(": ");
}

int
getPagerInput() {
	int c, i;
	char numstr[LINESIZE];

	pagerNumber = -1;
	setRawMode();
	c = 'Z';
	while (! ((c == 'q') || (c == ' ') || (c == '\n')
			  || (c == VKILL) || (c == VEOF))) {
		if ((numOK == TRUE) && isdigit(c)) {
			for (i = 0; isdigit(c); i++) {
				(void) putchar(c);
				numstr[i] = c;
				c = getchar();
			}
			if (c == '\n') {
				numstr[i+1] = '\0';
				c = pagerNumber = atoi(numstr);
				break;
			} else {
				clearLine();
				putPagePrompt();
				c = getchar();
				continue;
			}
		} else
			c = getchar();
	}
	unsetRawMode();
	clearLine();
	return c;
}

int
getpnum() {
	return pagerNumber;
}

