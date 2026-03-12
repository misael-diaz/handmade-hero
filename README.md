# handmade-hero

Academic Purposes.

I am following Casey's game dev [Handmade Hero](https://www.youtube.com/watch?v=I5fNrmQYeuI&list=PLnuhp3Xd9PYTt6svyQPyRO_AAuMWGxPzU) series to get the experience of porting a video game developed for Windows to
GNU/Linux. This is no easy task not only because these are two very different platforms but also because
I have little experience developing GUI applications on both platforms. So as I am learning from Casey's
series how to develop a GUI application in Windows I have to do research of my own to develop it in GNU/Linux.
The latter means digging through the Xlib documentation which is essentially a reference manual.

## Requirements

On Windows you need to install MinGW and Xlib to compile the game into an executable. It's likely that
you won't need to install anything on most GNU/Linux distributions since they are shipped with GCC and
Xlib. It's only a recent thing that some GNU/Linux distros have stopped using Xlib.

In contrast to Casey's approach I am using makefiles to build the project instead of a batch script.
This is because you can build the game with makefiles on both Windows and GNU/Linux.

## Configuration

Clone the project with `git` and edit the `make-inc` file. What you want to set is the PLATFORM
to either `win32` or `linux`.

If you don't provide a platform you will run into a build error, essentially there won't a be a rule
to perform the build and `make` will complain about that (this was in part a design decision).

## Compilation

Go to the top level directory of the project in your machine from your
terminal and execute the following command to compile the project in Windows:

```sh
mingw32-make
```

where we are assuming that you are going to build the project with MinGW in Windows.

And in GNU/Linux you can simply use:

```sh
make
```

note that GNU make should be part of the tools that your distro provides by default and so
you should not need to install anything else really.

## Libraries

Since we are developing while following Casey's series you will notice that we might use libraries
and these might be platform dependent. This will be the case until I reach the point in the series
where we rely on our own code at least for the Windows platform because I do not intend to reinvent
Xlib on GNU/Linux.

Here are some notes that I wrote for the Windows platform that I might want to look at later:

To display graphics for an application compiled with MinGW we need to link
the executable with the libgdi32 library; that's the use of the flag `-lgdi32`.
And the flag `-Wl,--subsystem,windows` tells the GNU linker to sets the entry point for
a Windows GUI application. If you look at the source code you will see that there's
no explicit implementation of main as usual for console applications (and that's the
case for both Windows and GNU/Linux).

Last but not least, you will notice that I am using C not C++ as Casey did and the reason
for that is that he is mostly writing C code with the exception of operator overloading
which I do not plan to rely on. That's where my code is going to differ. (Assuming that
I would have enough free time to make it there.)

## Troubleshooting

If the compilation fails make sure that the `obj` and `bin` or `exe` directories exist and that you have
installed MinGW and that `gcc` and `make` have been added to `PATH`. This is mostly for the Windows
platform and surely this is part of the documentation that needs improvement because so many things
can go wrong and it takes time to cover all the issues and that can change as the project advances.
