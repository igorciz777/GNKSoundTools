# GNK Sound Tools

Sound modding tools for PS2 Genki games

## [How to use](https://github.com/igorciz777/GNKSoundTools/wiki/How-to-use)

***
Run
```shell
gnk_sound [mode]
  Modes:
    BGM:
      -ml <bgm info file>                                : Lists all found music tracks
      -mv <bgm info file> <bgm .bd file> <output folder> : Extracts all music into .VAG files (for playback)
      -mr <bgm info file> <bgm .bd file> <output folder> : Extracts all music into RAW files
      -mi <bgm info file> <bgm .bd file> <input folder>  : Injects RAW files into the music .bd file

    Sound Effects:
      -sl <hd info file>                                 : Lists all found sound effects
      -sv <hd info file> <bd data file> <output folder>  : Extracts all sound effects into .VAG files (for playback)
      -sr <hd info file> <bd data file> <output folder>  : Extracts all sound effects into RAW files
      -si <hd info file> <bd data file> <input folder>   : Injects RAW files into the sound effects .bd file
```
Build

```shell
gcc -Wall src/gnk_sound.c -o gnk_sound
```
## Tested games with working music replacement
|                     **Name**                    | **Serial** | **Frequency**       | **Interleave**            | **BGM Info file**         | **BGM Data file**         |
|:-----------------------------------------------:|:----------:|:-------------------:|:-------------------------:|:-------------------------:|:-------------------------:|
| Tokyo Xtreme Racer 3                            | SLUS 20831 |     44100           |   1000 Bytes              |   `00000000.bin`          |  `00000001.bd`            |
| Racing Battle: C1 Grand Prix                    | SLPM 65897 |     44100           |   1000 Bytes              |  `00005794.bin`           |  `00005795.bd`            |
| Tokyo Xtreme Racer DRIFT 2                      | SLUS 21394 |     44100/48000     |   1000 Bytes/2000 Bytes   |  `00003026.bin`           |  `00003027.bd`            |

## Current issues
- Music not looping
- Sometimes imported music refuses to play
- Sound effects extraction and insertion not implemented