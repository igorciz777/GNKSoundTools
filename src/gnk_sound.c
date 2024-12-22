#include <dirent.h>
#include "include/gnk_sound.h"
#include "include/bgm.h"
#include "include/se.h"

void usage(const char *progname)
{
    printf("\nUsage: %s [mode]\n\n", progname);
    printf("  Modes:\n\n");
    printf("  BGM:\n");
    printf("    -ml <bgm info file> <bgm .bd file> <optional: out.txt>  : Lists all music file info\n");
    printf("    -mv <bgm info file> <bgm .bd file> <output folder>      : Extracts all music into ADS (SPU2) format\n");
    printf("    -mi <bgm info file> <bgm .bd file> <input folder>       : Imports ADS/VAGi files into the music .bd file\n");
    printf("    -lv <MUSIC.SMH> <MUSIC.SMC> <output folder>             : Extracts all music from older games (pre-TXR3)\n");
    printf("\n");
    printf("  Sound Effects:\n");
    printf("    -sl <hd info file> <optional: out.txt>                  : Lists all sound effects info\n");
    printf("    -sv <hd info file> <bd data file> <output folder>       : Extracts all sounds from the hd/bd soundbank into VAGp format\n");
    printf("    -si <hd info file> <bd data file> <input folder>        : Imports VAGp files into the hd/bd soundbank\n");
    printf("\n");

}

int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        usage(argv[0]);
        return 1;
    }
    switch(argv[1][1])
    {
        case 'm':
            switch(argv[1][2])
            {
                case 'v':
                    if(argc < 5){usage(argv[0]);return 1;}
                    extract_music(argv[2], argv[3], argv[4]);
                    break;
                case 'i':
                    if(argc < 5){usage(argv[0]);return 1;}
                    import_music(argv[2], argv[3], argv[4]);
                    break;
                case 'l':
                    if(argc < 4){usage(argv[0]);return 1;}
                    if(argc == 5)
                        list_music(argv[2], argv[3], argv[4]);
                    else
                        list_music(argv[2], argv[3], NULL);
                    break;
                default:
                    usage(argv[0]);
                    return 1;
            }
            break;
        case 's':
            switch(argv[1][2])
            {
                case 'v':
                    if(argc < 5){usage(argv[0]);return 1;}
                    extract_sounds(argv[2], argv[3], argv[4]);
                    break;
                case 'i':
                    if(argc < 5){usage(argv[0]);return 1;}
                    import_sounds(argv[2], argv[3], argv[4]);
                    break;
                case 'l':
                    if(argc < 3){usage(argv[0]);return 1;}
                    if(argc == 4)
                        list_sounds(argv[2], argv[3]);
                    else
                        list_sounds(argv[2], NULL);
                    break;
                default:
                    usage(argv[0]);
                    return 1;
            }
            break;
        case 'l':
            if(argc < 5){usage(argv[0]);return 1;}
            if(strcmp(argv[1], "-lv") == 0)
                extract_music_old(argv[2], argv[3], argv[4]);
            break;
        default:
            usage(argv[0]);
            return 1;
    }
    return 0;
}
