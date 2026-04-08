TODO:
- need like headers and bullets for fast readers with the text (the meat) following for those interested
in the details


A pragmatic approach for putting graphics on screen with Xlib in GNU/Linux without prior experience is to
read code written by game developers. In my case I read Quake II's source code to see what Xlib API calls
legendary developer John Carmack and his team used to put graphics on the screen.

NOTES: THESE REASONS SHOULD BE BROKEN DOWN AS BULLETS FOR SPEED READING
I can identify a couple of reasons for building the platform layer of the Handmade Hero game
with Xlib in GNU/Linux. Even though it's possible to work directly with the framebuffer
device in Linux, which is as low as one could go in userspace land, that poses a security risk because
the application has access to the entire screen,  can be done in console mode but switching makes for poor gaming experience, and last but not least
it would not be similar to the development of Handmade Hero as a Win32 application.

 The main reason is that it's not practical to work directly with the



FIX THIS SPECIAL PRIV EXCERPT NOT QUITE WORDED PROPERLY REGULAR USERS IN THE VIDEO GROUP

WHAT YOU REALLY DON'T WANT IS TO HAVE ACCESS TO DEVELOP AND APP THAT HAS ACCESS TO THE ENTIRE SCREEN
FOR SECURITY REASONS
Applications that can interact directly with the framebuffer device can only be run by users with special
privileges, and that alone poses a serious security risk that I am not interested to tackle at the moment. 
The other reason for not working directly with the framebuffer is that the application would have to run
on console mode. (It must be executed from an actual console tty commonly accessed via Ctrl + Alt + Fn). I am disciplined enough to cope with the context switching but that
would be a game that nobody would ever want to play because that's not how people use computers. They
want to be able to switch among several applications seaminglessly.

Another of the main reasons for using Xlib is that my desktop environment directly depends on it and this
means that an XServer must be running in my machine for the desktop environment to even launch.
This also means that I must develop the game with at least Xlib if I am to follow the Handmade Hero way
of game development. Also, I don't have a spare machine to tinker around with it, so I tend to favor
stability over running the latest version of
the software I use. There you have it, those are some of the practical reason I am not using Wayland to put
graphics on my screen. The last reason is academic, I cannot simply let the opportunity to learn from
legendary game engines like the Quake engine which used Xlib to display graphics on POSIX systems to slip
away. It's worthwhile to mention that idSoftware developers used the Win32 API for building the game in
Windows and that this is also what Casey shows in his Handmade Hero series.

NOTE FROM THIS POINT ON IT'S BETTER WRITTEN

## Installing dependencies

Even though your system may as well be using X11 for displaying graphics you probably need to install the
Xlib development packages. In debian based distros you can do so via the apt manager from the command-line:

```sh
sudo apt install libx11-dev libx11-doc
```

where the `libx11-dev` package provides the client interface to Xlib and `libx11-doc` provides the
man pages, which I strongly recommend you to install so that you can consult the Xlib documentation
from your console.

## Headers

The minimal set of headers that we may want to use for displaying graphics and logging
useful information to the user on the console is the following:

```
#include <stdio.h>
#include <X11/Xlib.h>
```

Since we would want to display some information on the console to the user as the game runs
we have included the standard input-output `<stdio.h>` header typically used for logging
normal and error messages and we can channel them through the standard output and error streams
respectively for convenience.

And the `<X11/Xlib.h>` header provides the necessary definitions for the Xlib data structures, macros, and
functions that we need for putting our game graphics on the screen.
It is useful to look at the contents of the header to know what the Xlib's opaque data
structures contain and how some of the query macros are implemented if you wish to take
Handmade Hero development vibe to the next level.
The header should be located in your system in the path
`/usr/include/X11/Xlib.h`. With those definitions we will
be able to open a connection to the XServer, create the window for our game, and put graphics on it.

## Connecting to the XServer

The first step towards displaying a window with Xlib is to establish a connection with
the XServer via the function call `XOpenDisplay()` which takes as argument the
hardware display name. In GNU/Linux it's okay to pass `NULL`, in that case the parameter
resolves to whatever the shell environment variable `DISPLAY` holds. If the call succeeds
the function returns a pointer to the `Display` structure. In the Xlib context this
encompasses not only the monitor for graphics output but also the keyboard, mouse, and other
peripherals for capturing the user input. On the other hand, if the connection to the XServer fails
the function returns a `NULL` pointer and the code should stop and return an error
message for the user.

ABOUT MEMLEAKS THIS IS AN ISSUE IF THE GAME RUNS FOREVER .. NO MATTER WHAT WHEN THE GAME ENDS THE
KERNEL WILL DO THE CLEANUP. THE MEMLEAK PROBLEM IS FOR SERVERS THAT RUN FOR LONG PERIODS OF TIME
EVENTUALLY LOOSE THE MEMORY FOR OTHER PROCESSES.

