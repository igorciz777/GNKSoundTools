#include <dirent.h>
#include "include/gnk_sound.h"
#include "include/bgm.h"
#include "include/se.h"

void usage(const char *progname)
{
    printf("\nUsage: %s [mode]\n\n", progname);
    printf("  Modes:\n\n");
    printf("  BGM:\n");
    printf("    -ml <bgm info file>                                : Lists all found music tracks\n");
    printf("    -mv <bgm info file> <bgm .bd file> <output folder> : Extracts all music into VAGi files (for playback)\n");
    printf("    -mr <bgm info file> <bgm .bd file> <output folder> : Extracts all music into RAW files\n");
    printf("    -mi <bgm info file> <bgm .bd file> <input folder>  : Imports RAW files into the music .bd file\n");
    printf("\n");
    printf("  Sound Effects:\n");
    printf("    -sl <hd info file>                                 : Lists all sound effects found in the hd file\n");
    printf("    -sv <hd info file> <bd data file> <output folder>  : Extracts all sounds from the hd/bd soundbank into VAGp format\n");
    printf("    -si <hd info file> <bd data file> <input folder>   : Imports VAGp files into the hd/bd soundbank\n");
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
                    extract_music(argv[2], argv[3], argv[4],0);
                    break;
                case 'r':
                    if(argc < 5){usage(argv[0]);return 1;}
                    extract_music(argv[2], argv[3], argv[4],1);
                    break;
                case 'i':
                    if(argc < 5){usage(argv[0]);return 1;}
                    import_music(argv[2], argv[3], argv[4]);
                    break;
                case 'l':
                    if(argc < 3){usage(argv[0]);return 1;}
                    list_music(argv[2]);
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
                    list_sounds(argv[2]);
                    break;
                default:
                    usage(argv[0]);
                    return 1;
            }
            break;
        default:
            usage(argv[0]);
            return 1;
    }
    return 0;
}
