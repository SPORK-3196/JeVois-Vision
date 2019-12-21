# JeVois-Vision
Team SPORK's [JeVois](http://jevois.org/) vision modules for various tasks.

## Modules
Each subdirectory in the repository is its own vision module, and may include
its own README with additional details. Currently, the modules are:
 - **retro-tape-tracker** - detects the midpoint of retroreflective tape and
 sends the coordinates over USB.

## Deploying
To deploy all modules to the JeVois camera:
 1. Open JeVois Inventor, and look under the "System" tab. Click Enable on the
    *"Export microSD card inside JeVois to host computer"* option.
 2. Once the JeVois microSD appears in your filesystem, copy over `SPORK3196` to
    the "modules/" directory, replacing any already existing files.
 3. Copy `videomappings.cfg` to the "config/" directory. *NOTE*: This will
    overwrite any other videomappings.
 3. Eject the microSD from the filesystem, wait for the JeVois to reboot.
    Reconnect with JeVois Inventor