It's okay to put all the Xlib code in the main function, this aligns with the spirit of exploratory
development philosophy that Casey advocates throughout the series. We are discovering what our game
needs from the underlying platform to put graphics on the screen and handle user input from peripherals
(keyboard, gaming console controller, etc.).

```sh
Display *display = XOpenDisplay(NULL);
if (!display) {
    fprintf(stderr, "%s", "failed to connect to the X Window Server\n");
    return 1;
}
```

## Creating a Window for the Game

This is where the fun part really begins. I strongly recommend you to familiarize yourself with the Xlib
documentation (man pages) and also get ideas from other game engines of the likes of the Quake engine
(which is also a cross-platform game engine) because it uses Xlib for displaying graphics for Unix like
systems such as Linux and other POSIX systems.

The following snippet borrowed from the Xlib official documentation shows that to create a simple window
one needs to have an active connection to the XServer (a display), a parent window, coordinates with
respect to the top-left corner of the parent window, dimensions (width and height), a border width which
can be zero for our purposes, and colors for the border and background:

NOTE WOULD BE NICE TO SAY THAT WINDOW IS AN ALIAS OF AN UNSIGNED LONG INTEGER

```
Window XCreateSimpleWindow(
    Display *display,
    Window parent,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    unsigned int border_width,
    unsigned long border,
    unsigned long background);
```

For the purposes of our putting graphics on screen for our game and keeping things simple we can tell Xlib
that we want the the root window as the parent of our game window via the macro `DefaultRootWindow(display)`
which takes as argument our display. The macro will return the Window ID of the root window. We can pass
zero for both the `x` and `y` coordinates, we could specify standard dimensions for our window such as
1600 x 900 where the width, 1600, and the height, 900, are in pixels.

If you wish to make your code more portable you may get the default screen of your display and use query
macros to use the default dimensions:

```
Screen *screen = DefaultScreenOfDisplay(display);
int width = WidthOfScreen(screen);
int height = HeightOfScreen(screen);
```

There is also a query function for getting the black pixel value for the screen to use it for the
border and background colors:

```
unsigned long BlackPixelValue = BlackPixelOfScreen(screen);
```

Thus to create the window for the game you may call:

```
Window window = XCreateSimpleWindow(
	display,
	DefaultRootWindow(display),
	0,
	0,
	WidthOfScreen(screen),
	HeightOfScreen(screen),
	0,
	BlackPixelOfScreen(screen),
	BlackPixelOfScreen(screen)
);
```

TODO CHECK THESE COMMENTS
It's worth mentioning that this alone won't make the game window visible, you will need to mapped the window.
What happens is that the call tells the XServer to allocate sources for the window but it won't be marked
as available for display until the `XMapWindowEvent` happens.
READ https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Mapping_Windows

TODO REALLY DO TELL THE XID IS A LONG UNSIGNED THAT MEANS THAT THE WINDOW IS A RESOURCE ID THAT WE PASS TO
THE X WINDOW SERVER SO THAT IT KNOWS WHAT TO OPERATE ON

## Peak into Xlib headers

From the `X11/Xlib.h` header can know how the `DefaultRootWindow()` macro is defined

```
#define DefaultRootWindow(dpy)  (ScreenOfDisplay(dpy,DefaultScreen(dpy))->root)
```

where we can see that it depends on other two macros `ScreenOfDisplay()` and `DefaultScreen()` and
a pointer to the display data structure, from the dereferencing operation we can surmise that the result is a 
`Screen` data structure and that one of its field is the Window ID of the root window.

The `Screen` data structure is defined as follows (some fields have been omitted for brevity):

```
typedef struct {
        struct _XDisplay *display;
        Window root;
        int width, height;
        int ndepths;
        Depth *depths;
        Visual *root_visual;
        unsigned long white_pixel;
        unsigned long black_pixel;
} Screen;
```

from the Screen data structure definition we can see that the Window ID of the root window is indeed
present (as expected), we also see the dimensions of the screen, the values for the black and
white pixels for the screen, and a pointer to the display data structure.

```
#define ScreenOfDisplay(dpy, scr) (((_XPrivDisplay)(dpy))->screens[scr])
```

where `_XPrivDisplay` is a pointer type to the `Display` resource;
the display data structure is defined this way when illegal access is enabled
(not showing all the fields for brevity):

```
typedef struct _XPrivDisplay {
    int fd;
    int proto_major_version;
    int proto_minor_version;
    char *display_name;
    int default_screen;
    int nscreens;
    Screen *screens;
} Display, *_XPrivDisplay;
```

