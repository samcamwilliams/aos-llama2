#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <emscripten.h>

// WeaveDrive file functions
EM_ASYNC_JS(int, arweave_fopen, (const char* filename_c, const char* mode), {
    try {
        const filename = UTF8ToString(Number(filename_c));
        
        await new Promise((r) => setTimeout(r, 1000));

        const pathCategory = filename.split('/')[1];
        const id = filename.split('/')[2];
        console.log("JS: Opening file: ", filename);

        if (pathCategory === 'data') {
            if(FS.analyzePath(filename).exists) {
                console.log("JS: File exists: ", filename);
                const file = FS.open("/data/" + id, UTF8ToString(Number(mode)));
                console.log("JS: File opened: ", file.fd);
                return Promise.resolve(file.fd);
            }
            else {
                console.log("JS: File does not exist: ", filename);
                return Promise.resolve(0);
            }
        }
        else if (pathCategory === 'headers') {
            console.log("Header access not implemented yet.");
            return Promise.resolve(0);
        }
        return Promise.resolve(0);
  } catch (err) {
    console.error('Error opening file:', err);
    return Promise.resolve(0);
  }
});

EM_ASYNC_JS(size_t, afs_fread, (void* fd, size_t buffer, size_t size), {
  try {
    console.log('PATH: ', FS.streams[fd].path);
    const data = FS.read(FS.streams[fd], HEAPU8, buffer, size, 0);
    return data;
  } catch (err) {
    console.error('Error reading file: ', err);
    return -1;
  }
});

// NOTE: This may not actually be necessary. But if it is, here is how we would
// emulate the 'native' emscripten fopen.
FILE *fallback_fopen(const char *filename, const char *mode) {
    int fd;
    int flags;

    // Basic mode to flags translation
    if (strcmp(mode, "r") == 0) {
        flags = O_RDONLY;
    } else if (strcmp(mode, "w") == 0) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    }

    // Open file and convert to FILE*
    fd = open(filename, flags, 0666); // Using default permissions directly
    if (fd == -1) { // If fd is -1, return NULL as if the fopen failed
        return NULL;
    }
    return fdopen(fd, mode);
}

FILE* fopen(const char* filename, const char* mode) {
    fprintf(stderr, "AO: Called fopen: %s, %s\n", filename, mode);
    FILE* res = (FILE*) 0;
    if (strncmp(filename, "/data", 5) == 0 || strncmp(filename, "/headers", 8) == 0) {
        int fd = arweave_fopen(filename, mode);
        fprintf(stderr, "AO: arweave_fopen returned fd: %d\n", fd);
        if (fd) {
            res = fdopen(fd, mode);
        }
    }
    fprintf(stderr, "AO: fopen returned: %p\n", res);
    return res; 
}

int fclose(FILE* stream) {
     fprintf(stderr, "Called fclose\n");
     return 0;  // Returning success, adjust as necessary
}

/*
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
  printf("read file\n");
    //afs_fread(ptr, size, nmemb);
    return 0;
}

int fseek(FILE* stream, long offset, int whence) {
    fprintf(stderr, "Called fseek with offset: %ld, whence: %d\n", offset, whence);
    return 0;  // Returning success, adjust as necessary
}

long ftell(FILE* stream) {
    fprintf(stderr, "Called ftell\n");
    return 0;  // Returning 0 as the current position, adjust as necessary
}
*/