## Handmade Hero: Why craftmanship still matters

Handmade Hero has never been more relevant than now for software developers because of the pressure of
shipping faster in part because of AI integration into software development workflows. That means that
software developers write code less frequently and so their ability to find solutions through writing
exploratory code diminishes. It's not hard to see that this can lead to burnout in some cases because
the intesity of the work that the developer does has increased by orders of magnitude. Or to some
developers it means an eventual loss of confidence in writing a single line of code when compared to
the amount of code that AI can generate in that same time interval.

A practical way of addressing this situation is to work on an engaging side project that keeps your
ability to engineer solutions through the act of writing code sharp. This means that as the technology
improves and we learn better ways to integrate AI into our workflow our craftmanship not only remains
but evolves.

For me that engaging project has been Handmade Hero. Handmade Hero is a low-level systems engineering series
that legendary engine developer Casey Muratori streamed for a period of two years to teach developers
how computers work and how to write performant software by developing a game from scratch
(no libraries and no game engines). This series has helped me encounter a balance between using AI to
generate code for work and keeping my problem solving and coding skills honed in my free time.

Even though developers have written about the series many times each post is unique and it matters because
it is a transformative experience.

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

There are great posts about Xlib, is another post necessary? Xlib's documentation is extensive
but it was not written for teaching it was written by experts at a time where
computers had very limited resources and the dominant language for systems programming was
probably C. Now programming is more widespread and there are more languages and more domains
that leverage software solutions and so it is not uncommon to have professionals from
other fields than computer science or software development and also hobbyists writing code.
And so it's better for the world to have more people from different backgrounds writing about
code.

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

TODO ADD THE XLIB_ILLEGAL_ACCESS AND TALK ABOUT WHY YOU ADD IT AND WRITE THE DEBUG CODE TO ILLUSTRATE

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

You may also wish to name your window as `Handmade Hero` at this point as Casey did to get that hello
world experience when your game window appears:

```c
XStoreName(display, window, "Handmade Hero");
```

It is important to note that to make the window visible we will need to make a mapping request, and
that is the topic of the next section.

## Mapping the Window

To make our game window visible we need to add a window mapping request to the XServer. The
Xlib function that allows us to place that request is `XMapWindow()`. But before doing that
we have to modify the attributes of our game window so that it responds to graphics exposure events.
Xlib's graphics exposure events can be thought to be analogous to Win32 `WM_PAINT` messages, this
parallelism would immediately resonate with those familiar with the Handmade Hero series.

