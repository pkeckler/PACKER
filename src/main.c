#include <stdio.h>
#include <stdint.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define PACKER_IMPLEMENTATION
#include "packer.h"

int main(int argc, char** argv)
{

    bool packer = false;

    bool data_loaded = false;
    packer_header_t data_header;
    FILE* data_file = NULL;

    InitWindow(320, 240, "Asset Packer");
    SetTargetFPS(60);
    SetExitKey(KEY_F12);

    while (!WindowShouldClose())
    {

        if (IsKeyPressed(KEY_F5)) {
            packer = !packer;
            if (packer) SetWindowTitle("Asset Packer");
            else SetWindowTitle("Asset Packer");
        }

        if (packer) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            int count = 0;
            char** filenames = GetDroppedFiles(&count);

            GuiLabel((Rectangle){10, 10, 100, 20}, "Asset packer");
            GuiPanel((Rectangle){10, 30, 140, 200});
            for (int i = 0; i < count; i++) {
                GuiLabel((Rectangle){15, 40+i*10, 100, 10}, GetFileName(filenames[i]));
            }

            if (GuiButton((Rectangle){170, 30, 90, 20}, "Pack Data")) {
                printf("packing data..\n");

                FILE* file = fopen("DATA.BIN", "wb");
                packer_write(file, filenames, count);
                ClearDroppedFiles();
                fclose(file);

            }

            EndDrawing();
            continue;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (GuiButton((Rectangle){10, 10, 90, 20}, "Load Assets")) {
            if (data_file) fclose(data_file);

            data_loaded = true;
            data_file = fopen("DATA.BIN", "rb");
            packer_debug(data_file);
            if (data_file) {
                fread(&data_header, sizeof(packer_header_t), 1, data_file);
            }
        }

        if (data_loaded && data_file) {
            DrawRectangleLines(10, 40, 300, 20, WHITE);
            DrawRectangleLines(10, 40, 300, 190, WHITE);
            DrawText("Header", 20, 45, 10, WHITE);

            DrawText(FormatText("sig = %c%c%c%c", data_header.sig[0], data_header.sig[1], data_header.sig[2], data_header.sig[3]), 20, 65, 10, WHITE);
            DrawText(FormatText("count = %d", data_header.count), 20, 75, 10, WHITE);
            DrawText(FormatText("version = %d", data_header.version), 20, 85, 10, WHITE);
            DrawLine(10, 100, 310, 100, WHITE);

            fseek(data_file, sizeof(packer_header_t), SEEK_SET);
            for (int i = 0; i < data_header.count; i++) {
                packer_lump_t lump;
                
                fread(&lump, sizeof(packer_lump_t), 1, data_file);
                
                char* name = malloc(sizeof(char)*lump.name_size);
                fread(name, sizeof(char)*lump.name_size, 1, data_file);

                void* data = malloc(lump.size);
                fread(data, lump.size, 1, data_file);

                DrawText(FormatText("%d | type:%d size:%d (%d)%s [%d,%d,%d,%d,%d,%d,%d,%d]\n",
                    lump.id, lump.type, lump.size, lump.name_size, name,
                    lump.params[0], lump.params[1], lump.params[2], lump.params[3],
                    lump.params[4], lump.params[5], lump.params[6], lump.params[7]), 15, 110+lump.id*10, 10, WHITE);
                // printf("%s\n", data);
                
                free(name);
                free(data);
            }
            // return 1;

        } else {
            DrawText("data file not opened or\nfailed to load.", 10, 45, 10, RED);
        }

        EndDrawing();

    }

    CloseWindow();

    if (data_file)
        fclose(data_file);

	return 0;
}
