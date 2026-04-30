| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

# _Juca Library Test_

This code allows the user to test the quaternion obtained from the MPU-6050 using a web based visualization through a page accessed through the board IP.


## How to use example
Configure the Wi-Fi on wifi_manager.c, using idf.py menuconfig go to ``Component config/HTTP Server`` and enable WebSocket server support, build and flash the code on a Juca robot.

## Folder contents

The project contains source files in C language for the MPU-6050, webserver, Wi-Fi, Wheels, Distance sensors and tasks.

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 
