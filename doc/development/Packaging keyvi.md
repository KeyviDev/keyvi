## Creating Debian packages

### dictionary cmdline tools

Go into the dictionary folder and first build, then package:

    cd dictionary
    scons mode=release
    scons debian

### pykeyvi

To build a debian package of pykeyvi you need stdeb:
 
    sudo apt-get install python-stdeb

To build a package edit setup.py and set dictionary_sources so that the source files can be found (todo: find a better solution for this).

Go into pykeyvi folder and build:

    cd pykeyvi
    python setup.py --command-packages=stdeb.command bdist_deb