//// file browser stuff
typedef struct
{
	char name[32];// File Name
	bool isFile;
}DirEntry;
DirEntry Files[128];
int numberOfFiles;

void getDirectory(const char * path) {
    numberOfFiles = 0;
    Directory dir;
    if(dir.open(path)){
        while (auto file = dir.read()) {
            char *ext = strrchr(file->name(), '.');
            if (ext && (!strcmp(ext, ".ch8") || !strcmp(ext, ".CH8") || !strcmp(ext, ".cH8") || !strcmp(ext, ".Ch8") )){
                strcpy(Files[numberOfFiles].name,file->name());
                numberOfFiles++;
            }
            //printf("%s\n",file->name());
        }
    }
    printf("%d Files Found.\n",numberOfFiles);
    for(int t=0; t<numberOfFiles; t++){
        printf("%s\n",Files[t].name);
    }
}

