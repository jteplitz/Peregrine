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

# Running

Once you have an install directory, just run the `run` script
inside of the install directory (make sure to cd to the install directory first).

## Docker
HALO only supports Ubuntu 16.04, but you can use the provided dockerfile to
run Peregrine on any linux system.
Build the dockerfile (runtime.df) within /opt/tensyr a
The run script supports passing a docker image to run Peregrine inside of
via the `-i` flag

# Dataset

We use the NABirds V1 dataset (DOI: 10.1109/CVPR.2015.7298658) for
species classification training, generously available through the 
[Cornell Lab of O](http://dl.allaboutbirds.org/nabirds).
