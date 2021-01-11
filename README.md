# opencvHandtrack
This program allows the detection of a hand with a gun-like hand gesture. It is intended to be used with a prototype FPS game which I am wokring on and allow the player to control the character's aim with their hand.
## Prequisites
### Install OpenCV on your platform
- For Windows installation, you can follow this tutorial [here](https://docs.opencv.org/3.4/d3/d52/tutorial_windows_install.html) to install the prebuilt libraries. In addition, to build the program with Visual Studio, you need to set the OpenCV Environment Variable and add it to the linker by following [this tutorial](https://docs.opencv.org/3.4/dd/d6e/tutorial_windows_visual_studio_opencv.html).
- For Linux installation, the easiest way is to install through your distro's package manager. In my testing, I used Arch Linux, so you can simply install with this command.
```
#Arch Linux
sudo pacman -S opencv
```
In addition, you may also choose to build from source with [this tutorial](https://docs.opencv.org/3.4/d7/d9f/tutorial_linux_install.html).

## Usage
By default, the program chooses camera 0. You can change this by inputing the camera number as an argument.
The program sends the data about validity of hand, as well as x and y of three points (tips of thumb and index, perlicue) as strings seperated by spaces to port on localhost via UDP. This can then be read by other software to do processing.
