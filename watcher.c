#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#define MAX_CMD 1024
#define MAX_PATH_SIZE MAX_PATH
PROCESS_INFORMATION currentProcess = {0};
#else
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <linux/limits.h>
#include <signal.h>
#include <errno.h>
#define MAX_PATH_SIZE PATH_MAX
pid_t childPid = -1;
#endif

char initialCwd[MAX_PATH_SIZE];
char lastModifiedFile[MAX_PATH_SIZE] = "";
time_t lastRunTime = 0;

void killPreviousProcess()
{
#ifdef _WIN32
    if (currentProcess.hProcess)
    {
        TerminateProcess(currentProcess.hProcess, 0);
        CloseHandle(currentProcess.hProcess);
        CloseHandle(currentProcess.hThread);
        ZeroMemory(&currentProcess, sizeof(currentProcess));
        printf("[Watcher] Previous process killed.\n");
    }
#else
    if (childPid > 0)
    {
        kill(childPid, SIGKILL);
        waitpid(childPid, NULL, 0);
        printf("[Watcher] Previous process killed.\n");
        childPid = -1;
    }
#endif
}

void startProcess(const char *command)
{
    // Ensure previous process is terminated
    killPreviousProcess();

#ifdef _WIN32
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    char cmd[MAX_CMD];
    strncpy(cmd, command, MAX_CMD);

    BOOL success = CreateProcessA(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &currentProcess);

    if (!success)
    {
        printf("Failed to start process: %lu\n", GetLastError());
    }
    else
    {
        printf("[Watcher] Started process: %s\n", command);
    }
#else
    childPid = fork();
    if (childPid == 0)
    {
        // child process
        chdir(initialCwd);
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        perror("execl failed");
        exit(1);
    }
    else if (childPid > 0)
    {
        printf("[Watcher] Started process: %s\n", command);
    }
    else
    {
        perror("fork failed");
    }
#endif

    lastRunTime = time(NULL); // Update last run time
}

// Handle file changes
void handleChange(const char *path, const char *filename, const char *command)
{
    // 1 second debounce
    if (difftime(time(NULL), lastRunTime) < 1)
    {
        return;
    }

    // // Skip if the file is the same as the last modified one
    // if (strcmp(lastModifiedFile, filename) == 0) {
    //     return;
    // }

    // Store the last modified file and trigger process
    strncpy(lastModifiedFile, filename, MAX_PATH_SIZE);
    printf("[Watcher] File changed: %s\n", filename);
    startProcess(command);
}

// Watch the directory for changes
#ifdef _WIN32
void watchDirectory(const char *path, const char *command)
{
    HANDLE hDir = CreateFileA(
        path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (hDir == INVALID_HANDLE_VALUE)
    {
        printf("Error opening directory: %lu\n", GetLastError());
        return;
    }

    char buffer[2048];
    DWORD bytesReturned;
    printf("[Watcher] Watching: %s\n", path);

    while (1)
    {
        if (ReadDirectoryChangesW(
                hDir,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME |
                    FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytesReturned,
                NULL,
                NULL))
        {
            FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION *)buffer;
            do
            {
                char filename[MAX_PATH_SIZE];
                int count = WideCharToMultiByte(
                    CP_ACP, 0, info->FileName,
                    info->FileNameLength / sizeof(WCHAR),
                    filename, MAX_PATH_SIZE - 1,
                    NULL, NULL);
                filename[count] = '\0';

                handleChange(path, filename, command);

                if (info->NextEntryOffset == 0)
                    break;
                info = (FILE_NOTIFY_INFORMATION *)((char *)info + info->NextEntryOffset);
            } while (1);
        }
        else
        {
            printf("Failed to read changes: %lu\n", GetLastError());
            break;
        }
    }

    CloseHandle(hDir);
    killPreviousProcess(); // Cleanup
}
#else
void watchDirectory(const char *path, const char *command)
{
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0)
    {
        perror("inotify_init");
        return;
    }

    int wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);
    if (wd < 0)
    {
        perror("inotify_add_watch");
        return;
    }

    printf("[Watcher] Watching: %s\n", path);

    char buffer[MAX_PATH_SIZE];
    while (1)
    {
        int length = read(fd, buffer, sizeof(buffer));
        if (length < 0 && errno != EAGAIN)
        {
            perror("read");
            break;
        }

        if (length > 0)
        {
            struct inotify_event *event = (struct inotify_event *)buffer;
            if (event->len)
            {
                printf("[Watcher] Change detected: %s\n", event->name);
                handleChange(path, event->name, command);
            }
        }

        usleep(100000); // 100ms
    }

    close(fd);
    killPreviousProcess(); // Cleanup
}
#endif

int main(int argc, char *argv[])
{
    if (argc < 4 || strcmp(argv[2], "--exec") != 0)
    {
        printf("Usage: %s <dir> --exec \"<command_to_run>\"\n", argv[0]);
        return 1;
    }

#ifdef _WIN32
    GetCurrentDirectoryA(MAX_PATH_SIZE, initialCwd);
#else
    getcwd(initialCwd, sizeof(initialCwd));
#endif

    const char *path = argv[1];
    const char *command = argv[3];
    watchDirectory(path, command);
    return 0;
}
