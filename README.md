# Peregrine 
Detect and classify bird species in realtime on a video feed

# Building

## HALO
You'll need the Tensyr HALO SDK. Make sure it's installed
in /opt/tensyr (if you have a tarball release, just untar it there)

We use CMake for our build. To build Peregrine, first create and cd to a
build directory and then run:

```bash
cmake path/to/sources
make
make install
```
This will create a new install directory inside of your build directory
with all of Peregrine's runnable artificats.

## Depedencies
Peregrine depends on opencv and ffmpeg. You can either install these or
use the docker image (runtime.df). If using the docker image, you can run
`enter-container.sh` to run make inside the image

# Running

Once you have an install directory, just run the `run` script
inside of the install directory (make sure to cd to the install directory first).
This will take in a video (with the -f flag) and output to a file called
`video_out.mp4`

## Docker
HALO only supports Ubuntu 16.04, but you can use the provided dockerfile to
run Peregrine on any linux system.
Build the dockerfile (runtime.df) and point the env to a folder with the HALO
debian. I.e.:
```
  docker build -f runtime.df /path/to/halo/deb
```
The run script supports running Peregrine inside a docker container.
You'll need to specify an image via the `-i` flag.

# Dataset

We use the NABirds V1 dataset (DOI: 10.1109/CVPR.2015.7298658) for
species classification training, generously available through the 
[Cornell Lab of O](http://dl.allaboutbirds.org/nabirds).
