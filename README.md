# GNK Sound Tools

Sound modding tools for PS2 Genki games

[How to use](https://github.com/igorciz777/GNKSoundTools/wiki/How-to-use)

***
Run
```shell
gnk_sound [mode]
  Modes:
    -l  <music info file>                                  : Lists all found music tracks
    -ev <music info file> <music .bd file> <output folder> : Extracts all music into Sony VAG files
    -er <music info file> <music .bd file> <output folder> : Extracts all music into RAW files
    -i  <music info file> <music .bd file> <input folder>  : Imports RAW files into the music .bd file
```
Build

```shell
gcc -Wall src/gnk_sound.c -o gnk_sound
```
