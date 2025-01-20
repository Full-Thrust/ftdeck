The ftdeck program can be used to create modified cards. The file ft_systems.txt contains the names of the GIF files on the systems to use. Each line has a corresponding GIF file with the same name. Running the program will run it including the systems in the file, so to add new systems add a new GIF file and name to the ft_systems.txt file. You will need to recompile the program and change the NUM_FT_SYSTEMS constant to match the number of systems to load. Alternatively replace the entries in the file with your own the program will still run if there are 28 entries.

```
ftdeck -i image_dir -f deck_filename -s system_filename

-i image source directory for icons

-f filename for the deck images

-s system definition filename

-v verbose shows processing parameters

```