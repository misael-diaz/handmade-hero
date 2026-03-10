#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

int main()
{
	fprintf(stdout, "%s", "Linux - HandMade Hero\n");
	Display *display = XOpenDisplay(NULL);
	if (!display) {
		fprintf(stdout, "%s", "XErrorOpenDisplay\n");
		exit(EXIT_FAILURE);
	}

	XCloseDisplay(display);
	return 0;
}
