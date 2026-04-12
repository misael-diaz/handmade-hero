## Handmade Hero: Why craftmanship still matters

Handmade Hero has never been more relevant than this time with so many developers saying that they are
burnedout or some other serious issue like not being able to write a single line of code on their own 
due to heavy AI usage. For me Handmade Hero has helped me encounter a balance between using AI to
generate code for work and keeping my problem solving and coding skills honed in my free time.

If you have never heard about Handmade Hero well today is your lucky day. Handmade Hero is a game
development series done by legendary game engine developer Casey Muratori in which he shows how to
build a cross-platform game from scratch, no libraries and no engine to teach you how computers work
and how to craft performant software.

If this sounds appealing I would also recommend to join the Handmade Network to connect
with other developers that might have similar experiences and ways of coping with the
difficulties that you may be going through. Also sharing your experiences and knowledge
can help you realize that you are not obsolete your skills are not obsolete but valuable
to a whole community whose mantra is to build performant software.

## Reasons for creating a GNU/Linux port of Handmade Hero

Handmade Hero is about learning about how computers work, and Casey did that by showing
us how to use the Win32 API to build a game in Windows. And he used Windows because it's
the platform he is most familiar with.

In analogy I want to build the game in GNU/Linux because I have been coding in that
platform and using the command line for a long time. I get that I am at home
whenever I am using GNU/Linux but, I don't intend to try convince anyone to switch to
GNU/Linux. And last but not least I respect the platform preferences other developers
have. And so it is only natural to me to work on the port in GNU/Linux because I want
to know my platform better by diving as deep as I can at my current level of systems
programming profiency. (That does not mean that I won't do the Win32 version of
Handmade Hero because it's truly a cross-platform development experience).

The scope of this post is to  share what I have learned about Xlib to create a game window in GNU/Linux
by following the way of the Handmade Hero craftman. To me that meant that I had to
read the Xlib man pages, dive into the source code to peek at its implementation, and also
borrow ideas from the Quake-II engine.

## Why use Xlib for graphics display

So why use Xlib for putting graphics on the game window when even the recommendation
from Xorg developers is to not use it but instead use a toolkit such as GTK+ or Qt?
The rational for choosing Xlib is simple, all that we need is a framebuffer to put
the game graphics on the screen and it so happens that doing just that is not that
difficult with Xlib. If we were to create a desktop environment then maybe we would
reach out for the toolkits but for the purposes of doing low level systems programming
for learning about some aspects of how computers work the Xlib pathway is the way to
go in GNU/Linux.

The other reason for using Xlib is that Handmade
Hero has not been my first game development experience for a deep systems programming
experience. I started with Quake-II so by the time I found out about Handmade Hero I 
knew that Quake's engine uses Xlib to put graphics on the screen in GNU/Linux. It was
only natural to avail myself of the experience of diving into Quake engine source
code to develop my own GNU/Linux port of Handmade Hero.

The last reason is that Xlib is a core component of Cinnamon my favorite desktop
environment, and so it's only natural for me to stick with Xlib since desktop needs
a running XServer for display.

## Xlib's client-server architecture overview

Xlib has a client-server architecture in which the client applications tell the
XServer what they want to draw and into what window and the server responds to the
request by performing those requests asynchronously. This solves the problem of
multiple clients competing for the same portion of the screen for drawing graphics.

The server also knows what's the window that the user is using to know to what client
application events need to be sent to typically via the network. If both the client
and the server are in the same machine a Unix socket is used for the communication.

With these ideas in mind one can read more easily an Xlib client application.

## Installing dependencies

If you want to follow along you would probably need to install the development libraries
for creating client X11 applications. If you have not done that before the likeliness is
that they may not be installed in your system even if your desktop environment still
uses Xlib.

On Debian based distributions (such as Ubuntu and Linux Mint) you can do so by
invoking the package manager from the command-line:

```sh
sudo apt install libx11-dev libx11-doc
```

- `libx11-dev` package provides the client interface to Xlib
- `libx11-doc` provides the official Xlib documentation as man pages

For example if you wish to consult the documentation for openning a display (more on that later)
you can use the command-line string:

```sh
man XOpenDisplay
```

if that does not work use

```sh
man 3 XOpenDisplay
```

the `3` tells `man` that what follows is a function from a library.

Now we can start writing our X client code to create a game window for our game.

## Developing an X Client application

We are going to be writing C code to develop the X client application because we don't need any of
the facilities that C++ provides to do just that. It's worth mentioning that Casey used C++ features
thoughtfully; for example, he reached out for operator overloading for making the vector math more
readable. (I know that not because I have made it that far into the series but because I have seen
mention that in other streams.)

## Headers

C programs typically start with a header section that specifies the functions and data structures that
the program needs to compile source to machine code. 

The minimal set of headers that we may want to use for displaying graphics and logging
useful information to the developer or user on the console is the following:

```c
#include <stdio.h>
#include <X11/Xlib.h>
```

The standard input-output header `<stdio.h>` provides functions for formatted printing (such as
`fprintf`) which can be used to channel messages through the standard output `stdout` and standard error
`stderr` streams.
By doing this way we can redirect error messages to a file for debugging without the informative
messages that we may want to show to the user.

The `<X11/Xlib.h>` header provides the necessary definitions for the Xlib data structures, macros, and
functions that we need for putting our game graphics on the screen.
It is useful to look at the contents of the header to know what the Xlib's "opaque" data
structures contain and how some of the query macros are implemented if you wish to take
Handmade Hero development vibe to the next level. Later we are going to see that these "opaque"
data structures are incomplete types that the debugger won't be able to peek into. The header has
sufficient information for any developer to know what the actual types are.

The Xlib header should be located in your system in the standard path
`/usr/include/X11/Xlib.h`.

With those definitions we will be able to open a connection to the XServer, create the window for our game,
and put graphics on it.

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

It's okay to put all the Xlib code in the main function, this aligns with the spirit of exploratory
development philosophy that Casey advocates throughout the series. We are discovering what our game
needs from the underlying platform to put graphics on the screen and (maybe in a future post)
handle user input from peripherals (keyboard, gaming console controller, etc.).

```c
Display *display = XOpenDisplay(NULL);
if (!display) {
    fprintf(stderr, "%s", "failed to connect to the X Window Server\n");
    return 1;
}
```

Behind the scenes `XOpenDisplay` in Linux opens a socket in non-blocking mode to connect to the XServer
so that the server may handle requests from multiple clients asynchronously.
If the XServer is
running in localhost then the connection to the XServer happens through a Unix socket for performance.
It is interesting to note that the code that performs the socket connection is not present in Xlib but
in libXCB.

The file descriptor of the socket is stored in the Display structure. I am mentioning it because
Xlib keeps the file descriptor around for subsequent manipulations via `fcntl` calls, not because
we are expected to read (write) from (to) the socket ourselves.
Display also contains other useful
info that's fetched from the XServer such as the minimum and maximum values of the keyboard codes
(we care about this for handling user input), the number of screens, the default graphics
context, etc.

From the XServer we also get valuable information about the screens such as the default dimensions
(width and height), the values of the white and black pixels, the screen depth, and the visual information.
In particular the visual info stores the RGB masks so that we know what the RGB layout that the XServer
expects for packing the pixel data; otherwise, the colors that you see on the screen are not going to be what
you might expect.

Xlib also gets the default screen by parsing the DISPLAY environment variable by calling dedicated libXCB
utils. For example if `DISPLAY` is `:0` we know that the XServer is running in localhost and that the default
screen number is zero. This matters to us because we use the default screen for our game window as you
will see shortly.

## Creating a Window for the Game

This is where the fun part really begins. I strongly recommend you to consult the Xlib
documentation (man pages) as you read the code snippets to familiarize yourself with some aspects of Xlib
that we cannot possibly cover in a post like this.

The other recommendation is to read the source code that the Quake engine uses to display graphics on
screen with Xlib. I am sharing the
[link](https://github.com/id-Software/Quake-2/blob/master/linux/rw_x11.c)
to the source file for your convenience.

The following snippet borrowed from the Xlib official documentation shows that to create a simple window
one needs to have an active connection to the XServer (a display), a parent window, coordinates with
respect to the top-left corner of the parent window, window dimensions (width and height),
a border width which can be zero for our purposes, and colors for the border and background:

```c
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

For the purposes of putting graphics on screen for our game and keeping things simple we can tell Xlib
that we want the root window as the parent of our game window via the macro `DefaultRootWindow()`
which takes as argument our display. The macro will return the Window ID of the root window. We can pass
zero for both the `x` and `y` coordinates, we could specify standard dimensions for our window such as
1600 x 900 where the width, 1600, and the height, 900, are in pixels.

If you wish to make your code more portable you may get the default screen of your display and use query
macros to use the default dimensions:

```c
Screen *screen = DefaultScreenOfDisplay(display);
int width = WidthOfScreen(screen);
int height = HeightOfScreen(screen);
```

There is also a query function for getting the black pixel value for the screen to use it for the
border and background colors:

```c
unsigned long BlackPixelValue = BlackPixelOfScreen(screen);
```

Thus to create the window for the game you may write:

```c
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
```

Bear in mind that this alone will not make the window visible and this could be a little surprising
at first because we are used to think in the OOP terms. We might expect that the `Window` is an object but
that is not the case it is actually an Xlib resource Id `XID` 64-bits wide. What
happens under the hood is that the request for creating a window is stored in the `Display` data structure.
That means that the XServer knows nothing about this until we explicitly ask the server to
process this request (more of that later because we still have work to do).
Xlib behaves this for performance, it stacks our requests until the time is right for processing them.

To make the window visible we need to make a mapping request.

## Mapping the Window

To make our game window visible we need to stack a window mapping request to the XServer. The
Xlib function that allows us to place that request is `XMapWindow()`. But before doing that
we have to modify the attributes of our simple window so that it responds to exposure events.
Graphics exposure events can be thought to be analogous to Win32 `WM_PAINT` messages (those
familiar with the series would recall that from the very first few episodes).

After our game window gets a graphics exposure event it will be ready to displaying graphics.
Because simple windows do not respond to exposure events we must change its attributes.

To achieve that we have to call `XChangeWindowAttributes` with an instance of the
`XSetWindowAttributes` data structure, the latter must set the `event_mask` field
with the `ExposureMask` to state that the window will respond to those events. The
reason for waiting for a expose event is that the window would be ready to display
graphics on it.

```c
XSetWindowAttributes template = {};
template.event_mask = ExposureMask;
```

The function signature of the function for changing the window attributes is the following

```c
int XChangeWindowAttributes(
    Display *display,
    Window w,
    unsigned long valuemask,
    XSetWindowAttributes *attributes
);
```

it takes the usual display and window id, the valuemask which must match the event mask, and a pointer
to the XSetWindowAttributes data structure.

```c
XChangeWindowAttributes(display, window, CWEventMask, &template);
``` 

To map the window to make it elegible for display we need to call the `XMapWindow` function with the
display and window id of our game window as parameters to the function.

```c  
XMapWindow(display, window);
```

Under the hood these requests have been stored in the `Display` structure locally and so the server is
unaware of all of them. This explains the asynchronous nature of the X Window system, and this makes
perfect sense, since the network is a precious resource that must be used wisely to create performant
applications. 

Now we are ready to call
XWindowEvent function
Due to the asynchronous of the X Window system we need to wait for the exposure event to happen for the
game window to display and to do that we need to call the 
 which has the following
signature:

```c
int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return);
```

again we have the display and the window id, we also need to pass the event mask that corresponds
to the exposure event, and a pointer to the XEvent data structure.

```c
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


TODO REVISE THAT THE SOURCE CODE COMPILES


```c
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

## References

Here's a list of additional resources that I have found to be useful to learn about Xlib:

- https://tronche.com/gui/x/xlib-tutorial/
- https://handmade.network/forums/articles/t/2834-tutorial_a_tour_through_xlib_and_related_technologies
- https://github.com/Faison/xlib-learning
- https://github.com/QMonkey/Xlib-demo

## Ports

- https://davidgow.net/handmadepenguin/
- https://dailyollie.hashnode.dev/building-handmade-penguin-0-a-linux-journey-using-xlib-inspired-by-handmade-hero

DON'T FORGET TO ADD THE MACPORT

NOTE MAYBE THAT'S THE BEAUTY OF OPEN SOURCE WHAT WE HAVE NOW STILL AND WHAT WE SHOULD STRIVE TO PRESERVE

I am sharing them for two reasons.
First to give credit to those that made them available and second
because they may also be helpful to someone else.

## Conclusions

In this current world of software which is comprised by multiple layers of code that we cannot simply
discard or choose not to use it because it is integral to the code that we write, at least we can dive
into it to write better software. For example, if you are a frontend engineer developing with Next.js
maybe taking your coding to the next level means to dive into React's implementation to find out what
happens under the hood. Maybe by doing just that you will know enough about React to write better code
and discover things about JavaScript that you did not know. And I mention this because by diving into
Xlib's implementation I was able to see why just calling `XCreateSimpleWindow` will not just make the
window visiable and also learned some interesting things about C such as
[expression-based macros](
https://github.com/mirror/libX11/blob/ff8706a5eae25b8bafce300527079f68a201d27f/include/X11/Xlibint.h#L286
)
(macros that leverage the comma operator by chaining multiple arithmetic operations into a single expression).

## Acknowledgements

ADD HERE WHOMEVER THAT REVISED YOUR CODE

TODO:
- need like headers and bullets for fast readers with the text (the meat) following for those interested
in the details

IMPORTANT TO MENTION THAT LIBXCB IS LINKED DYNAMICALLY AND YOU CAN EVEN SHOW ldd output

NOTE WOULD BE NICE TO SAY THAT WINDOW IS AN ALIAS OF AN UNSIGNED LONG INTEGER

IMPORTANT
===================
THE DISPLAY STRUCT HAS THE LAST_REQUEST_READ AND REQUEST WHICH SHOWS CLEARLY WHEN XLIB
HAS MADE AN ATTEMPT TO PROCESS A REQUEST

IT'S NOT UNTIL WE DO A XWINDOWEVENT CALL THAT THESE TWO COUNTERS MATCH AND SO ALL EVENTS
HAVE BEEN READ AND THIS IS WHY THE WINDOW SHOWS UP. I THINK THIS IS IMPORTANT FOR THE
DISCUSSION

APPENDIX
## Appendix

## Peeking into Xlib headers

From the `X11/Xlib.h` header can know how the `DefaultRootWindow()` macro is defined

```c
#define DefaultRootWindow(dpy)  (ScreenOfDisplay(dpy,DefaultScreen(dpy))->root)
```

where we can see that it depends on other two macros `ScreenOfDisplay()` and `DefaultScreen()` and
a pointer to the display data structure, from the dereferencing operation we can surmise that the result is a 
`Screen` data structure and that one of its field is the Window ID of the root window.

The `Screen` data structure is defined as follows (some fields have been omitted for brevity):

```c
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

```c
#define ScreenOfDisplay(dpy, scr) (((_XPrivDisplay)(dpy))->screens[scr])
```

where `_XPrivDisplay` is a pointer type to the `Display` resource;
the display data structure is defined this way when illegal access is enabled
(not showing all the fields for brevity):

```c
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

```c
#include <stdio.h>
#define XLIB_ILLEGAL_ACCESS 1
#include <X11/Xlib.h>
```

it's important that the definition takes place before including the `X11/Xlib.h` header as shown
(otherwise it has no effect).

You may want to enable this for debugging purposes, not to bypass the intended usage patterns to implement
the platform layer for the game. After enabling illegal access you should be able to use GDB to peek
at the contents of the display data structure.

```c
#define DefaultScreen(dpy) (((_XPrivDisplay)(dpy))->default_screen)
```

From this simple example we can see that the macros that we use are used to dereference the display
pointer, and this is an important point in Xlib because this was done by design to allow for changes
in the display structure as the X Protocol goes from version to version. TODO NEEDS IMPROVEMENT


MAPPING WINDOWS

It's worth mentioning that this alone won't make the game window visible, you will need to mapped the window.
What happens is that the call tells the XServer to allocate sources for the window but it won't be marked
as available for display until the `XMapWindowEvent` happens.
READ https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Mapping_Windows

