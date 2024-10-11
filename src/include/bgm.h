

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

        char *data = malloc(tracks[i].padded_size);
        fread(data, 1, tracks[i].padded_size, bd_file);
        if(!raw) write_vagi_header(out, tracks[i].sample_rate, tracks[i].padded_size, 1000,2);
        fwrite(data, 1, tracks[i].padded_size, out);
        fclose(out);
        free(data);
    }

    fclose(bd_file);
    free(tracks);

    printf("Done\n");
}

void import_music(const char *music_info, const char *music_bd, const char *input_folder)
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
    printf("Importing music into %s\n", music_bd);

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
            char *unchanged = malloc(tracks[i].padded_size);
            fread(unchanged, 1, tracks[i].padded_size, bd_file);
            fwrite(unchanged, 1, tracks[i].padded_size, new_bd_file);
            free(unchanged);
            continue;
        }else{
            fseek(bd_file, tracks[i].padded_size, SEEK_CUR);
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

        fseek(in, 0, SEEK_END);
        uint32_t bgm_size = ftell(in);
        fseek(in, 0, SEEK_SET);

        char header_check[16];
        fread(header_check, 1, 16, in);
        if(memcmp(header_check, BD, 16) != 0)
        {
            //add header data, MFAudio generates raw files without it
            fwrite(BD, 1, 16, new_bd_file);
            bgm_size += 16;
        }
        fseek(in, 0, SEEK_SET);

        uint32_t block_size = 0x1000;

        uint32_t padded_size = (1 + (bgm_size / block_size)) * block_size; 

        char *data = calloc(padded_size, 1);
        fread(data, 1, bgm_size, in);
        fwrite(data, 1, padded_size, new_bd_file);
        fseek(info_file, 0x10 + i * sizeof(music_track), SEEK_SET);
        fseek(info_file, 8, SEEK_CUR);
        fwrite(&bgm_size, 4, 1, info_file);
        fwrite(&padded_size, 4, 1, info_file);
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