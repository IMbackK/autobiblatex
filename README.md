# autobiblatex

autobiblatex is a commandline application that allows one to quickly generate a biblatex database given only a folder of pdf files.
autobiblatex uses crossref where possible and a heuristic based on the metadata where it is not.

autobiblatex's development and target platform is UNIX, but it should compile on nigh any platform

## Compile/Install

### Requirements

* git
* c++20 capable compiler (GCC, CLANG)
* [cmake](https://cmake.org/) 3.20 or later
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) (or change/hardcode the paths in CMakeLists.txt)
* [libpoppler](https://poppler.freedesktop.org)
* [curl](https://curl.se/)

### Procedure (UNIX)

In a console do:

* git clone https://git-ce.rwth-aachen.de/carl_philipp.klemm/eisgenerator.git
* cd eisgenerator
* mkdir build
* cd build
* cmake ..
* make -j8
* sudo make install

## Usage of autobiblatex

### Generate spectra

autobiblatex [FOLDER] [FILENAME]

with FOLDER being a folder containing *.pdf files and FILENAME being a output file name where to place the biblatex database

## Licence

autobiblatex is Licenced to you under the GPLv3. See gpl-3.0.txt for more information
