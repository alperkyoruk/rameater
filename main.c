#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define CHUNK_SIZE (100 * 1024 * 1024) // 100MB chunks

int main() {
 size_t total_allocated = 0; // Track total memory allocated in bytes
 void **chunks = NULL; // Array to store pointers to allocated chunks
 size_t chunk_count = 0; // Number of chunks allocated
 size_t attempts = 0; // Count allocation attempts

 printf("Starting to eat RAM. This will run until you stop it or the system limits it.\n");

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
 } else {
 printf("Failed to realloc chunk array after %zu chunks.\n", chunk_count);
 }

 // Report progress
 printf("Allocated %zu MB so far after %zu attempts.\n", 
 total_allocated / (1024 * 1024), attempts);

 // Periodically write to a random chunk to keep memory in the working set
 if (chunk_count > 0) {
 size_t idx = rand() % chunk_count;
 memset(chunks[idx], 42, CHUNK_SIZE);
 }
 } else {
 printf("Failed to allocate more memory after %zu attempts and %zu MB.\n", 
 attempts, total_allocated / (1024 * 1024));
 // Keep running to test limits, but report failure
 }
 }

 // Note: This is an infinite loop, so we never reach here
 // Included for completeness to avoid memory leaks if adapted later
 for (size_t i = 0; i < chunk_count; i++) {
 free(chunks[i]);
 }
 free(chunks);
 return 0;
}
