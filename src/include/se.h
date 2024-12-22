void extract_sounds(const char *hd_info, const char *bd_data, const char *output_folder)
{
    SCEIVers vers;
    SCEIHead head;
    SCEIVagi vagi;
    SCEIVagiData *vagi_data;

    FILE *hd_file = fopen(hd_info, "rb");
    if(!hd_file)
    {
        printf("Error: Could not open %s\n", hd_info);
        return;
    }
    FILE *bd_file = fopen(bd_data, "rb");
    if(!bd_file)
    {
        printf("Error: Could not open %s\n", bd_data);
        return;
    }

#ifdef _WIN32
    mkdir(output_folder);
#else
    mkdir(output_folder, 0700);
#endif

    printf("Extracting sounds from %s\n", bd_data);

    fread(&vers, sizeof(SCEIVers), 1, hd_file);
    if(vers.SCEIVers_size > 0x10) fseek(hd_file, vers.SCEIVers_size - 0x10, SEEK_CUR);
    fread(&head, sizeof(SCEIHead), 1, hd_file);
    fseek(hd_file, head.SCEIVagi_offset, SEEK_SET);
    fread(&vagi, sizeof(SCEIVagi), 1, hd_file);

    printf("Found %d sounds\n", vagi.data_count + 1);

    vagi_data = malloc((vagi.data_count + 2) * sizeof(SCEIVagiData));
    uint32_t *vagi_offsets = malloc((vagi.data_count + 1) * sizeof(uint32_t)); //these start from SCEIVagi_offset rather than 0x0

    for(int i = 0; i <= vagi.data_count; i++)
    {
        fread(&vagi_offsets[i], 4, 1, hd_file);
    }

    for(int i = 0; i <= vagi.data_count; i++)
    {
        fseek(hd_file, vagi_offsets[i] + head.SCEIVagi_offset, SEEK_SET);
        fread(&vagi_data[i], sizeof(SCEIVagiData), 1, hd_file);
    }

    fseek(bd_file, 0, SEEK_END);
    uint32_t bd_size = ftell(bd_file);
    fseek(bd_file, 0, SEEK_SET);

    vagi_data[vagi.data_count+1].start_offset = bd_size;

    for(int i = 0; i <= vagi.data_count; i++)
    {
        char *filename = malloc(strlen(output_folder) + 12);
        sprintf(filename, "%s/%04d.%s", output_folder, i, "VAG");
        FILE *out = fopen(filename, "wb");
        if(!out)
        {
            printf("Error: Could not open %s for writing\n", filename);
            continue;
        }

        uint32_t size = vagi_data[i + 1].start_offset - vagi_data[i].start_offset;

        char *data = malloc(size);
        fseek(bd_file, vagi_data[i].start_offset, SEEK_SET);
        fread(data, 1, size, bd_file);
        write_vagp_header(out, vagi_data[i].sample_rate, size);
        fwrite(data, 1, size, out);
        fclose(out);
        free(data);
    }

    fclose(bd_file);
    fclose(hd_file);

    printf("Done\n");
}