By design, the XServer will not send events unless the client application requests them. From the
context of the ongoing discussion if our game window is not configured to handle graphics expose events
the server will not send any.
The interested reader might want to consult this
[resource](https://tronche.com/gui/x/xlib/window/attributes/) for verification.
So, if we were to call the function that looks for events (right now without changing the window attributes)
the call would block until the specific event is received from the server. It is not hard to see that
the server will not send any such event because we have not requested that from the server and so the client
application will hang there indefinitely (a deadlock).

To change the attributes of our game window we must call `XChangeWindowAttributes` with an instance of the
`XSetWindowAttributes` data structure. For the objective of making the window ready for graphics display
all that we need to do is to tell the XServer that we want
to respond to graphics exposure events and therefore we must set the `event_mask` field
with the `ExposureMask` in this way:

```c
XSetWindowAttributes template = {};
template.event_mask = ExposureMask;
```

It is worth mentioning that if we wanted to capture user input from the keyboard we would bitwise OR
the even mask with the `KeyPressMask` and `KeyReleaseMask` as in this example:

```c
template.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;
```

Note that these event-masks are actually bitmasks and they can be found in `/usr/include/X11/X.h`:

```c
#define NoEventMask                     0L
#define KeyPressMask                    (1L<<0)
#define KeyReleaseMask                  (1L<<1)
#define ExposureMask                    (1L<<15)
```

Just for displaying our game window we only need to set the `event_mask` with the `ExposureMask` and
nothing else really (in the future I could post about user input).

The function signature of the function that enable us to change the window attributes is the following:

```c
int XChangeWindowAttributes(
    Display *display,
    Window w,
    unsigned long valuemask,
    XSetWindowAttributes *attributes
);
```

It takes the usual display pointer and window id,
a bitmask that corresponds to the window attributes that we want to change, and a pointer
to the `XSetWindowAttributes` data structure.
Because the only thing that we want to tell the server is about the events the game window will respond
to we only need to pass the `CWEventMask` bitmask, this one is also defined in the `<X11/X.h>` header:

```c
#define CWEventMask             (1L<<11)
```

You may also find it worthwhile to read about Xlib's window attributes
[here](https://tronche.com/gui/x/xlib/window/attributes/).

The call in our code would look like this:

```c
XChangeWindowAttributes(display, window, CWEventMask, &template);
``` 

Now we are in a good position to place the request of mapping the game window to the XServer to
mark it as elegible for display via the `XMapWindow`.
The function takes as arguments our display and Window Id:

```c  
XMapWindow(display, window);
```

It's worth noting that at this point we are still adding requests to our display structure locally, the
XServer is unaware of our intentions until we call `XWindowEvent` to wait for the graphics expose
event so that our game window becomes visible. The signature of the `XWindowEvent`
is the following:

```c
int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return);
```

again we have the display and the window id, we also need to pass the event-mask `ExposureMask`
of the graphics expose event, and a pointer to the XEvent data structure:

```c
XEvent ev = {};
XWindowEvent(display, window, ExposureMask, &ev);
```

This is fine to make our window visible, note that the only event
that gets pushed out of the event queue is the expose graphics event; all the other events are preserved
in the queue.

## Pausing the Game

One thing that you want to do is to pause the game to be able to see the window on your screen and this
can be done in simply by requesting a read from standard input:

```c
char c = 0;
fprintf(stdout, "%s", "game paused, press enter to continue\n");
fread(&c, sizeof(c), 1, stdin);
```

where the `fprintf` shows a instructive message to the user (the game is paused and press enter to
continue) on standard output and `fread` is used to read a byte from the input stream `stdin`.
The `fread` call
is a blocking call and that means that the code will not proceed until a character has been read,
effectively pausing our game. Note that the variable `c` is just a placeholder for the byte to be read.
The `sizeof()` function is commonly used by Linux kernel programmers to write type independent code
(the placeholder may change but the fact that we intend to write into the entire placeholder will not).
The magic number one after `sizeof` tells that we only want to read one item of size `sizeof(c)`. This
could be a little strange at first and this is why I prefer to use `read` instead but to keep things
to a minimum I decided to stick with `fread`.

## Closing the display

At the end of the program you want to close the display so that Xlib's internal data structures get
freed from the heap memory and to close the socket used for communicating with the XServer via:

```c
XCloseDisplay(display);
```

## Compilation

For simplicity we have opted to write all the source code in a single source file `linux_handmade.c`
that goes with the convention established by Casey in Handmade Hero to prefix the platform name
for the platform layer code.

```sh
gcc -Wall -g -Og -gdwarf-4 linux_handmade.c -o linux-handmade.bin -lX11
```

where we have enabled all warnings and instructed the compiler to generate debugging symbols in
DWARF version 4 format that `valgrind` (memcheck tool) understands and
apply optimizations that do not interfere with the debugging session, and the last one is for 
the linker to link the executable dynamically with Xlib.

It's important to mention that Casey uses a batch file to compile the source code and that's what I also
did because my intention is to experience the cross-platform development. To have a consistent build
I am currently using a Makefile that can be used for compiling the source in Windows (via MinGW) and Linux.
The advantage of using a Makefile is that one can extend it for other platforms -- "one Makefile
to build them all".

## Running the Game

To run the game from the command-line:

```sh
./linux-handmade.bin
```

where the dot slash means execute this relative to this path. This is important because if you are new
to GNU/Linux you don't know that commands and executables must be in the PATH in order to run them.
By default the current working directory is in the PATH so by using that notation you are saying the
path of the executable is relative to the current directory.

## Checking for Memory Leaks with Valgrind

Valgrind is a tool suite for debugging and profiling programs. I use often to check for memory leaks
at the end of the program. In the context of our simple client application we are making sure that
by calling `XCloseDisplay()` all the internals allocated in the heap memory are freed. In Xlib there
are some resources that we need to free ourselves by calling `XFree`; there are others and so it
is important to check the official documentation for the right function call to free a resource.

To use the tool to check for memory leaks use the following command:

```sh
valgrind -s ./linux-handmade.bin
```

where the `-s` flag informs valgrind that we want a list of detected (and suppressed) errors so
that we can address them if any. 

The output for this code is the following:

```
==16548== Memcheck, a memory error detector
==16548== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==16548== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==16548== Command: ./linux-handmade.bin
==16548== 
==16548== 
==16548== HEAP SUMMARY:
==16548==     in use at exit: 0 bytes in 0 blocks
==16548==   total heap usage: 89 allocs, 89 frees, 98,228 bytes allocated
==16548== 
==16548== All heap blocks were freed -- no leaks are possible
==16548== 
==16548== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

We can see that in the heap summary the system reports 89 allocations (on the heap) and 89 frees and so
this means that by calling `XCloseDisplay()` we have made sure that the client application releases
its memory to the operating system. Even if we don't do that the operating system will reclaim the
memory anyways but it's a good practice to do so. The edge of doing these checks periodically during
development is that you can find errors related to memory more easily, reducing the time needed to
find the faulty line of code.


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

    XStoreName(display, window, "Handmade Hero");
    XSetWindowAttributes template = {};
    template.event_mask = ExposureMask;
    XChangeWindowAttributes(display, window, CWEventMask, &template);

    XMapWindow(display, window);

    XEvent ev = {};
    XWindowEvent(display, window, ExposureMask, &ev);

    char c = 0;
    fprintf(stdout, "%s", "game paused, press enter to continue\n");
    fread(&c, sizeof(c), 1, stdin);

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

ADD HERE WHOMEVER THAT REVISED YOUR CODE NONE

TODO:
- need like headers and bullets for fast readers with the text (the meat) following for those interested
in the details

IMPORTANT TO MENTION THAT LIBXCB IS LINKED DYNAMICALLY AND YOU CAN EVEN SHOW ldd output

NOTE WOULD BE NICE TO SAY THAT WINDOW IS AN ALIAS OF AN UNSIGNED LONG INTEGER

IMPORTANT TODO
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


TODO ILLEGAL ACCESS and GDB
YOU PROBABLY WANT TO SHOW OUTPUT FROM GDB TO CONVINCE THEM THAT XLIB BEHAVES ASYNC AND THAT OUR REQUESTS
ARE JUST STORED IN DISPLAY STRUCT UNTIL WE PROCESS EVENTS


TODO IMPORTANT LATENCY CONSIDERATIONS WHY NO ROUNDTRIPS FOR PERFORMANCE WHY NOT TRY TO SYNC WITH THE SERVER
ON EVERY OPERATION MENTION THAT TOO.

TODO MENTION THAT THE ACTUAL DISPLAY TYPE LIVES IN THE XLIBINTERNALS XLIBINT.H HEADER

TODO NICE TOUCH OF SHOWING THE NAME OF THE WINDOW BEFORE MAPPING SECTION
