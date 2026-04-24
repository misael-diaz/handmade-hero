
[---]: #

[title: Handmade Hero: A Systems Programming Odyssey]: #

[published: preview]: #

[description: Demystifying Xlib for crafting a window for a custom software renderer targeting GNU/Linux.]: #

[tags: xlib, linux, lowlevel, gamedev]: #

[---]: #

TODO:
- APPLY MIT LIC TO SRC LISTING
- MAKE THE README OF YOUR REPO MORE USEFUL MAYBE EVEN EXPLAIN WHAT MIGHT BE NEW NOT ADDRESSED IN PREVIOUS IMPLEMENTATIONS (TOUGH HANDMADE HERO HAS BEEN DONE BY MANY). THIS IS A MUST IF YOU ARE GOING TO LINK THIS POST TO YOUR GITHUB WHICH YOU ARE TO DO. CONSIDER SHOWING EITHER SNAPSHOTS OR VIDEOS AT KEY COMMITS. WHAT PROBLEMS DID YOU STUMBLED ON TOO SINCE YOU ARE DOING IT IN LINUX.
  THIS RIGHT.
- LINK TO LIBX11 AND PROTO REPOS IN GITLAB IN REFERENCES
- ADD THE GITHUB REPOS ON XLIB AS A REFERENCE TO GIVE DUE CREDIT
- LINK THIS POST FROM GITHUB FOR SEO
- ALSO LINK THE POST ITSELF TO YOUR GITHUB FOR SEO
- MAYBE STATE THAT XLIB CODE IS NOT GOING AWAY WITH WAYLAND SHIFT THERE'S XWAYLAND
- SHOW THE POLLING VERSION FOR XWAYLAND CLIENT APPS FOR SEO AND USE THAT VERSION IN
  THE FINAL SRC CODE LISTING. DON'T FORGET TO CITE THE OFFICIAL SOURCE THAT TALKS ABOUT
  AVOIDING BLOCKING COMMS WITH XWAYLAND
- SHOW THAT STORENAME() CHANGES THE PROPERTIES OF THE WINDOW BEHIND THE SCENES
- ADD USEFUL COMMENTS TO THE FINAL SOURCE CODE THAT EXPLAIN BEYOND THE OBVIOUS GOTCHAS; NOTE THAT YOU
  ARE NOT EXPECTED TO COMMENT ON EACH XLIB CALL. DOING THIS IS IN THE BEST OF YOUR INTEREST.
- UPDATE TAGS IN POSTING SITE
- IMPROVE REASONS FOR STICKING WITH XLIB
- USE BETTER LINKING TAGGING WORDS IN THE OUTLINE PARAGRAPH

## Handmade Hero: A Systems Programming Odyssey

<p><img src="https://raw.githubusercontent.com/misael-diaz/handmade-hero/refs/heads/posts/posts/0-CreateGameWindow/img/x11-arch-code-window-cover.png" alt="Architecture diagram of X11 Server-Client communication next to C code for Xlib window creation." width="100%"></p>

