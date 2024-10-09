# GNK Sound Tools

Sound modding tools for PS2 Genki games

[How to use](https://github.com/igorciz777/GNKSoundTools/wiki)

***
Run
```shell
gnk_sound [mode]
  Modes:
    -l  <music info file>                                  : Lists all found music tracks
    -ev <music info file> <music .bd file> <output folder> : Extracts all music into .VAG files (for playback)
    -er <music info file> <music .bd file> <output folder> : Extracts all music into RAW files\n
    -i  <music info file> <music .bd file> <input folder>  : Injects RAW files into the music .bd file
```
Build

```shell
gcc -Wall src/gnk_sound.c -o gnk_sound
```