void import_sounds(const char *hd_info, const char *bd_data, const char *input_folder)
{
    SCEIVers vers;
    SCEIHead head;
    SCEIVagi vagi;
    SCEIVagiData *vagi_data;
    int file_index;
    int *to_import;
    DIR *dir;
    struct dirent *entry;
    

    FILE *hd_file = fopen(hd_info, "r+b");
    if(!hd_file)
    {
        printf("Error: Could not open %s\n", hd_info);
        return;
    }
    FILE *bd_file = fopen(bd_data, "r+b");
    if(!bd_file)
    {
        printf("Error: Could not open %s\n", bd_data);
        return;
    }

    fread(&vers, sizeof(SCEIVers), 1, hd_file);
    if(vers.SCEIVers_size > 0x10) fseek(hd_file, vers.SCEIVers_size - 0x10, SEEK_CUR);
    fread(&head, sizeof(SCEIHead), 1, hd_file);
    fseek(hd_file, head.SCEIVagi_offset, SEEK_SET);
    fread(&vagi, sizeof(SCEIVagi), 1, hd_file);

    to_import = calloc(vagi.data_count + 1, sizeof(int));

    vagi_data = malloc((vagi.data_count + 2) * sizeof(SCEIVagiData));
    uint32_t *vagi_offsets = malloc((vagi.data_count + 1) * sizeof(uint32_t)); //these start from SCEIVagi_offset rather than 0x0

    for(int i = 0; i <= vagi.data_count; i++)
    {
        fread(&vagi_offsets[i], 4, 1, hd_file);
    }

    for(int i = 0; i <= vagi.data_count; i++)
    {
        fseek(hd_file, vagi_offsets[i] + head.SCEIVagi_offset, SEEK_SET);
        fread(&vagi_data[i], sizeof(SCEIVagiData), 1, hd_file);
    }

    fseek(bd_file, 0, SEEK_END);
    uint32_t bd_size = ftell(bd_file);
    fseek(bd_file, 0, SEEK_SET);

    vagi_data[vagi.data_count+1].start_offset = bd_size;

    dir = opendir(input_folder);
    if (dir == NULL)
    {
        printf("Error: Could not open %s\n", input_folder);
        return;
    }

    FILE *new_bd_file = fopen("new_sounds.bd", "wb");
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

        if (file_index < 0 || file_index > vagi.data_count)
        {
            printf("Invalid file index for file %s\n", entry->d_name);
            continue;
        }

        to_import[file_index] = 1;
    }

    for(int i = 0; i <= vagi.data_count; i++)
    {
        uint32_t size = vagi_data[i + 1].start_offset - vagi_data[i].start_offset;
        uint32_t current_bd_offset = ftell(new_bd_file);
        if (to_import[i] == 0)
        {
            fseek(hd_file, vagi_offsets[i] + head.SCEIVagi_offset, SEEK_SET);
            fwrite(&current_bd_offset, 4, 1, hd_file);
            char *unchanged = malloc(size);
            fread(unchanged, 1, size, bd_file);
            fwrite(unchanged, 1, size, new_bd_file);
            free(unchanged);
            continue;
        }else{
            fseek(bd_file, size, SEEK_CUR);
        }
        printf("Importing %04d\n", i);

        char *filename = malloc(strlen(input_folder) + 12);
        sprintf(filename, "%s/%04d.VAG", input_folder, i);
        FILE *in = fopen(filename, "rb");
        if(!in)
        {
            printf("Error: Could not open %s for reading\n", filename);
            continue;
        }
        fseek(in, 0, SEEK_SET);

        VAGp vagp;
        fread(&vagp, sizeof(VAGp), 1, in);
        vagp.size = swap_uint32(vagp.size);
        vagp.sample_rate = swap_uint32(vagp.sample_rate);

        size = vagp.size;

        //uint32_t block_size = 0x10;
        uint32_t padded_size = (1 + (size / (0x38 / 0x10))) * (0x38 / 0x10);

        char *data = calloc(padded_size, 1);
        fread(data, 1, size, in);
        fwrite(data, 1, padded_size, new_bd_file);
        fseek(hd_file, vagi_offsets[i] + head.SCEIVagi_offset, SEEK_SET);
        fwrite(&current_bd_offset, 4, 1, hd_file);
        fwrite(&vagp.sample_rate, 2, 1, hd_file);
        fclose(in);
        free(data);
    }

    fclose(new_bd_file);
    fclose(bd_file);
    fclose(hd_file);

    remove(bd_data);
    rename("new_sounds.bd", bd_data);
    free(vagi_data);
    free(to_import);

    closedir(dir);

    printf("Done\n");
}

void list_sounds(const char *hd_info, const char *out_log)
{
    SCEIVers vers;
    SCEIHead head;
    SCEIVagi vagi;
    SCEIVagiData *vagi_data;

    uint32_t bitrate;

    FILE *log_file;
    FILE *hd_file = fopen(hd_info, "rb");
    if(!hd_file)
    {
        printf("Error: Could not open %s\n", hd_info);
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

    fread(&vers, sizeof(SCEIVers), 1, hd_file);
    if(vers.SCEIVers_size > 0x10) fseek(hd_file, vers.SCEIVers_size - 0x10, SEEK_CUR);
    fread(&head, sizeof(SCEIHead), 1, hd_file);
    fseek(hd_file, head.SCEIVagi_offset, SEEK_SET);
    fread(&vagi, sizeof(SCEIVagi), 1, hd_file);

    printf("Found %d sounds\n", vagi.data_count + 1);

    vagi_data = malloc((vagi.data_count + 2) * sizeof(SCEIVagiData));
    uint32_t *vagi_offsets = malloc((vagi.data_count + 1) * sizeof(uint32_t)); //these start from SCEIVagi_offset rather than 0x0

    for(int i = 0; i <= vagi.data_count; i++)
    {
        fread(&vagi_offsets[i], 4, 1, hd_file);
    }

    for(int i = 0; i <= vagi.data_count; i++)
    {
        fseek(hd_file, vagi_offsets[i] + head.SCEIVagi_offset, SEEK_SET);
        fread(&vagi_data[i], sizeof(SCEIVagiData), 1, hd_file);
    }

    for(int i = 0; i <= vagi.data_count; i++)
    {
        bitrate = ((vagi_data[i].sample_rate << 5) / 0x38);
        printf("%04d: %dHz Sample Rate, %s\n", i, vagi_data[i].sample_rate, vagi_data[i].looping ? "looping" : "");
        if(out_log != NULL)
        {
            fprintf(log_file, "%04d:\n", i);
            fprintf(log_file, "\tSample Rate: %d Hz\n", vagi_data[i].sample_rate);
            fprintf(log_file, "\tStart Offset: %#08x\n", vagi_data[i].start_offset);
            fprintf(log_file, "\t%s\n", vagi_data[i].looping ? "Looping" : "Not Looping");
            fprintf(log_file, "\tBitrate: %d byte/sec\n", bitrate);
        }
    }

    fclose(hd_file);
    if(out_log != NULL) fclose(log_file);
    if(out_log != NULL) printf("Log written to %s\n", out_log);
    free(vagi_data);
    free(vagi_offsets);
}