from this definition we know that Xlib uses a file descriptor `fd` for the network socket, that
it keeps a record of the major and minor X protocol versions, the display name, the default screen index,
the number of available screens, an array of screen structures, etc.

If you wish to enable illegal access you have to modify the header section of your source in this way

```
#include <stdio.h>
#define XLIB_ILLEGAL_ACCESS 1
#include <X11/Xlib.h>
```

it's important that the definition takes place before including the `X11/Xlib.h` header as shown
(otherwise it has no effect).

You may want to enable this for debugging purposes, not to bypass the intended usage patterns to implement
the platform layer for the game. After enabling illegal access you should be able to use GDB to peak
at the contents of the display data structure.

```
#define DefaultScreen(dpy) (((_XPrivDisplay)(dpy))->default_screen)
```

From this simple example we can see that the macros that we use are used to dereference the display
pointer, and this is an important point in Xlib because this was done by design to allow for changes
in the display structure as the X Protocol goes from version to version. TODO NEEDS IMPROVEMENT

## Mapping the Window

TODO READ DOCUMENTATION FOR MAPPING WINDOW
For the game window to show up needs to be mapped in Xlib idiom because creating the window only
allocates the resources for it. This is in part because some applications may not want to display a
window right away. However we have no reason to defer that operation for the game.

One has to take into account that Xlib operates asynchronously and so we need to wait for the
Exposure event to happen for the window to show up. However it is necessary to change the
window attributes of the simple window to listen to exposure events; without that configuration
the execution of our code would block while waiting for the exposure event to happen.

TODO SURELY THE WORDING CAN IMPROVE HERE

To achieve that we have to call `XChangeWindowAttributes` with an instance of the
`XSetWindowAttributes` data structure, the latter must set the `event_mask` field
with the `ExposureMask` to state that the window will respond to those events. The
reason for waiting for a expose event is that the window would be ready to display
graphics on it.

```
XSetWindowAttributes template = {};
template.event_mask = ExposureMask;
```

The function signature of the function for changing the window attributes is the following

```
int XChangeWindowAttributes(
    Display *display,
    Window w,
    unsigned long valuemask,
    XSetWindowAttributes *attributes
);
```

it takes the usual display and window id, the valuemask which must match the event mask, and a pointer
to the XSetWindowAttributes data structure.

```
XChangeWindowAttributes(display, window, CWEventMask, &template);
``` 

To map the window to make it elegible for display we need to call the `XMapWindow` function with the
display and window id of our game window as parameters to the function.

```   
XMapWindow(display, window);
```

Due to the asynchronous of the X Window system we need to wait for the exposure event to happen for the
game window to display and to do that we need to call the XWindowEvent function which has the following
signature:

```
int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return);
```

again we have the display and the window id, we also need to pass the event mask that corresponds
to the exposure event, and a pointer to the XEvent data structure.

```
XEvent ev = {};
XWindowEvent(display, window, ExposureMask, &ev);
```

This difference between this call and polling for XEvents is that the only one that gets pushed
out of the event queue is the expose graphics event.

## Compilation

For simplicity we have opted to write all the source code in a single source file `linux_handmade.c`
that goes with the convention established by Casey in Handmade Hero to prefix the platform name
for the platform layer code.

```sh
gcc -Wall -g -Og linux_handmade.c -o linux-handmade.bin -lX11
```

where we have enabled all warnings and instructed the compiler to generate debugging symbols and
apply optimizations that do not interfere with the debugging session, and the last one is for 
the linker to link the executable dynamically with Xlib.

It's important to mention that Casey uses a batch file to compile the source code and that's what I also
did because my intention is to experience the cross-platform development. To have a consistent build
I am currently using a Makefile that can be used for compiling the source in Windows (via MinGW) and Linux.

## CLosing the display

TODO IT'S IMPORTANT TO STATE THAT WE DELIBERATELY STICK TO THE HH WAY OF CRASHING THE GAME FOR DEBUGGING
WHILE TRUSTING THE OS TO DO THE CLEANUP.


```
#include <stdio.h>
#include <X11/Xlib.h>

int main() {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
	fprintf(stderr, "%s", "failed to open display\n");
	return 1;
    }
    Screen *screen = DefaultScreenOfDisplay(display);
    Window window = XCreateSimpleWindow(
	display,
	DefaultRootWindow(display),
	0,
	0,
	WidthOfScreen(screen),
	HeightOfScreen(screen),
	0,
	BlackPixelOfScreen(screen),
	BlackPixelOfScreen(screen)
    );

    XSetWindowAttributes template = {};
    template.event_mask = ExposureMask;
    XChangeWindowAttributes(display, window, CWEventMask, &template);

    XMapWindow(display, window);

    XEvent ev = {};
    XWindowEvent(display, window, ExposureMask, &ev);

    XCloseDisplay(display);
    return 0;
}
```
