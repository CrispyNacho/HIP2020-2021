# HIP 2020 - 2021
## Description
This repository contains relevant code and tools for the HIP 2020 - 2021 project.

## Game Data Generator
### How to Build
In order to build this simple C++ program, you'll want to install a compiler. You can download the mingw-64 compiler here:

- https://sourceforge.net/projects/mingw-w64/files/mingw-w64/

Click on "MinGW-W64-install.exe" to install the compiler. Once it's installed, you'll also want to add the `mingw-w64\x86_64-8.1.0-win32-seh-rt_v6-rev0\mingw64\bin` folder to your System Path. Check out this guide [here](https://docs.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14)) for more details.

Once you have the compiler set up, open up a cmd prompt at the GameDataGenerator directory and type the following command:

- `g++ game_data_generator.cpp -o game_data_generator`

This will compile the .cpp file and output an executable that you can then run.
