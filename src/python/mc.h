struct OpenFile {
    int modified;
    const char *filename;
    const char *filepath;
};
typedef struct OpenFile OpenFile;

struct OpenFiles {
    OpenFile *_array;
    size_t _count;
};
typedef struct OpenFiles OpenFiles;

extern OpenFiles* pymc_get_open_files(void);