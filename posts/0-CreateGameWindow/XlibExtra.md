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

TODO CONSIDER DROPPING FROM THE POST; TAKEN FROM FROM HEADERS
It is useful to look at the contents of the header to know what the Xlib's "opaque" data
structures contain and how some of the query macros are implemented if you wish to take
Handmade Hero development vibe to the next level. Later we are going to see that these "opaque"
data structures are incomplete types that the debugger won't be able to peek into. The header has
sufficient information for any developer to know what the actual types are.

The Xlib header should be located in your system in the standard path
`/usr/include/X11/Xlib.h`.
