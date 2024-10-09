#include <dirent.h>
#include "include/gnk_sound.h"

void write_vag_header(FILE *out, uint32_t sample_rate, uint32_t size, uint32_t interleave, uint8_t channels)
{
    char *padding;
    padding = calloc(0x40, 1);
    fwrite(VAG_HEADER, 1, sizeof(VAG_HEADER), out);
    fwrite(&interleave, 4, 1, out);
    size = swap_uint32(size);
    sample_rate = swap_uint32(sample_rate);
    fwrite(&size, 4, 1, out);
    fwrite(&sample_rate, 4, 1, out);
    fwrite(padding, 1, 0x0a, out);
    fwrite(&channels, 1, 1, out);
    fwrite(padding, 1, 0x11, out);
    free(padding);
}

void extract_music(const char *music_info, const char *music_bd, const char *output_folder, int raw)
{
    music_track *tracks;
    uint32_t num_tracks; // @ 0x08 in music_info
    FILE *info_file = fopen(music_info, "rb");
    if(!info_file)
    {
        printf("Error: Could not open %s\n", music_info);
        return;
    }

#ifdef _WIN32
    mkdir(output_folder);
#else
    mkdir(output_folder, 0700);
#endif

    printf("Extracting music from %s\n", music_bd);

    fseek(info_file, 0x08, SEEK_SET);
    fread(&num_tracks, 4, 1, info_file);
    tracks = malloc(num_tracks * sizeof(music_track));
    fseek(info_file, 0x10, SEEK_SET);
    fread(tracks, sizeof(music_track), num_tracks, info_file);
    fclose(info_file);
    printf("Found %d tracks\n", num_tracks);

    FILE *bd_file = fopen(music_bd, "rb");
    if(!bd_file)
    {
        printf("Error: Could not open %s\n", music_bd);
        return;
    }

    fseek(bd_file, 0x0, SEEK_SET);

    for(int i = 0; i < num_tracks; i++)
    {
        char *filename = malloc(strlen(output_folder) + strlen(tracks[i].name) + 12);
        sprintf(filename, "%s/%04d_%s.%s",output_folder, i, tracks[i].name, raw ? "RAW" : "VAG");
        FILE *out = fopen(filename, "wb");
        if(!out)
        {
            printf("Error: Could not open %s for writing\n", tracks[i].name);
            continue;
        }

        char *data = malloc(tracks[i].size);
        fread(data, 1, tracks[i].size, bd_file);
        if(!raw) write_vag_header(out, tracks[i].sample_rate, tracks[i].size, 1000,2);
        fwrite(data, 1, tracks[i].size, out);
        fclose(out);
        free(data);
    }

    fclose(bd_file);
    free(tracks);

    printf("Done\n");
}

