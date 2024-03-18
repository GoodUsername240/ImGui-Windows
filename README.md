# ImGui Windows

Allows you to have a desktop window drawn with ImGui.

![Image](https://github.com/GoodUsername240/ImGui-Windows/blob/main/Screenshot%202024-03-10%20225315.png?raw=true)

Example can be found in `Example.cpp`


## Features

- Resizing
- Dragging/moving
- Minimizing
- ImGui controlled title bar

## Limitations

- Can only have 1 window per process
- Cannot have rounded windows

## Building

```
git clone https://github.com/GoodUsername240/ImGui-Windows.git
cd ImGui-Windows
git submodule update --init
```
Build `ImGui Windows.sln` in `Release x64` mode.

## TODO

- Multiple windows
- Support for different backends
test