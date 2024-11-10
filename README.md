# GNK Sound Tools

Sound modding tools for PS2 Genki games

## [How to use](https://github.com/igorciz777/GNKSoundTools/wiki/How-to-use)

***
Run
```shell
gnk_sound [mode]
  Modes:
    BGM:
      -ml <bgm info file> <bgm .bd file> <optional: out.txt>  : Lists all music file info
      -mv <bgm info file> <bgm .bd file> <output folder>      : Extracts all music into ADS(PSF2) format
      -mi <bgm info file> <bgm .bd file> <input folder>       : Imports ADS/VAGi files into the music .bd file

    Sound Effects:
      -sl <hd info file> <optional: out.txt>                  : Lists all sound effects info
      -sv <hd info file> <bd data file> <output folder>       : Extracts all sounds from the hd/bd soundbank into VAGp format
      -si <hd info file> <bd data file> <input folder>        : Imports VAGp files into the hd/bd soundbank
```
Build

```shell
gcc -Wall src/gnk_sound.c -o gnk_sound
```
## Tested games with working music replacement
|                     **Name**                    | **Serial** | **Frequency**       | **Interleave**            | **BGM Info file**         | **BGM Data file**         |
|:-----------------------------------------------:|:----------:|:-------------------:|:-------------------------:|:-------------------------:|:-------------------------:|
| Tokyo Xtreme Racer 3                            | SLUS 20831 |     44100           |   0x1000 Bytes            |  `00000000.bin`           |  `00000001.bd`            |
| Racing Battle: C1 Grand Prix                    | SLPM 65897 |     44100           |   0x1000 Bytes            |  `00005794.bin`           |  `00005795.bd`            |
| Tokyo Xtreme Racer DRIFT 2                      | SLUS 21394 |     44100/48000     |   0x2000 Bytes            |  `00003026.bin`           |  `00003027.bd`            |

## Current issues
- Music not looping
- Sometimes imported music refuses to play
- Sound effects longer than original will break music playback