void inject_music(const char *music_info, const char *music_bd, const char *input_folder)
{
    music_track *tracks;
    uint32_t num_tracks; // @ 0x08 in music_info
    int file_index;
    int *to_import;
    DIR *dir;
    struct dirent *entry;

    FILE *info_file = fopen(music_info, "r+b");
    if(!info_file)
    {
        printf("Error: Could not open %s\n", music_info);
        return;
    }
    printf("Injecting music into %s\n", music_bd);

    fseek(info_file, 0x08, SEEK_SET);
    fread(&num_tracks, 4, 1, info_file);
    tracks = calloc(num_tracks, sizeof(music_track));
    fseek(info_file, 0x10, SEEK_SET);
    fread(tracks, sizeof(music_track), num_tracks, info_file);
    
    FILE *bd_file = fopen(music_bd, "rb+");
    if(!bd_file)
    {
        printf("Error: Could not open %s\n", music_bd);
        return;
    }

    fseek(bd_file, 0x0, SEEK_SET);

    to_import = calloc(num_tracks, sizeof(int));

    dir = opendir(input_folder);
    if (dir == NULL)
    {
        printf("Error: Could not open %s\n", input_folder);
        return;
    }

    FILE *new_bd_file = fopen("new_music.bd", "wb");
    if(!new_bd_file)
    {
        printf("Error: Could not create temp music file\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL){
        if (strstr(entry->d_name, ".") == NULL)
        {
            continue;
        }
        if (entry == NULL || entry->d_name[0] == '.')
        {
            continue;
        }

        char filename_index[300];
        strncpy(filename_index, entry->d_name, 4);

        file_index = atoi(filename_index);

        if (file_index < 0 || file_index >= num_tracks)
        {
            printf("Invalid file index for file %s\n", entry->d_name);
            continue;
        }

        to_import[file_index] = 1;
    }

    for(int i = 0; i < num_tracks; i++)
    {
        if (to_import[i] == 0)
        {
            char *unchanged = malloc(tracks[i].size);
            fread(unchanged, 1, tracks[i].size, bd_file);
            fwrite(unchanged, 1, tracks[i].size, new_bd_file);
            free(unchanged);
            continue;
        }else{
            fseek(bd_file, tracks[i].size, SEEK_CUR);
        }
        printf("Importing %s\n", tracks[i].name);

        char *filename = malloc(strlen(input_folder) + strlen(tracks[i].name) + 12);
        sprintf(filename, "%s/%04d_%s.RAW",input_folder, i, tracks[i].name);
        FILE *in = fopen(filename, "rb");
        if(!in)
        {
            printf("Error: Could not open %s for reading\n", filename);
            continue;
        }

        char header_check[16];
        fread(header_check, 1, 16, in);
        if(memcmp(header_check, BD, 16) != 0)
        {
            //add header data, MFAudio generates raw files without it
            fwrite(BD, 1, 16, new_bd_file);
        }

        fseek(in, 0, SEEK_END);
        int size = ftell(in);
        fseek(in, 0, SEEK_SET);

        char *data = malloc(size);
        fread(data, 1, size, in);
        fwrite(data, 1, size, new_bd_file);
        fseek(info_file, 0x10 + i * sizeof(music_track), SEEK_SET);
        fseek(info_file, 12, SEEK_CUR);
        fwrite(&size, 4, 1, info_file);
        free(data);
        fclose(in);
    }

    fclose(bd_file);
    fclose(new_bd_file);
    fclose(info_file);
    free(tracks);
    free(to_import);

    remove(music_bd);
    rename("new_music.bd", music_bd);

    closedir(dir);

    printf("Done\n");
}

void list_music(const char *music_info)
{
    music_track *tracks;
    uint32_t num_tracks; // @ 0x08 in music_info
    FILE *info_file = fopen(music_info, "rb");
    if(!info_file)
    {
        printf("Error: Could not open %s\n", music_info);
        return;
    }

    fseek(info_file, 0x08, SEEK_SET);
    fread(&num_tracks, 4, 1, info_file);
    tracks = malloc(num_tracks * sizeof(music_track));
    fseek(info_file, 0x10, SEEK_SET);
    fread(tracks, sizeof(music_track), num_tracks, info_file);
    fclose(info_file);
    printf("Found %d tracks\n", num_tracks);


    for(int i = 0; i < num_tracks; i++)
    {
        printf("%04d: %s\n", i, tracks[i].name);
    }

    free(tracks);

    printf("Done\n");
}

void usage(const char *progname)
{
    printf("\nUsage: %s [mode] [log]\n\n", progname);
    printf("  Modes:\n");
    printf("    -l  <music info file>                                  : Lists all found music tracks\n");
    printf("    -ev <music info file> <music .bd file> <output folder> : Extracts all music into .VAG files (for playback)\n");
    printf("    -er <music info file> <music .bd file> <output folder> : Extracts all music into RAW files\n");
    printf("    -i  <music info file> <music .bd file> <input folder>  : Injects RAW files into the music .bd file\n");
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
        case 'e':
            if(argc < 5)
            {
                usage(argv[0]);
                return 1;
            }
            switch(argv[1][2])
            {
                case 'v':
                    extract_music(argv[2], argv[3], argv[4],0);
                    break;
                case 'r':
                    extract_music(argv[2], argv[3], argv[4],1);
                    break;
                default:
                    usage(argv[0]);
                    return 1;
            }
            break;
        case 'i':
            if(argc < 5)
            {
                usage(argv[0]);
                return 1;
            }
            inject_music(argv[2], argv[3], argv[4]);
            break;
        case 'l':
            if(argc < 3)
            {
                usage(argv[0]);
                return 1;
            }
            list_music(argv[2]);
            break;
        default:
            usage(argv[0]);
            return 1;
    }
    return 0;
}
