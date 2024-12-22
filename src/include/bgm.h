uint32_t get_interleave(char* adpcm_data, uint32_t adpcm_data_size){
    uint32_t interleave = 0;
    for(int i = 0x10; i < adpcm_data_size; i+=0x10){
        if(memcmp(adpcm_data + i, BD, 16) == 0){
            interleave = i;
            break;
        }
    }
    if(interleave == 0 || interleave > 0x18000){
        interleave = 0x1000;
    }
    return interleave;
}

void extract_music(const char *music_info, const char *music_bd, const char *output_folder)
{
    music_track *tracks;
    uint32_t num_tracks; // @ 0x08 in music_info
    uint32_t interleave;
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
        sprintf(filename, "%s/%04d_%s.%s",output_folder, i, tracks[i].name, "ADS");
        FILE *out = fopen(filename, "wb");
        if(!out)
        {
            printf("Error: Could not open %s for writing\n", tracks[i].name);
            continue;
        }

        char *data = malloc(tracks[i].padded_size);
        fread(data, 1, tracks[i].padded_size, bd_file);
        interleave = get_interleave(data, tracks[i].padded_size);
        write_ads_header(out, tracks[i].sample_rate, tracks[i].padded_size, interleave, 2);
        fwrite(data, 1, tracks[i].padded_size, out);
        fclose(out);
        free(data);
    }

    fclose(bd_file);
    free(tracks);

    printf("Done\n");
}

