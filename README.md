# MPVideoCube

- [MPVideoCube](#mpvideocube)
  - [How it works](#how-it-works)
  - [Dependencies](#dependencies)
    - [Optional mpv-build](#optional-mpv-build)
  - [Installation / Compile](#installation--compile)
    - [If compile fails but you want to know how it looks like](#if-compile-fails-but-you-want-to-know-how-it-looks-like)
  - [References](#references)
  - [License and Author](#license-and-author)

This is an example for how to use libmpv (mpv) for creating video textures on OpenGL objects. There are a lot of examples but AFAIK none of them shows how to combine "basic" OpenGL objects and video textures in a modern way.

## How it works
* GLFW is used for the the window creation, event handling and  OpenGL context creation.
* libmpv is used to create a render context - this is actually the "Video to Texture" transfer/method. HW-accel is enabled, if it works depends on system and hardware.
  
* There are two OpenGL "objects" in this demonstration:
  * A CUBE where the Video is used as a texture (video_textureColorbuffer). The Cube is rendered to the "Screen Framebuffer" (RED BACKGROUND). The cube is rendered twice.
  * A QUAD where the "Screen Framebuffer" (screen_textureColorbuffer) is used as the texture.
  * The QUAD is then rendered into the main frambebuffer (ID == 0) (BLUE BACKGROUND)
  
* Each of the objects does have its own shader:
  * The CUBE shader (fragment) does not modify the texture
  * The QUAD shader (fragment) addes a vignette to the whole scene.

* Each of the objects is animated (tranformed) over time:
  * The CUBE is spinning and moving
  * The QUAD is scaled 

## Dependencies
* Main dependencies: 
  * libmpv-dev  (at least version 0.30 and with opengl enabled)
  * libglfw3-dev
  * youtube-dl

### Optional mpv-build 
Please do not do this unless you know what you are doing. 

I recommend compile mpv from scratch using https://github.com/mpv-player/mpv-build.git but this is not for the faint hearted. I included my compile settings in the assets folder in case someone needs them. Please note that mpv does not "explicitly" (stop and show) warn about missing libs that are not absolutly required using the configure process. Use the "scripts/ffmpeg-config" and "scripts/mpv-config" script in the mpv-build to rerun and check. Use the "./install" to install and make sure you run a "ldconfig" after. Removing the system installation of mpv and libmpv is recommended (```apt-get remove mpv libmpv-dev```)

## Installation / Compile
* make sure you do fullfill the requirements: 
  * ``` sudo apt-get install libmpv-dev libglfw3-dev build-essential ```
* To compile the demonstration just use:
  * ``` make  ```
* To run it, use the demonstration commands or your own video file:
  * ``` ./rundemo-video-2160p-60p.sh ``` (a 4K video)
  * ``` ./rundemo-video-1080p-60p.sh ``` (a FullHD video)
  * ``` ./rundemo-youtube.sh ```  (This only works if youtube-dl is installed and working!)
  * ``` ./mpvideocube myvideofile.mp4 ``` (try with your own video files)

If the spinning cubes stay black but you here the audio then the rendering of the video is broken. This can be due to the codec of the video and/or the limitations of the HW-accel. Check the output in the console.

### If compile fails but you want to know how it looks like

If you want to know how the result looks like but you have problems to compile the program then take a look at the **assets/how-it-looks-like.mkv** video. This is is a screen capture of the program running.

## References
* The main OpenGL concept is based on the examples from https://www.learnopengl.com
* The libmpv usage is roughly based on the mpv-examples https://github.com/mpv-player/mpv-examples/tree/master/libmpv
* The video clips test-1080p-60fps.m4v and test-2160p-60fps.mp4 are (c) copyright 2008, Blender Foundation / www.bigbuckbunny.org
   
## License and Author
MIT License - Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com