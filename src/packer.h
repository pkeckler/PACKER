#ifndef PACKER_H
#define PACKER_H

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

typedef enum packer_lump_e {
    LUMP_IMAGE,
    LUMP_WAVE,
    LUMP_VERTEX,
    LUMP_RAW,
    LUMP_COUNT
} packer_lump_e;

typedef struct packer_lump_t {
    unsigned int id;
    packer_lump_e type;
    unsigned int size;
    unsigned int name_size;
    unsigned int params[8];
} packer_lump_t;

typedef struct packer_header_t {
    char sig[4];
    unsigned int count;
    unsigned int version;
} packer_header_t;

void packer_write(FILE*, char**, int);
void packer_debug(FILE*);

#endif

#ifdef PACKER_IMPLEMENTATION

void packer_write(FILE* file, char** filenames, int count)
{
    packer_header_t header;
    memcpy(&header.sig[0], "p@ck", 4);
    header.count = count;
    header.version = 0x01;
    
    fwrite(&header, sizeof(header), 1, file);

    for (int i = 0; i < count; i++) {
        packer_lump_t lump;
        lump.id = i;
        memset(lump.params, 0, sizeof(lump.params));
        lump.name_size = strlen(GetFileName(filenames[i]))+1;

        const char* ext = GetExtension(filenames[i]);
        // alternative plain text
        if (!ext) {
            lump.type = LUMP_RAW;

            char* data = LoadText(filenames[i]);
            lump.size = strlen(data);

            fwrite(&lump, sizeof(lump), 1, file);
            fwrite(GetFileName(filenames[i]), sizeof(char)*lump.name_size, 1, file);
            fwrite(data, lump.size, 1, file);

            free(data);
            continue;
        }

        // images
        if (!strcmp(ext, "jpg") || !strcmp(ext, "jpeg") || !strcmp(ext, "png")) {
            lump.type = LUMP_IMAGE;

            Image img = LoadImage(filenames[i]);
            lump.size = GetPixelDataSize(img.width, img.height, img.format);

            lump.params[0] = img.width;
            lump.params[1] = img.height;
            lump.params[2] = img.mipmaps;
            lump.params[3] = img.format;

            fwrite(&lump, sizeof(lump), 1, file);
            fwrite(GetFileName(filenames[i]), sizeof(char)*lump.name_size, 1, file);
            fwrite(img.data, lump.size, 1, file);

            UnloadImage(img);
        }
        // meshes
        else if (!strcmp(ext, "obj")) {
            // TODO: 
            lump.type = LUMP_VERTEX;
            
            Mesh mesh = LoadMesh(filenames[i]);
            lump.size = mesh.vertexCount;
        }
        else if (!strcmp(ext, "wav")) {
            lump.type = LUMP_WAVE;
            
            // sampleCount*sampleSize/8
            Wave wave = LoadWave(filenames[i]);
            
            lump.size = wave.sampleCount*wave.sampleSize/8;
            lump.params[0] = wave.sampleCount;
            lump.params[1] = wave.sampleRate;
            lump.params[2] = wave.sampleSize;
            lump.params[3] = wave.channels;
            
            fwrite(&lump, sizeof(lump), 1, file);
            fwrite(GetFileName(filenames[i]), sizeof(char)*lump.name_size, 1, file);
            fwrite(wave.data, lump.size, 1, file);
            
        }
        // plain text (e.g. shaders)
        else {
            lump.type = LUMP_RAW;

            char* data = LoadText(filenames[i]);
            lump.size = strlen(data);

            fwrite(&lump, sizeof(lump), 1, file);
            fwrite(GetFileName(filenames[i]), sizeof(char)*lump.name_size, 1, file);
            fwrite(data, lump.size, 1, file);

            free(data);
        }
    }
}

void packer_debug(FILE* file)
{
    fseek(file, 0, SEEK_SET);
    
    packer_header_t header;
    fread(&header, sizeof(packer_header_t), 1, file);
    
    printf("+-------------------------+\n");
    printf("| Header                  |\n");
    printf("+-------------------------+\n");
    printf("| %c%c%c%c                    |\n", header.sig[0], header.sig[1], header.sig[2], header.sig[3]);
    printf("| Count   = %-2d            |\n", header.count);
    printf("| Version = %-2d            |\n", header.version);
    printf("+-------------------------+\n");
    for (int i = 0; i < header.count; i++) {
        packer_lump_t lump;
        fread(&lump, sizeof(packer_lump_t), 1, file);
        char* name = malloc(lump.name_size);
        fread(name, lump.name_size, 1, file);
        printf("+-------------------------+\n");
        printf("| Lump                    |\n");
        printf("+-------------------------+\n");
        printf("|                         |\n");
        printf("| Id   = %-3d              |\n", lump.id);
        printf("| Type = %d                |\n", lump.type);
        printf("| Name = %-16s |\n", name);
        printf("| Size = %-16d |\n", lump.size);
        printf("+-------------------------+\n");
        fseek(file, lump.size, SEEK_CUR);
    }
    
    fseek(file, 0, SEEK_SET);
}

#endif
