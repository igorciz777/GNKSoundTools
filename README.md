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
## Tested games with working music replacement
|                     **Name**                    | **Serial** | **Frequency**       | **Interleave**            |
|:-----------------------------------------------:|:----------:|:-------------------:|:-------------------------:|
| Tokyo Xtreme Racer 3                            | SLUS 20831 |     44100           |   1000 Bytes              |
| Racing Battle: C1 Grand Prix                    | SLPM 65897 |     44100           |   1000 Bytes              |
| Tokyo Xtreme Racer DRIFT 2                      | SLUS 21394 |     44100/48000     |   1000 Bytes/2000 Bytes   |
