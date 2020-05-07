# FakeWar3

Used to bypass around *GameRanger* limitation of not being able to launch *Warcraft III* in windowed mode.

To run *Warcraft III* in windowed mode you must pass the `-window` argument to the game, but *GameRanger* does not have an option for that. The solution was to take a program that requires no arguments, but launches the game in windowed mode.

This program attempts to read the command line to execute from a configuration file. If file is missing or there is an error reading it, then it will attempt to use default *Warcraft III* location: `C:\Games\Warcraft III 1.26\<warcraft_3_game_executable>`.

## Installation

In order to make it work with GameRanger one has to navigate to the *Warcraft III* root directory and change the name of **``war3.exe``** to some other name (***NOTE: do NOT overwrite this file***), by deafult it is assumed that the file was renamed to **``realWar3.exe``**. Then insert the fake **``war3.exe``** and configuration file named **``fakeWar3.cfg``** into the root directory.

## Configuration

The configuration file **fakeWar3.cfg** must contain one and only one line specifying the path to the real *Warcraft III* executable and any parameters you want (*do **not** include* ``-window`` *parameter, as it is added automatically*). **Example:**

```
"C:\Games\Warcraft III 1.26\realWar3.exe"
```

**Please remember** to wrap the path with double quotes **``"``**, as shown in the example.

## Notes

- This software was only tested for *Warcraft III: Frozen Throne*.
- This software does not support multiple displays (the resulting window should occupy the main display).