This post is the start of my Handmade Hero journey documentation. For those that do not know what Handmade Hero is, it is a completed game development [series](https://mollyrocket.com/#handmade) done by legendary game engine developer Casey Muratori. In the series, Casey shows how to build an entire game from scratch in Windows to teach us how computers work and write performant software.


For me, and many others that have followed the series, the best way to solidify the knowledge is by writing the code in a different platform (not Windows) and posting about it. The reason for choosing a different platform is that you are forced to explore the platform (with its distinctive characteristics) on your own rather than following an established path or solution. However, writing this post is not all about learning but about sharing knowledge and experiences that might be useful to other developers.


Because of my intention to share knowledge, I have decided to write this post with an academic like format so that it could be used as a reference for low-level systems programming.


The next section presents a table of contents so that this resource can be use a reference and for quick navigation. The following [section](#handmade-hero-why-craftsmanship-still-matters) talks about why Handmade Hero is so relevant even in these times were AI can generate code at scales never seen before. [Next](#reasons-for-creating-a-gnulinux-port-of-handmade-hero) I write about my motivation to create a GNU/Linux port of Handmade Hero. This is followed by a [section](#is-yet-another-xlib-post-necessary) that talks about the value of writing about Xlib, that is, the library that we are going to use to put graphics on our game. [Then](#why-use-xlib-for-graphics-display) I try to answer the difficult question of why still using Xlib for graphics application development even if there are better alternatives, such as libXCB, favored even by highly respected X developers. After that I talk briefly about the client-server [architecture](#clientserver-architecture) of the X windowing system. Then I show how to [install](#installing-dependencies) the development files for developing X client applications in GNU/Linux. Then I write about X application development in [chunks](#developing-an-x-client-application). This is followed by the [conclusions](#conclusions) , [final thoughts](#final-thoughts) on why Xlib is so important, a list of [references](#references) for crediting sources not included in this post but that were useful to me, and a list of Handmade Hero [ports](#ports) that might be of interest.

## Table of Contents

Use the table of contents to get an outline of the post and to navigate to the sections that you might want to read next time you come back.


If you are here for the source code you can copy it from [here](#initial-platform-layer-of-the-game).

- [Handmade Hero: Why craftsmanship still matters](
	#handmade-hero-why-craftsmanship-still-matters
)
- [Reasons for creating a GNU/Linux port of Handmade Hero](
	#reasons-for-creating-a-gnulinux-port-of-handmade-hero
)
- [Is yet another Xlib post necessary?](
	#is-yet-another-xlib-post-necessary
)
- [Why use Xlib for graphics display](
	#why-use-xlib-for-graphics-display
)
- [Client-Server Architecture](
	#clientserver-architecture
)
- [Installing dependencies](
	#installing-dependencies
)
- [Developing an X Client application](
	#developing-an-x-client-application
)
	* [Headers](
		#headers
	)
	* [Connecting to the XServer](
		#connecting-to-the-xserver
	)
	* [Creating a Window for the Game](
		#creating-a-window-for-the-game
	)
	* [Mapping the Window](
		#mapping-the-window
	)
	* [Pausing the Game](
		#pausing-the-game
	)
	* [Destroying the Window](
		#destroying-the-window
	)
	* [Closing the Display](
		#closing-the-display
	)
	* [Initial Platform Layer of the Game](
		#initial-platform-layer-of-the-game
	)
	* [Compilation](
		#compilation
	)
	* [Running the Game](
		#running-the-game
	)
	* [Checking Memory Leaks with Valgrind](
		#checking-memory-leaks-with-valgrind
	)
- [Conclusions](
	#conclusions
)
- [Final Thoughts](
	#final-thoughts
)
- [References](
	#references
)
- [Ports](
	#ports
)

## Handmade Hero: Why craftsmanship still matters

Handmade Hero has never been more relevant now than ever for software developers. Largely because of the increasing pressure of shipping faster by integrating software development with AI, this trend has intensified the work without leaving much time to have developers connect directly with the problem space through the act of driving solutions through handmade code.


As a result of this trend, software developers write code less frequently and their ability to find solutions through writing exploratory code diminishes. It's not hard to see that this can lead to burnout in some cases because the intensity of the work that the developer does has increased by orders of magnitude. Or to some developers it means an eventual loss of confidence in writing a single line of code when compared to the amount of code that AI can generate in that same time.


A practical way of addressing this situation is to work on an engaging side project that keeps your ability to engineer solutions through the act of writing code sharp. This means that as the technology improves and we learn better ways to integrate AI into our workflow our craftsmanship not only remains but evolves.


For me that engaging project has been [Handmade Hero](https://www.youtube.com/watch?v=I5fNrmQYeuI&list=PLnuhp3Xd9PYTt6svyQPyRO_AAuMWGxPzU). Handmade Hero is a low-level systems engineering series that legendary engine developer Casey Muratori streamed for a period of two years to teach developers how computers work and how to write performant software by developing a game from scratch (no libraries and no game engines). This series has helped me encounter a balance between using AI to generate code for work and keep my problem solving and coding skills honed in my free time. 


Even though developers have written about the series many times each post is unique and it matters because it is a transformative experience.

## Reasons for creating a GNU/Linux port of Handmade Hero

Handmade Hero is about learning about how computers work, and Casey did that by showing us how to use the Win32 API to build a game in Windows. And he used Windows because it's the platform he is most familiar with.


By analogy, I want to build the game in GNU/Linux because I am comfortable with the command-line (gives me that "at home" feeling) and because I really want to know my platform better by writing systems programming code. On a side note, I am not adamant at trying to convince anyone to switch to GNU/Linux. You will know if it is for you after interacting with it for a while. I would like to add that I do not intend to skip the Win32 platform because Handmade Hero is a cross-platform development experience.


The scope of this post is to share what I have learned about Xlib to create a game window in GNU/Linux by following the way of the Handmade Hero craftsman. To me that meant that I had to read the Xlib man pages, dive into the source code to peek at its implementation, and also borrow ideas from the Quake-II engine. 


## Is yet another Xlib post necessary?

There are books and many great posts about Xlib, is there really a need for another post?  The authors of the Xlib [guide](https://www.x.org/wiki/guide/) for new developers express that there is plenty of documentation but a large portion of it is outdated or not that well written.  They also express that learning X is tricky, fortunately the learning curve may not be as steep as one might think.  Nevertheless, if you try to dive directly into the Xlib man pages without prior exposure to the [C Language X Interface](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html) you will probably hit a wall (like it happened to me) because of the nomenclature and concepts that you need to understand.  So the man pages are an excellent resource as a reference of the API; however, they were not written to teach you anything in a tutorial kind of way.  Unfortunately, the new developer guide is not considered complete not even by the authors and for that reason there is a gap of knowledge that can be closed (to some extent) by studying the source code and experimenting with it. I would like to add that probably the best way to learn Xlib is by interacting with the X developers and contributing to the project. I wish I could say that I have had that experience but cannot say so at the time of writing this post. To answer the original question, any post about Xlib that tries to look at it from a new angle or that is written with the intention to teach or to share experiences is valuable.


I strongly recommend that if you are serious about learning Xlib that you invest some time into reading the [guide](https://www.x.org/wiki/guide/) for new developers, for it is a great starting point because it provides high-level descriptions about the X Windowing system.


I would like to end this section by saying that now that programming is more widespread, it is more likely for professionals from different domains other than computer science to write code. And I think that is great because they learn about computers from a different standpoint that perhaps is more relatable to non computer scientists and software developers. So I believe it is better for the word to not just have people from different domains writing software but also writing about it.


## Why use Xlib for graphics display

So why use Xlib for putting the graphics of the game on the screen when the official documentation from Xorg recommends not using Xlib but instead use the X C Bindings (XCB) for low-level development or toolkits such as GTK+ or Qt for GUI applications?


To answer that question it is important to have in mind that Xlib development began in the 80s and it has remained backward compatible since then. If it didn't, then devices equipped with older hardware (such as embedded devices and medical equipment with long life cycles) still in use today would be rendered useless. We should recognize that Xlib developers have done the best to preserve backward compatibility while modernizing it. At the present time there is not much that can be done to improve its performance without compromising the existing architecture. To give you an idea of what this means, some Xlib calls incur roundtrips that hinder its performance on modern hardware but those roundtrips are features that are needed to remain backwards compatible. This is one of the major reasons Xlib developers do not recommend using it for new projects. The other reason is that modern GUI applications, such as desktop environments, leverage toolkits tailored for modern applications, such as handling complex international input and ouput. The toolkits have better programming models than the underlying Xlib layer and for that reason applications should call Xlib sparingly (only to reach out for features not provided by the toolkits). For modern low-level application development on embedded devices the obvious choice is XCB. I don't think that anyone would object to that. I strongly recommend reading the official Xorg [guide](https://www.x.org/wiki/guide/) for new developers for those wishing to learn more about this.


The other thing that we have to keep in mind is the philosophy of Handmade Hero &mdash to write exploratory code to learn about the problem space and after getting an idea of what the solution looks like optimize it.  Since we are just starting the development we are content with any code that allows us to display a window for our game on the screen. We do not care if it looks clean or if its performant. What we care is that it works and that we know (to some extent) how it works. And the latter is an important point for me, we have the privilege of having the opportunity to learn from source code written by professional developers because of open-source. This is why I do recommend to read even if it's just the headers of Xlib so that the code that you call is not a black-box to you. This is one of the advantages of developing the game for the Linux platform over doing that for Windows, that you can take your time to study the source code of all the libraries that your game depends on.

Now that we have talked about some general reasons for using Xlib, we can consider some key specific reasons for using Xlib.  In the handmade spirit, what we need for displaying the graphics of our game is a framebuffer structured in a way that the screen can understand.  Also we are not going to need widgets, menus, text, buttons, etc. for our game; this is why we do not need toolkits. What we do need is a window to put the game graphics for the delight of our players and we can achieve that through Xlib calls (see [Xorg-wiki](https://www.x.org/wiki/guide/client-ecosystem/)).  What Xorg developers discourage is reinventing the wheel of the entire stack because it disregards what took several years for professional developers to implement and improve. 



For this use case the Xlib code is relatively straightforward to write.  The client code that one has to write essentially reads by itself, you don't need to know the entire Xlib API to understand it.  This means that the Xlib code that we are going to write is readable and that is a good enough reason for using it in my book.



Another reason for using Xlib is that it has been modernized, it leverages the X C Bindings (XCB) under the hood, it also supports multi-threading (if `XThreads` are initialized on the client side), and it is still maintained to this day by Alan Coopersmith, a veteran Xlib developer, and many others. Surely a properly implemented client code with XCB will perform better than its Xlib counterpart but that depends entirely on your ability to write that code (as mentioned in this XCB [tutorial](https://xcb.freedesktop.org/tutorial/)). Working directly with XCB entails that you must have a fairly good understanding of the X11 protocol.


In the context of handmade game development, reaching for XCB does resonate with the Handmade Hero spirit only after you have determined that Xlib calls limit performance. Casey also advocates that it is best to write a working code first to get a good idea of what the platform layer should be; and only then work on optimizing it if there are factual reasons to do so.  Thus my rationale for using Xlib is that I want to push it to its limits and then after identifying that performance limitations stem from Xlib itself then I would consider using XCB. And if I were to focus on performance bottlenecks, I would look at the game loop code because the less overhead the better so that the game would hit the target framerate. I would not bother too much about the Xlib code preceeding the game loop. The expectation is that the game will at some point leverage XCB in the game loop and use Xlib elsewhere for readability.


To be clear, I do find it appealing to develop the platform layer of the game with XCB.  However, choosing XCB over Xlib right now without having a playable game would be an early optimization for me.  We cannot talk effectively about performance until we have a system running.  This is why that I rather spend time to get a working system sooner with Xlib and then consider the performance bottlenecks. One has to consider that Xlib supports multi-threading and extensions such as the shared memory extension are features that should be explored in addition to speaking directly to the X11 protocol with XCB. It is worthwhile to mention that Xlib and XCB code can coexist, that means that you can have the best of two worlds by using XCB for optimizing the rate limiting features while keeping around the convenient functions that Xlib provides.  After conducting the due research I would be at a better position to assess performance differences and post about what worked best.


Another reason for using Xlib is that Handmade Hero has not been my first experience with game development.  I started learning about game development by reading Quake-II's source code and so by the time I found out about Handmade Hero I knew that Quake uses Xlib to put graphics on the screen in GNU/Linux. It was only natural to avail myself of the experience of diving into Quake's source code to develop my own GNU/Linux port of Handmade Hero.


I would like to tell more about why not using a toolkit with more details here.  The strongest reason for not use a toolkit is because it hides the difficulties of dealing directly with visuals, a lesson on how computers work that I do not want to skip.  Some of those difficulties translate to the platform layer of the game. For example, if the visual is TrueColor (modern monitors) one has to work directly with RGB bitmasks so that the colors shown on the game window look right.  It is important to mention that the XServer (more on that later) still does the heavy lifting to determine the properties of the visual, such as if the visual is a TrueColor or PseudoColor type, if it has a 16-bit or 24-bit depth, the RGB layout in memory (bitmasks), etc. To be precise, you can still experiment with RGB bitmasks with the GTK2 toolkit (see [here]( https://www.manpagez.com/html/gdk2/gdk2-2.24.24/gdk2-GdkRGB.php#gdk-draw-rgb-image)) but you must consider the pros and cons of adding another layer of abstraction to your application; probably, the answer to that lies with you (or with your team).


The last reason is the most personal one.  By the time I began my transition from Windows 7 to Ubuntu 9.10 (code named Karmic Koala) that Linux desktop shipped with libX11 version 1.2.2 (that can be verified via the [manifest](http://old-releases.ubuntu.com/releases/9.10/ubuntu-9.10-desktop-amd64.manifest)).  At that point Xlib already had the modernized XCB transport layer that made my Linux desktop experience so memorable. Knowing that GTK2 at that time was leveraging Xlib code heavily (as can be verified in the [source](https://gitlab.gnome.org/GNOME/gtk/-/tree/gtk-2-18?ref_type=heads)) to create the desktop environment makes me want to build the platform layer of my game with Xlib (call it nostalgia if you may).  Also my desktop is using a version of [Cinnamon](https://github.com/linuxmint/cinnamon) (a fork of GNOME2) that still depends on Xlib and so it is natural for me to stick with it. 


I just want to add here a little note about possible issues that you could stumble upon if you try to run the game (without modification) on a Wayland-based desktop. Xlib blocking calls, such as `XNextEvent` and `XWindowEvent`, must be replaced with their polling alternatives to avoid potential deadlocks due to the asynchronous nature of Wayland. The interested reader is referred to the official Wayland [documentation](https://wayland.freedesktop.org/docs/book/Xwayland.html).


## <a id="clientserver-architecture"></a>Client-Server Architecture

Xlib has a client-server architecture as illustrated in the following diagram:

<p><img src="https://raw.githubusercontent.com/misael-diaz/handmade-hero/refs/heads/posts/posts/0-CreateGameWindow/img/client-server-architecture-diagram.png" alt="X11 Client-Server Architecture Diagram" width="100%"></p>

The diagram shows that the XServer receives the user input from the keyboard, mouse, and possibly a game
controller. The diagram shows that the applications that we use such as the browser and the console
are clients, and that even the desktop environment could be a client (in some GNU/Linux distributions).
I would like to comment here that the shown diagram is a simplified illustration that does not delve
into the fact that the server has both device independent and dependent code. If you wish to look at a
closer depiction of the architecture see the one provided in the
[X Window Concepts](https://www.x.org/wiki/guide/concepts/) section of the guide for new developers.

An interesting finding for me that helped me realize that even desktop environments can also be X clients 
can be found in the Cinnamon desktop environment source code. In case you don't know, the Cinnamon desktop
environment is actively developed by Linux Mint developers.
If you do look at its source code
you will see that a connection to the XServer (via [`XOpenDisplay()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XOpenDisplay)) happens in the [main](
https://github.com/linuxmint/cinnamon/blob/bfc454e799f0284a3c2fd3a0ec11a716b2d425bb/src/main.c#L303)
source file.

The communication between the client and the server happens over the network via the X11 protocol and
so that means that the client could be in a remote machine as well. For security
ssh forwarding (see `man ssh`) is commonly used to encrypt the communication between the client and server.
If both the client and the server are in the same machine, a Unix socket is commonly used for communication.
It is worth mentioning for clarification that the server does not capture the input directly
from the hardware, that is the job of the Linux kernel. And that is important because it hints that the
server implementation is modular.

In general, a client application tells the
XServer what operation it wants to do (such as drawing) and the server responds to the
request by performing that asynchronously. The motivation for this architecture is
that it solves the problem of multiple clients competing for the same portion of the screen.

The server also knows the window the user is currently using (for input) 
and also knows the client that owns it, and so by extension it knows the client that
will respond to the input events.

It helps to bear these aspects of Xlib in mind when reading X client applications.

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

For example if you wish to consult the documentation for opening a display (more on that later)
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

Xlib is the original Application Programming Interface (API) of the X11 protocol, written in C. Although
there are bindings to make it possible to call Xlib from other languages such as Python, Go, and Rust,
you would want to write C code if you wanted to work closer to the metal.

Another reason for using C is that you will be free of the overhead of context switching when reading the
Xlib source code or documentation. The Xlib code will not seem foreign to you as it would if you are
calling it from Python.

If you are familiar with the Handmade Hero series you know that Casey essentially writes C code
on stream despite that he is compiling the source code of the game with a C++ compiler. 
He did not leverage C++ features heavily until implementing vector math (via operator overloading)
to make the operations read more like math to the next developer.

## Headers

C programs typically start with a header section that specifies the function prototypes (declarations) and 
types (data structures) that the program needs to compile the source down to machine code. 

The minimal set of headers that we may want to use for creating the game window, having an improved
debugging experience, and logging useful information to the developer (or user) is the following:

```c
#include <stdio.h>
#define XLIB_ILLEGAL_ACCESS 1
#include <X11/Xlib.h>
```

The standard input-output header `<stdio.h>` provides functions for formatted printing (such as
`fprintf`) which can be used to channel messages through the standard output `stdout` and standard error
`stderr` streams.
By doing this way we can redirect error messages to a file for debugging without the informative
messages that we may want to show to the user.

The `<X11/Xlib.h>` header provides the necessary definitions for the Xlib data structures, macros, and
functions that we need for putting our game graphics on the screen.
The Xlib header should be located in your system in the standard path
`/usr/include/X11/Xlib.h`.
If you read it you are going to find out that client X window applications that
set the `XLIB_ILLEGAL_ACCESS` macro to a truthy value before including `<X11/Xlib.h>` obtain
(limited) access to the `Display` data structure which contains all Xlib internals. 
As we advance in the discussion we are going to use
it to reinforce our understanding that Xlib is asynchronous. I do not recommend to use this hack to
write the platform layer of the game, instead use the macros to make your client code portable, readable,
and maintainable.

With those definitions we will be able to open a connection to the XServer, create the window for our game,
and put graphics on it (in a future post).

## Connecting to the XServer

The first step towards displaying a window with Xlib is to establish a connection with
the XServer via the function call [`XOpenDisplay()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XOpenDisplay) which takes as argument the
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

Behind the scenes `XOpenDisplay()` in Linux opens a socket in non-blocking mode to connect to the XServer
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
via [`XCreateSimpleWindow()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XCreateSimpleWindow) one needs to have an active connection to the XServer (a display), a parent window, coordinates with
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
at first because we are used to thinking with an Object-Oriented Programming OOP mindset. 
We might expect that the `Window` is an object but
that is not the case, it is an Xlib resource Id `XID` of 64-bits in size (`typedef unsigned long XID;`). What
happens under the hood is that the request for creating a window is stored in the `Display` data structure.
That means that the XServer knows nothing about this until we explicitly ask the server to
process this request (more of that later because we still have work to do).
Xlib behaves this way for performance, it stacks our requests until the time is right for processing them. When the server does process the request it will allocate memory for
the window but that memory region is not accessible from client applications (it is
private to the server).

From looking at the display data with gdb we can determine that we have a request that has not been processed
by the server (only showing some of the fields):

```gdb
(gdb) p *display
$1 = {
	fd = 3,
	proto_major_version = 11,
	proto_minor_version = 0,
	vendor = 0x406590 "The X.Org Foundation",
	last_request_read = 6,
	request = 7,
	display_name = 0x406500 ":0",
	default_screen = 0,
	nscreens = 1,
	screens = 0x406a10,
	min_keycode = 8,
	max_keycode = 255,
}
```

The sequence number of the last request considered by the server is 6 (`last_request_read = 6`)
and that happened when we
called [`XOpenDisplay()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XOpenDisplay) and the sequence number of the current request is 7 (`request = 7`). This means
that the request is stored locally in the display structure. The server has not seen it because we have
not called [`XFlush()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XFlush) ourselves to push the requests to the server. Xlib's implementation is
conservative on what function calls have the side-effect of flushing the output buffer (we will call one soon
enough) and so client applications seldom have to call `XFlush()` directly, as stressed in the official
documentation. Only functions that require an immediate response from the server flush the output buffer
and block until the response has been received.
I hope that this has convinced you of the asynchronous nature of the X protocol.

You may also wish to name your window as `Handmade Hero` at this point as Casey did to get that hello
world experience when your game window appears:

```c
XStoreName(display, window, "Handmade Hero");
```

Calling [`XStoreName()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XStoreName) also updates the output buffer with the request to change the name of the window shown in the
window manager, the function does not trigger a synchronization request with the server
as in the previous cases. It is interesting to see that it forwards the operation to the [`XChangeProperty()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XChangeProperty)
function (which implements the corresponding X11 protocol request):

```c
int
XStoreName (
    register Display *dpy,
    Window w,
    _Xconst char *name)
{
    if (name != NULL && strlen(name) >= USHRT_MAX)
        return 0;
    return XChangeProperty(dpy, w, XA_WM_NAME, XA_STRING, /*  */
                           8, PropModeReplace, (_Xconst unsigned char *)name,
                           name ? (int) strlen(name) : 0);
}
```

As can be seen from the code snippet, the function we just called is a convenient wrapper that makes the Xlib code easier to read. The display and window handle arguments are easy to see, the new ones are `Atom` macros,`XA_WM_NAME` and `XA_STRING`, which are defined in the standard [header](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Standard_Header_Files) `X11/Xatom.h`, that help identify the property name and the type of the property. 
The underlying size of the `Atom` is platform dependent (either 32 or 64-bit integer). In this case the name of the window and the string type. 
The magic number `8` tells the server that the property should be interpreted in chunks of 8-bits which make sense for strings. 
The `PropModeReplace` instructs the server to replace whatever it had stored for that property.
If the name that we provide is a NULL pointer or if its length exceeds the maximum
storage size it returns zero to indicate that an error has ocurred. This behavior is not explicitly mentioned
in the man page; however, the official [Xlib documentation](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Errors) state that zero status is used to hint errors. In this case it is up for the client
application to check the status and up to the developer to determine what was the cause. This is a good
example that the source code itself is the ultimate documentation.

The stored name remains in the server until the it receives a request to destroy the window, it is not
stored on the client side.

It is important to note that to make the window visible we will need to make a mapping request, and
that is the topic of the next section.

## Mapping the Window

To make our game window visible we need to add a window mapping request to the XServer. The
Xlib function that allows us to place that request is [`XMapWindow()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XMapWindow). But before doing that
we have to modify the attributes of our game window so that it responds to expose events.

By design, the XServer will not send events unless the client application requests them. From the
context of the ongoing discussion if our game window is not configured to handle (graphics) expose events
the server will not send any.
The interested reader might want to consult this
[resource](https://tronche.com/gui/x/xlib/window/attributes/) for verification.
So, if we were to call the function that looks for events (right now without changing the window attributes)
the call would block until the specific event is received from the server. It is not hard to see that
the server will not send any such event because we have not requested that from the server and so the client
application will hang there indefinitely (a deadlock).

To change the attributes of our game window we must call [`XChangeWindowAttributes()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XChangeWindowAttributes) with an instance of the
[`XSetWindowAttributes`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Window_Attributes) data structure. For the objective of making the window ready for graphics display
all that we need to do is to tell the XServer that we want
to respond to graphics exposure events and therefore we must set the `event_mask` field
with the `ExposureMask` in this way:

```c
XSetWindowAttributes template = {};
template.event_mask = ExposureMask;
```

It is worth mentioning that if we wanted to capture user input from the keyboard we would bitwise OR
the event mask with the `KeyPressMask` and `KeyReleaseMask` as in this example:

```c
template.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;
```

Note that these [event-masks](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Event_Masks) are actually bitmasks and they can be found in `/usr/include/X11/X.h`:

```c
#define NoEventMask                     0L
#define KeyPressMask                    (1L<<0)
#define KeyReleaseMask                  (1L<<1)
#define ExposureMask                    (1L<<15)
```

Just for displaying our game window we only need to set the `event_mask` with the `ExposureMask` and
nothing else really (in the future I could post about user input).

The function [`XChangeWindowAttributes()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XChangeWindowAttributes) enable us to change the window attributes, which has the following signature:

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
mark it as eligible for display via the `XMapWindow`.
The function takes as arguments our display and Window Id:

```c  
XMapWindow(display, window);
```

It's worth noting that at this point we are still adding requests to our display structure locally, the
XServer is unaware of our intentions until we call `XWindowEvent` to wait for the expose
event so that our game window becomes visible. This can be confirmed by looking at the display data:

```gdb
(gdb) p *display
$2 = {
	fd = 3,
	proto_major_version = 11,
	proto_minor_version = 0,
	vendor = 0x406590 "The X.Org Foundation",
	last_request_read = 6,
	request = 10,
	display_name = 0x406500 ":0",
	default_screen = 0,
	nscreens = 1,
	screens = 0x406a10,
	min_keycode = 8,
	max_keycode = 255,
}
```

We can see that the sequence number of last request read has not mutated but the request sequence number
has increased to 10 after calling [`XMapWindow()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XMapWindow). We have requested a name for our game window, 
requested attributes change, and performed a window mapping request; thus, the increment in the
request sequence number adds up perfectly.

The signature of the [`XWindowEvent()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XWindowEvent) function is the following:

```c
int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return);
```

again we have the display and the window id, we also need to pass the event-mask `ExposureMask`
of the expose event, and a pointer to the `XEvent` data structure:

```c
XEvent ev = {};
XWindowEvent(display, window, ExposureMask, &ev);
```

The `XWindowEvent()` call will block until the game window comes into view, which is one of the conditions that causes the server to send
the expose event, as stated in the official documentation for [expose-events](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Expose_Events). 
Because of our use of `XWindowEvent` the only event
that gets pushed out of the event queue is the expose event, all the other events are preserved in the
event queue.

We can verify with gdb that the sequence number of the last request read now matches the current one:

```gdb
(gdb) p *display
$3 = {
	fd = 3,
	proto_major_version = 11,
	proto_minor_version = 0,
	vendor = 0x406590 "The X.Org Foundation",
	last_request_read = 10,
	request = 10,
	display_name = 0x406500 ":0",
	default_screen = 0,
	nscreens = 1,
	screens = 0x406a10,
	min_keycode = 8,
	max_keycode = 255,
}
```

The game window should now be visible on your screen.

<p><img src="https://raw.githubusercontent.com/misael-diaz/handmade-hero/refs/heads/posts/posts/0-CreateGameWindow/img/x11-window-handmade-game.png" alt="X11 Client Window of the Handmade Hero Game" width="100%"></p>

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
The magic number one after `sizeof` tells us that we only want to read one item of size `sizeof(c)`. This
could be a little strange at first and this is why I prefer to use `read` instead but to keep things
to a minimum I decided to stick with `fread`.

## Destroying the Window

As mentioned in the [creating](#Creating-a-Window-for-the-Game) a window section the XServer allocates
resources for the game window and so the right thing to do is to request the server to destroy it before we
close the connection.
The server will destroy all the properties associated to that window and decrement the global
property registry accordingly.

Destroying the game window can be done by calling [`XDestroyWindow()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XDestroyWindow) :

```c
XDestroyWindow(display, window);
```

As you can see the function takes the usual display pointer and window resource Id.
If you look at the implementation of `XDestroyWindow()` in `Xlib` you will find that it stores the
destroy window request in the display structure.
There is no implicit synchronization, it is not needed really because
closing the display does the final synchronization. That's when the server will receive the
destroy request and when the resources allocated on the server side will be freed.

## Closing the Display

At the end of the program you want to close the display via [`XCloseDisplay()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Closing_the_Display) so that Xlib's internal data structures get
freed from the heap memory and to close the socket used for communicating with the XServer via:

```c
XCloseDisplay(display);
```

As mentioned in the preceding [section](#Destroying-the-Window) the XServer performs a final synchronization
via [`XSync()`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#XSync) to send requests in the output buffer (this feature is also mentioned in the man page).

## Initial Platform Layer of the Game

Here's the source code we have written to create a window for our game that we can use to put graphics
on it.

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

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
```

As you can see the Xlib API is quite readable and this is why I have not added comments to the
source code. As stressed in the stream "comments are always outdated". In this case comments should
not be used to tell what the Xlib function does, that's the purpose of the documentation (man pages).
And this is why I recommended you to install and consult Xlib's man pages.

Note that the `XLIB_ILLEGAL_ACCESS` definition has been excluded from the source code to
reinforce that it should not be used except when you have strong reasons for looking at Xlib's internals,
such as debugging and learning purposes. Use Xlib's macros to write portable and readable code.

The current state of the development of this project can be found in this GitHub [repository](
https://github.com/misael-diaz/handmade-hero/
).

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
did because my intention is to experience cross-platform development. To have a consistent build
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

## Checking Memory Leaks with Valgrind

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
==17063== Memcheck, a memory error detector
==17063== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==17063== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==17063== Command: ./linux-handmade.bin
==17063== 
game paused, press enter to continue

==17063== 
==17063== HEAP SUMMARY:
==17063==     in use at exit: 0 bytes in 0 blocks
==17063==   total heap usage: 91 allocs, 91 frees, 100,276 bytes allocated
==17063== 
==17063== All heap blocks were freed -- no leaks are possible
==17063== 
==17063== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

We can see that in the heap summary the system reports 91 allocations (on the heap) and 91 frees and so
this means that by calling `XCloseDisplay()` we have made sure that the client application releases
its memory to the operating system. Even if we don't do that the operating system will reclaim the
memory anyways but it's a good practice to do so. The edge of doing these checks periodically during
development is that you can find errors related to memory more easily, reducing the time needed to
find the faulty line of code.

I would like to mention that leaving out `XDestroyWindow()` from the client code would not affect
the total heap usage results. The reason for that is that the memory for the window is allocated
by the server this request instructs the server to destroy the window and its properties. And even
if we miss the call (due to a crash for example) the server will free the resources allocated to
the client and that of course includes the window.

## Conclusions

In this post we delved into some of Xlib internals to close the gap between the information presented in the
official documentation and what we can understand from reading it in order to be in a better position to
build the foundation of our game based on [Handmade Hero](handmadehero.org). As a result of doing this exercise we
now have a better idea of what happens behind scenes, we no longer perceive Xlib code to be a "black box"
through which we can make a window visible in our desktop environment that a user can interact with.

These are some of the most important achievements and things that we learned from going through this exercise:

- We have an initial idea of what the platform layer of the game (for graphics display) with Xlib might look like.

- In the process we
learned that Xlib has a client-server architecture that enables multiple client applications to draw to the
screen (a shared resource) concurrently; it is the server that processes and resolves those requests to
update the screen framebuffer.

- We also found out that in Xlib the display refers to the
connection to the XServer which manages the screens
(for graphics output) along with the peripherals such
as the keyboard, mouse, or game console controller (for user input).

- We have realized that Xlib provides convenient
macros for getting at the screen, visuals, etc. in a portable way. We have found that under the hood these macros
cast the "opaque" display structure into a known type (private display) and subsequently dereferenced
to get at,
for example, the root window Id, screen dimensions, or the black pixel value for the screen.
These macros have enabled the developers to change Xlib internals while not breaking the
existing client code.

- Additionally, we learned that Xlib allocates the resources for the window on the server side and that the
client
code (our game) allocates the window resource Id (or handle).

- We used the GNU debugger `gdb` to demonstrate that
client applications batch their requests into an output buffer.

- We also saw that Xlib commands that
need synchronization such as `XWindowEvent` flush the output buffer and block until the server responds.

- To verify that our code has no memory leaks we used valgrind's memcheck tool.

- In the end we succeeded in making our game window visible and ready to put graphics on it.

## Final Thoughts

In this current world of software which is comprised by multiple layers of code that we cannot simply
discard or choose not to use it because it is integral to the code that we write, at least we can dive
into it to write better software. For example, if you are a frontend engineer developing with Next.js
maybe taking your coding to the next level means to dive into React's implementation to find out what
happens under the hood. Maybe by doing just that you will know enough about React to write better code
and discover things about JavaScript that you did not know. And I mention this because by diving into
Xlib's implementation I was able to see why just calling `XCreateSimpleWindow` will not just make the
window visible and also learned some interesting things about C such as the comma-operator.
The comma operator can be used for chaining multiple arithmetic operations into a single expression
as shown [here](
https://github.com/mirror/libX11/blob/ff8706a5eae25b8bafce300527079f68a201d27f/include/X11/Xlibint.h#L286
).
It is not hard to imagine that this technique was used by Xlib developers to write maintainable code for
abstracting away operations that are frequently used throughout the codebase.

During my researching for writing this post I did not just learn how to write the X client code for
the initial platform layer of my Handmade Hero game.
By looking at pieces of the history of the development of Xlib from the commit logs
I have truly developed a fondness for the Xlib project that I could not have obtained otherwise.
I discovered the dedication of the developers
that laid out the foundation for the desktop environments for GNU/Linux. The list of Xlib contributors
is extensive but I would like to mention some notable ones.
Keith Packard who has worked on the [development](https://www.xfree86.org/cvs/changes_4_2.html) of the
X Windowing since the days of the XFree86 project and has continued doing so for Xlib until
[2022](https://gitlab.freedesktop.org/xorg/lib/libx11/-/commit/622de26180b295eddd39bd4be1528f2358885095).
Alan Coopersmith has authored several contributions to Xlib for over 20 years and still maintains it.
And Jamey Sharp modernized Xlib with the XCB transport layer
and contributed the Xlib project from
[2006](https://gitlab.freedesktop.org/xorg/lib/libx11/-/commit/6b0158dfad714db5b89c04dbea3aedeafa0fb146)
to
[2011](https://gitlab.freedesktop.org/xorg/lib/libx11/-/commit/83e1ba59c48c79f8b0a7e7aa0b9c9cfd84fa403d);
it is also remarkable that during that same time he was also developing libxcb.
But what it is more impressive is that he kept reviewing code submitted to the Xlib project until
[2017](https://gitlab.freedesktop.org/xorg/lib/libx11/-/commit/2d20890e7ffd3ee88a9ceb25cdd2ac1fe7aaceb6).
It was interesting to see a commit log from Keith Packard mentioning that Jamey Sharp was looking over his
shoulder while he was working on fixing an issue with the XCB transport layer
(here is the link to the commit [log](
https://gitlab.freedesktop.org/xorg/lib/libx11/-/commit/bedfe68259037c5564fe52758c92b9c97729640a
)).
I am certain that there are many other outstanding contributors that made possible the
Linux desktop experience that we enjoy that should also be credited but that would probably require
many more posts like this one.

I think that the Xlib developers deserve more credit than what they get, for the most common thing I have
found are
complaints about the X Windowing system. They modernized the internal implementation while keeping the API
intact and still support older hardware. It is impressive to find out that that some of the Xlib
code that we considered in this post such as `XCreateSimpleWindow()` has only undergone minor changes in 
a period of over 20 years. This makes me reflect on how starkly different the world of software development
must have been in the 90's when some of the veteran Xlib developers were already involved in implementing the
X11 protocol (for the [XFree86](https://www.xfree86.org/) project).
I am assigning the readers (that have made it up to this point) the task of answering to themselves
what of that craftsmanship evinced by these developers they need as a foundation to be outstanding
software developers.

Stayed tuned for the next post.

## References

In this section presents a list of the most relevant resources to learn about the X11 protocol and the C Language Interface libX11 (or commonly known as Xlib): 

- [Xlib - C Language Interface](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html) The official Xorg Technical Specification of the X11 Protocol with the C bindings. 
 
- [The X New Developer's Guide](https://www.x.org/wiki/guide/) Provides a high-level documentation of the X11 Protocol that is tailored for new contributors. It provides information about modern extensions, the asynchronous X C Bindings (XCB), a fresher presentation of the the original C Language Interface libX11, and contributing guidelines.

- [Tronche's Xlib - C Language Interface](https://tronche.com/gui/x/xlib-tutorial/) This is the legacy (release 6 of the) X11 protocol C Language Interface by Christophe Tronche. It is worth mentioning that Tronche brought the Xlib documentation to the web before the X Consortium and this is why it is still a relevant, highly indexed, and searchable resource.

- [Handmade Network Tour through Xlib and related technologies](https://handmade.network/forums/articles/t/2834-tutorial_a_tour_through_xlib_and_related_technologies) A practical tour of Xlib (creating the window, handling events, and working with the framebuffer) to get you on the fast track for developing the Handmade Hero game in GNU/Linux written by Florian Behr.

## Ports

I would like to share some of the well known ports of Handmade Hero to GNU/Linux. I found about them while working on this post. I am sharing them because you may also find them to be useful.

- [First GNU/Linux port of Handmade Hero](https://davidgow.net/handmadepenguin/) A series by David Gow on developing the platform layer for Handmade Penguin (a Linux port of Handmade Hero). David leverages the Simple Direct Media Layer (SDL) for handling graphics, sounds, and user input. This is useful for people that want to defer the hardships of implementing the platform layer with Xlib and ALSA.

- [Multi-Platform Handmade Hero](https://github.com/laszlokorte/handmade-hero) A comprehensive implementation of Handmade Hero on various platforms Laszlo Korte. He has succeeded in porting the game to MacOS, iOS, Android, and GNU/Linux (both Wayland and X11). His work demonstrates that well designed game layer written in one platform can be easily ported to other platforms.