void extract_music_old(const char *music_smh, const char *music_smc, const char *output_folder)
{
    smh_track *tracks;
    uint32_t num_tracks; // @ 0x08 in music_info
    uint32_t interleave;
    FILE *info_file = fopen(music_smh, "rb");
    if(!info_file)
    {
        printf("Error: Could not open %s\n", music_smh);
        return;
    }

#ifdef _WIN32
    mkdir(output_folder);
#else
    mkdir(output_folder, 0700);
#endif

    printf("Extracting music from %s\n", music_smc);
    fread(&num_tracks, 4, 1, info_file);
    fread(&interleave, 4, 1, info_file);
    tracks = malloc(num_tracks * sizeof(smh_track));

    fseek(info_file, 0x10, SEEK_SET);

    for(int i = 0; i < num_tracks; i++)
    {
        fread(&tracks[i], sizeof(smh_track), 1, info_file);
    }
    fclose(info_file);

    FILE *bd_file = fopen(music_smc, "rb");
    if(!bd_file)
    {
        printf("Error: Could not open %s\n", music_smc);
        return;
    }

    fseek(bd_file, 0x0, SEEK_SET);

    for(int i = 0; i < num_tracks; i++)
    {
        char *filename = malloc(strlen(output_folder) + 12);
        sprintf(filename, "%s/%04d.%s",output_folder, i, "ADS");
        FILE *out = fopen(filename, "wb");
        if(!out)
        {
            printf("Error: Could not open %s for writing\n", filename);
            continue;
        }

        char *data = malloc(tracks[i].sample_size);
        fread(data, 1, tracks[i].sample_size, bd_file);
        write_ads_header(out, tracks[i].sample_rate, tracks[i].sample_size, interleave, 2);
        fwrite(data, 1, tracks[i].sample_size, out);
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

        int is_vagi = 0;

        char *filename = malloc(strlen(input_folder) + strlen(tracks[i].name) + 12);
        sprintf(filename, "%s/%04d_%s.ADS",input_folder, i, tracks[i].name);
        FILE *in = fopen(filename, "rb");
        if(!in)
        {
            sprintf(filename, "%s/%04d_%s.VAG",input_folder, i, tracks[i].name);
            in = fopen(filename, "rb");
            if(!in)
            {
                printf("Error: Could not open %s for reading\n", tracks[i].name);
                continue;
            }else{
                is_vagi = 1;
            }
        }

        uint32_t file_size;
        fseek(in, 0, SEEK_END);
        file_size = ftell(in);
        fseek(in, 0, SEEK_SET);

        ADS ads;
        VAGp vagi;

        uint32_t sample_size, sample_rate, interleave, header_size;//, bitrate, duration, bit_duration, channels;

        if(is_vagi)
        {
            fread(&vagi, sizeof(VAGp), 1, in);
            sample_size = vagi.size;
            sample_rate = swap_uint32(vagi.sample_rate);
            interleave = vagi.interleave;
            header_size = sizeof(VAGp);
            //channels = vagi.channels;
        }
        else
        {
            fread(&ads, sizeof(ADS), 1, in);
            sample_size = ads.sample_size;
            sample_rate = ads.sample_rate;
            interleave = ads.interleave;
            header_size = sizeof(ADS);
            //channels = ads.channels;
        }
        /*
        bitrate = ((sample_rate << 5) / 0x38) * channels;
        duration = ((sample_size >> 5) * 0x38) / channels;
        
        char header_check[16];
        fread(header_check, 1, 16, in);
        if(memcmp(header_check, BD, 16) != 0)
        {
            //add header spacing if missing
            fwrite(BD, 1, 16, new_bd_file);
            add_header = 16;
        }*/
        
        fseek(in, header_size, SEEK_SET);

        //file_size -= sizeof(header_size);
        uint32_t padded_size;

        if(sample_size % interleave * (0x38 / 0x10) == 0)
        {
            padded_size = sample_size;
        }else{
            padded_size = sample_size + (interleave - (sample_size % interleave * (0x38 / 0x10)));
        }

        char *data = calloc(padded_size, 1);
        fread(data, 1, file_size, in);
        fwrite(data, 1, padded_size, new_bd_file);
        fseek(info_file, 0x10 + i * sizeof(music_track), SEEK_SET);
        fseek(info_file, 4, SEEK_CUR);

        fwrite(&sample_rate, 4, 1, info_file);
        fwrite(&sample_size, 4, 1, info_file);
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

void list_music(const char *music_info, const char *music_bd, const char *out_log)
{
    music_track *tracks;
    uint32_t num_tracks; // @ 0x08 in music_info
    uint32_t bitrate, duration;
    FILE *log_file;
    FILE *info_file = fopen(music_info, "rb");
    if(!info_file)
    {
        printf("Error: Could not open %s\n", music_info);
        return;
    }

    if(out_log != NULL)
    {
        log_file = fopen(out_log, "w");
        if(!log_file)
        {
            printf("Error: Could not open %s for writing\n", out_log);
            return;
        }
    }

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
        char *data = malloc(tracks[i].padded_size);
        fread(data, 1, tracks[i].padded_size, bd_file);
        printf("%04d_%s\n", i, tracks[i].name);
        if(tracks[i].stereo)
        {
            bitrate = ((tracks[i].sample_rate << 5) / 0x38) * 0x2;
            duration = ((tracks[i].padded_size >> 5) * 0x38) / 0x2;
        }else{
            bitrate = ((tracks[i].sample_rate << 5) / 0x38);
            duration = ((tracks[i].padded_size >> 5) * 0x38);
        }
        if(out_log != NULL)
        {
            fprintf(log_file, "%04d_%s\n", i, tracks[i].name);
            fprintf(log_file, "\tStereo: %s\n", tracks[i].stereo ? "Yes" : "No");
            fprintf(log_file, "\tSample Rate: %d Hz\n", tracks[i].sample_rate);
            fprintf(log_file, "\tPadded Size: %#08x bytes\n", tracks[i].padded_size);
            fprintf(log_file, "\tInterleave: %#08x bytes\n", get_interleave(data, tracks[i].padded_size));
            fprintf(log_file, "\tBit rate: %d byte/sec\n", bitrate);
            fprintf(log_file, "\tDuration: %f sec\n", (double)duration / tracks[i].sample_rate);
            //fprintf(log_file, "\tDuration * bit rate: %#08x bytes\n", tracks[i].track_size);
        }else{
            printf("\tStereo: %s\n", tracks[i].stereo ? "Yes" : "No");
            printf("\tSample Rate: %d Hz\n", tracks[i].sample_rate);
            printf("\tSample Size: %#08x bytes\n", tracks[i].padded_size);
            printf("\tInterleave: %#08x bytes\n", get_interleave(data, tracks[i].padded_size));
            printf("\tBit rate: %d byte/sec\n", bitrate);
            printf("\tDuration: %f sec\n", (double)duration / tracks[i].sample_rate);
            //printf("\tDuration * bit rate: %#08x bytes\n", tracks[i].track_size);
        }
        free(data);
    }

    free(tracks);
    fclose(bd_file);
    if(out_log != NULL) fclose(log_file);
    if(out_log != NULL) printf("Log written to %s\n", out_log);
    fclose(info_file);

    printf("Done\n");
} 