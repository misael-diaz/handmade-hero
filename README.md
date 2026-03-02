# handmade-hero
following Casey's game dev tutorial

## Requirements

To compile the game you may install Visual Studio or MinGW. The former is
Casey's approach and the latter is my own approach (experimental).

## Compilation

From the MS DOS prompt execute the following command line from the top level of this
project:

```sh
build.bat
```

the `.bat` file is batch file in Windows and what it is similar to some extent to a
bash script, it simply lists the system commands that should be executed and those
are executed in the order that they appear.

The script compiles the handmade hero game.

```sh
gcc src/handmade.c -o obj/handmade.exe -Wl,--subsystem,windows -lgdi32
```

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
