# VJass IDE

Simple standalone IDE for the scripting languages JASS and vJass of the computer game Warcraft III which provides live syntax checking and auto completion.

## Automatic Build with TravisCI

[![Build Status](https://travis-ci.org/tdauth/vjasside.svg?branch=master)](https://travis-ci.org/tdauth/vjasside)

## Motivation

There are some JASS and vJass tools out there such as [TESH](https://www.hiveworkshop.com/threads/a-new-tesh-syntax-highlighter-for-warcraft-3.246081/) which is integrated into the World Editor's trigger editor.
It might be the most useful and advanced JASS IDE so far.
However, it does not provide live syntax checking nor does it allow refactoring code or check the context when auto completing code.
It simply suggests all natives and functions from Warcraft's standard script files.
Hence, there is much room from improvement.
On the one hand it might be useful to have the tool directly integrated into the World Editor allowing you to create triggers and using generated variables.
However, a standalone tool might also be useful for non-Windows users or performance wise or even people who do not own Warcraft III.

There are some other tools like [vscode for jass](https://www.hiveworkshop.com/threads/vscode-for-jass.333627/) or [vjass support extension for Visual Studio Code](https://www.hiveworkshop.com/threads/vjass-support-extension-for-visual-studio-code.303564/) integrating vJass support into VS code.
This might be useful enough for people using this IDE.
However, this project can optimize the GUI for vJass only, removing unnecessary features from common IDEs.

Tools like [JAST 1.1.1](https://www.hiveworkshop.com/threads/jast-1-1-1.325057/) are also standalone IDEs but lack of good performance and basic GUI features.
Besides, they integrate parsers like [pjass](https://www.hiveworkshop.com/threads/pjass-updates.258738/) instead of checking the syntax live.

This IDE tries to check syntax while writing the code and give some meaningful context-related suggestions in the auto completion.

## Architecture

The IDE is written in C++ and requires only the [Qt framework](https://www.qt.io/).
It can be used on all platforms which are supported by the Qt framework.
Other dependencies are avoided to keep it as simple as possible.
I highly recommend the [Qt Creator](https://en.wikipedia.org/wiki/Qt_Creator) for developing this tool.

## Future Work

These are some possible upcoming features for this IDE:

* Code formatting: Different code styles.
* Refactoring: Renaming symbols etc.
* Static analysis: Leak detection, accessing uninitialized variables.
* Integration of existing parsers and compilers: [pjass](https://www.hiveworkshop.com/threads/pjass-updates.258738/) and JassHelper.
* Map integration: Load existing maps to suggest symbols from it like triggers, variables or even rawcodes.
* Export/import formats: trigger data, LUA, Wurst.
* Optimization: Optimizing the map script.
* Obfuscationg: Obfuscating your code with not readable symbols.

## JASS and vJass

* [JASS Manual](http://jass.sourceforge.net/doc/)
* [vJass Documentation](https://wc3modding.info/pages/vjass-documentation/)
