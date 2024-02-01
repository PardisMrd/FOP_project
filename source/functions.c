#include "header.h"

mode_t CompareMode(char *file1, char *file2)
{
    struct stat stat1, stat2;
    stat(file1, &stat1);
    stat(file2, &stat2);
    return stat1.st_mode == stat2.st_mode;
}

/* If .neogit exists, path will be cwd of the folder it's in it. */
// 1 if .neogit exists, 0 if not
int Find_Repo(char *path)
{
    if (!path || !(*path))
        return 0;
    DIR *dir = opendir(path);
    struct dirent *entry;
    if (dir == NULL)
        return 0;

    while ((entry = readdir(dir)) != NULL)
        if (!strcmp(entry->d_name, ".neogit"))
            return 1;
    closedir(dir);
    path[strlen(path)] = '\0'; // Guess can be ommites

    char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL)
        *lastSlash = '\0';

    return Find_Repo(path);
}

void ChangeConfig_UserName(char *infoPath, const char name[])
{
    FILE *fp_r = fopen(infoPath, "r");
    char *line = malloc(2000);
    size_t n = 0;
    while (getline(&line, &n, fp_r) != -1)
        if (strncmp(line, "email =", 7) == 0)
            break;
    fclose(fp_r);
    FILE *fp_w = fopen(infoPath, "w");
    fprintf(fp_w, "name =%s\n%s", name, line);
    free(line);
    fclose(fp_w);
    printf("Username added successfully!\n");
}

void ChangeConfig_UserEmail(char *infoPath, const char email[])
{
    FILE *fp_r = fopen(infoPath, "r");
    char *line = malloc(2000);
    size_t n = 0;
    while (getline(&line, &n, fp_r) != -1)
        if (strncmp(line, "name =", 6) == 0)
            break;
    fclose(fp_r);
    FILE *fp_w = fopen(infoPath, "w");
    fprintf(fp_w, "%semail =%s\n", line, email);
    free(line);
    fclose(fp_w);
    printf("Useremail added successfully!\n");
}

// 0 if different, 1 if same
int IsModified(char *path1, char *path2) // check when file1 has something more and elsewise
{
    int ch1, ch2;
    FILE *file1 = fopen(path1, "rb");
    if (file1 == NULL)
        return 0;
    FILE *file2 = fopen(path2, "rb");
    if (file2 == NULL)
        return 0;
    while ((ch1 = fgetc(file1)) != EOF && (ch2 = fgetc(file2)) != EOF)
        if (ch1 != ch2)
        {
            fclose(file1);
            fclose(file2);
            return 0;
        }
    ch2 = fgetc(file2);
    if (ch1 != ch2)
    {
        fclose(file1);
        fclose(file2);
        return 0;
    }
    fclose(file1);
    fclose(file2);
    return 1;
}

void ListFolders_n(char ProjPath[], char curAddress[], int depth, int Cdepth, bool check)
{
    if (!depth)
        return;
    DIR *cur_dir = opendir(curAddress);
    struct dirent *entry;
    char folder_name[PATH_MAX];
    while ((entry = readdir(cur_dir)) != NULL)
    {
        strcpy(folder_name, entry->d_name);
        if (entry->d_type == DT_DIR && strncmp(folder_name, ".", 1))
        {
            strcat(curAddress, "/");
            strcat(curAddress, folder_name);
            printf("\n");
            for (int i = 0; i < Cdepth - depth + 1; i++)
                printf("\t");
            printf("Entering " _SGR_BOLD _SGR_YELLOWF "%s : \n"_SGR_RESET, folder_name);
            ListFolders_n(ProjPath, curAddress, depth - 1, Cdepth, true);
        }

        else if (entry->d_type != DT_DIR)
        {
            for (int i = 0; i < Cdepth - depth + 1; i++)
                printf("\t");
            char WDPath[PATH_MAX];
            strcpy(WDPath, curAddress);
            strcat(WDPath, "/");
            strcat(WDPath, folder_name);

            char SAPath[PATH_MAX];
            strcpy(SAPath, ProjPath);
            int ignore = strlen(SAPath);
            strcat(SAPath, "/.neogit/.staged/");
            strcat(SAPath, curAddress + ignore);
            strcat(SAPath, "/");
            strcat(SAPath, folder_name);
            printf("%s", folder_name);

            if (!IsModified(SAPath, WDPath))
            {
                printf(_SGR_REDF _SGR_BOLD "\tU\n" _SGR_RESET);
                check = false;
            }
            else
                printf(_SGR_GREENF _SGR_BOLD "\tS\n" _SGR_RESET);
        }
    }
    for (int i = 0; i < Cdepth - depth; i++)
        printf("\t");
    if (check)
        printf("*Directory is " _SGR_BLUEF "staged\n\n" _SGR_RESET);
    else
        printf("*Directory is " _SGR_BLUEF "not staged\n\n" _SGR_RESET);
    while (curAddress[strlen(curAddress) - 1] != '/')
        curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cur_dir);
    return;
}

void CreateFile(char FileName[], char ToWrite[])
{
    FILE *file = fopen(FileName, "w");
    if (ToWrite)
        fputs(ToWrite, file);
    fclose(file);
    return;
}

// 1 if .neogit created, 0 if already exists
void init()
{
    char cur_dir[PATH_MAX];
    getcwd(cur_dir, sizeof(cur_dir));
    if (Find_Repo(cur_dir))
    {
        printf("Repository already exists!\n");
        return;
    }
    getcwd(cur_dir, sizeof(cur_dir));
    strcat(cur_dir, "/.neogit");
    mkdir(cur_dir, S_IRWXU | S_IRWXG | S_IRWXO);

    strcat(cur_dir, "/config.txt");
    FILE *f = fopen("/home/pardis/tmp.sh", "w");
    fprintf(f, "cp -r /home/pardis/.neogitconfig/info.txt %s", cur_dir);
    fclose(f);
    chmod("/home/pardis/tmp.sh", 0x777);
    system("/home/pardis/tmp.sh");
    remove("/home/pardis/tmp.sh");

    CreateFile(".neogit/branch.txt", "master_0_0\n");
    CreateFile(".neogit/head.txt", "master");
    CreateFile(".neogit/current_IDs.txt", "20000\n10000\n0");
    CreateFile(".neogit/commitPointer.txt", "0\n");
    CreateFile(".neogit/tag.txt", NULL);

    mkdir(".neogit/.stash", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/alias", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/shortcuts", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/commits", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/.staged", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/.records", S_IRWXU | S_IRWXG | S_IRWXO);

    printf("Repository successfully initialized!\n");
    return;
}

int Get_addID(char *ProjPath)
{
    char filePath[PATH_MAX];
    strcpy(filePath, ProjPath);
    strcat(filePath, "/.neogit/current_IDs.txt");
    int toPass_ID = 0;
    char tmp[10];
    FILE *current_IDs = fopen(filePath, "r");
    fgets(tmp, 10, current_IDs);
    for (int i = 0; i < strlen(tmp) - 1; i++)
    {
        toPass_ID *= 10;
        toPass_ID += (tmp[i] - '0');
    }
    fclose(current_IDs);
    return toPass_ID;
}

int Get_commitID(char *ProjPath)
{
    char filePath[PATH_MAX];
    strcpy(filePath, ProjPath);
    strcat(filePath, "/.neogit/current_IDs.txt");
    int toPass_ID = 0;
    char tmp[10];
    FILE *current_IDs = fopen(filePath, "r");
    fgets(tmp, 10, current_IDs);
    fgets(tmp, 10, current_IDs);
    for (int i = 0; i < strlen(tmp) - 1; i++)
    {
        toPass_ID *= 10;
        toPass_ID += (tmp[i] - '0');
    }
    fclose(current_IDs);
    return toPass_ID;
}

void Increase_Decrease_addID(char *ProjPath, char sign)
{
    char filePath[PATH_MAX];
    strcpy(filePath, ProjPath);
    strcat(filePath, "/.neogit/current_IDs.txt");

    FILE *current_IDs = fopen(filePath, "r");
    char addID[10];
    char commitID[100];
    fgets(addID, 10, current_IDs);
    fgets(commitID, 100, current_IDs);
    fclose(current_IDs);
    // ++ the addID
    int pos = strlen(addID) - 2;
    if (sign == '+')
    {
        while (addID[pos] == '9')
        {
            addID[pos] = '0';
            pos--;
        }
        addID[pos]++;
    }
    else
    {
        while (addID[pos] == '0')
        {
            addID[pos] = '9';
            pos--;
        }
        addID[pos]--;
    }

    current_IDs = fopen(filePath, "w");
    fputs(addID, current_IDs);
    fputs(commitID, current_IDs);
    fclose(current_IDs);
    return;
}

void Increase_Decrease_commitID(char *ProjPath, char sign)
{
    char filePath[PATH_MAX];
    strcpy(filePath, ProjPath);
    strcat(filePath, "/.neogit/current_IDs.txt");

    FILE *current_IDs = fopen(filePath, "r");
    char addID[10];
    char commitID[100];
    fgets(addID, 10, current_IDs);
    fgets(commitID, 100, current_IDs);
    fclose(current_IDs);
    // ++ the addID
    int pos = strlen(commitID) - 2;
    if (sign == '+')
    {
        while (commitID[pos] == '9')
        {
            commitID[pos] = '0';
            pos--;
        }
        commitID[pos]++;
    }
    else
    {
        while (commitID[pos] == '0')
        {
            commitID[pos] = '9';
            pos--;
        }
        commitID[pos]--;
    }

    current_IDs = fopen(filePath, "w");
    fputs(addID, current_IDs);
    fputs(commitID, current_IDs);
    fclose(current_IDs);
    return;
}

void CreateDir(const char *dirPath)
{
    if (access(dirPath, F_OK) != 0)
        if (mkdir(dirPath, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
        {
            perror("mkdir"); // replace with printf
            return;
        }
    return;
}

// 0 if found, 1 if not
int FindFile(char *SearchPath) // can be replaced by access ?
{
    int tmp = strlen(SearchPath) - 1;
    while (tmp > 0 && SearchPath[tmp--] != '/')
        ;
    DIR *SearchingDir;
    bool isSlash = tmp;
    if (tmp)
    {
        SearchPath[tmp + 1] = '\0';
        SearchingDir = opendir(SearchPath);
    }
    else
        SearchingDir = opendir(".");

    struct dirent *walking;
    while ((walking = readdir(SearchingDir)) != NULL)
    {
        if (!strcmp(walking->d_name, SearchPath + isSlash * (strlen(SearchPath) + 1)))
        {
            closedir(SearchingDir);
            return 1;
        }
    }
    return 0;
}

void AddToStaging(const char *name, char *ProjPath)
{
    char FileName[PATH_MAX];
    strcpy(FileName, name);
    if (!FindFile(FileName))
    {
        printf("File/Directory not found!\n");
        return;
    }

    char FilePath[PATH_MAX];
    getcwd(FilePath, sizeof(FilePath));

    char StagedPath[PATH_MAX];
    strcpy(StagedPath, ProjPath);
    strcat(StagedPath, "/.neogit/.staged");
    strcat(StagedPath, FilePath + strlen(ProjPath));
    CreateDir(StagedPath);

    strcat(FilePath, "/");
    strcat(FilePath, FileName);
    FILE *f = fopen("/home/pardis/tmp.sh", "w");
    fprintf(f, "cp -r %s %s", FilePath, StagedPath);
    fclose(f);
    chmod("/home/pardis/tmp.sh", 0x777);
    system("/home/pardis/tmp.sh");
    remove("/home/pardis/tmp.sh");
    printf("File added successfully!\n");
}

void AddToRecords(char *ProjPath)
{
    char SAPath[PATH_MAX];
    strcpy(SAPath, ProjPath);
    strcat(SAPath, "/.neogit/.staged");
    char RCPath[PATH_MAX];
    strcpy(RCPath, ProjPath);
    strcat(RCPath, "/.neogit/.records");
    FILE *f = fopen("/home/pardis/tmp.sh", "w");
    fprintf(f, "cp -r %s %s", SAPath, RCPath);
    fclose(f);
    chmod("/home/pardis/tmp.sh", 0x777);
    system("/home/pardis/tmp.sh");
    remove("/home/pardis/tmp.sh");

    int ID = Get_addID(ProjPath);
    char NewName[PATH_MAX];
    strcpy(NewName, RCPath);
    strcat(RCPath, "/.staged");
    strcat(NewName, "/");
    sprintf(NewName + strlen(NewName), "%d", ID);
    rename(RCPath, NewName);
    Increase_Decrease_addID(ProjPath, '+');
}

void AddRedo(char *ProjPath, char *Staged)
{
    DIR *cur_dir = opendir(Staged);
    struct dirent *entry;
    char folder_name[PATH_MAX];
    while ((entry = readdir(cur_dir)) != NULL)
    {
        strcpy(folder_name, entry->d_name);
        if (entry->d_type == DT_DIR && strncmp(folder_name, ".", 1))
        {
            strcat(Staged, "/");
            strcat(Staged, folder_name);
            AddRedo(ProjPath, Staged);
        }
        else if (entry->d_type != DT_DIR)
        {
            char WDPath[PATH_MAX];
            strcpy(WDPath, ProjPath);
            strcat(WDPath, Staged + strlen(ProjPath) + 16); // /.neogit/.staged
            strcat(WDPath, "/");
            strcat(WDPath, folder_name);

            char SAPath[PATH_MAX];
            strcpy(SAPath, Staged);
            strcat(SAPath, "/");
            strcat(SAPath, folder_name);

            char tmp[PATH_MAX];
            strcpy(tmp, WDPath);
            if (!FindFile(tmp))
            {
                remove(SAPath);
                return; // not exist. handle if deleted
            }
            FILE *f = fopen("/home/pardis/tmp.sh", "w");
            fprintf(f, "cp -r %s %s", WDPath, SAPath);
            fclose(f);
            chmod("/home/pardis/tmp.sh", 0x777);
            system("/home/pardis/tmp.sh");
            remove("/home/pardis/tmp.sh");
        }
    }
    while (Staged[strlen(Staged) - 1] != '/')
        Staged[strlen(Staged) - 1] = '\0';
    Staged[strlen(Staged) - 1] = '\0';
    closedir(cur_dir);
    return;
}

void RemoveFromStaging(const char *name, char *ProjPath)
{
    char FileName[PATH_MAX];
    strcpy(FileName, name);

    char FilePath[PATH_MAX];
    getcwd(FilePath, sizeof(FilePath));

    char StagedPath[PATH_MAX];
    strcpy(StagedPath, ProjPath);
    strcat(StagedPath, "/.neogit/.staged");
    strcat(StagedPath, FilePath + strlen(ProjPath));
    strcat(StagedPath, "/");
    strcat(StagedPath, FileName);
    char tmp[PATH_MAX];
    strcpy(tmp, StagedPath);
    if (!FindFile(tmp))
    {
        printf("File/Directory not found!\n");
        return;
    }
    remove(StagedPath);
    printf("File unstaged successfully!\n");
    return;
}

void ResetUndo(char *ProjPath)
{
    Increase_Decrease_addID(ProjPath, '-');

    int ID = Get_addID(ProjPath);
    char stagePath[PATH_MAX];
    sprintf(stagePath, "%s/.neogit/.records/%d", ProjPath, ID);
    FILE *f = fopen("/home/pardis/tmp.sh", "w");
    fprintf(f, "rm -r %s", stagePath);
    fclose(f);
    chmod("/home/pardis/tmp.sh", 0x777);
    system("/home/pardis/tmp.sh");
    remove("/home/pardis/tmp.sh");

    sprintf(stagePath, "%s/.neogit/.staged/", ProjPath);
    f = fopen("/home/pardis/tmp.sh", "w");
    fprintf(f, "rm -r %s", stagePath);
    fclose(f);
    chmod("/home/pardis/tmp.sh", 0x777);
    system("/home/parids/tmp.sh");
    remove("/home/pardis/tmp.sh");

    sprintf(stagePath, "%s/.neogit/.records/%d", ProjPath, --ID);
    char projNeo[PATH_MAX];
    sprintf(projNeo, "%s/.neogit", ProjPath);
    f = fopen("/home/pardis/tmp.sh", "w");
    fprintf(f, "cp -r %s %s", stagePath, projNeo);
    fclose(f);
    chmod("/home/pardis/tmp.sh", 0x777);
    system("/home/pardis/tmp.sh");
    remove("/home/pardis/tmp.sh");

    sprintf(stagePath, "%s/.neogit/%d", ProjPath, ID);
    sprintf(projNeo, "%s/.neogit/.staged", ProjPath);
    rename(stagePath, projNeo);
    printf("Reset undone successfully!\n");
    return;
}

// 1 if first one is newer than second one, 2 if reverse
int compareFileTimes(const char *file1, const char *file2)
{
    struct stat stat1, stat2;

    if (stat(file1, &stat1) != 0)
    {
        perror("Error getting information about file 1"); // replace with printf
        return -1;
    }

    if (stat(file2, &stat2) != 0)
    {
        perror("Error getting information about file 2"); // replace with printf
        return -1;
    }
    if (stat1.st_mtime > stat2.st_mtime)
        return 1;
    else if (stat1.st_mtime < stat2.st_mtime)
        return 2;
    else
        return 0;
}

void Commit(char *ProjPath, char *message)
{
    char StagePath[PATH_MAX];
    strcpy(StagePath, ProjPath);
    strcat(StagePath, "/.neogit/.staged");
    int count = 0;
    DIR *StageDir = opendir(StagePath);
    struct dirent *entry;
    while ((entry = readdir(StageDir)) != NULL)
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            count++;
    if (count == 0 || count == -1)
    {
        printf("Nothing has been staged since last commit!\n");
        closedir(StageDir);
        return;
    }
    closedir(StageDir);
    int ID = Get_commitID(ProjPath);
    Increase_Decrease_commitID(ProjPath, '+');

    char commitPath[PATH_MAX];
    sprintf(commitPath, "%s/.neogit/commits", ProjPath);

    FILE *f = fopen("/home/pardis/tmp.sh", "w");
    fprintf(f, "cp -r %s %s", StagePath, commitPath);
    fclose(f);
    chmod("/home/pardis/tmp.sh", 0x777);
    system("/home/pardis/tmp.sh");
    remove("/home/pardis/tmp.sh");

    f = fopen("/home/pardis/tmp.sh", "w");
    fprintf(f, "rm -r %s", StagePath);
    fclose(f);
    chmod("/home/pardis/tmp.sh", 0x777);
    system("/home/pardis/tmp.sh");
    remove("/home/pardis/tmp.sh");

    CreateDir(StagePath);

    sprintf(StagePath, "%s/.neogit/commits/.staged", ProjPath);
    sprintf(commitPath, "%s/.neogit/commits/%d", ProjPath, ID);
    rename(StagePath, commitPath);

    strcat(commitPath, "/info.txt");
    f = fopen(commitPath, "w");
    char Username[100], Useremail[100], branch[100];
    char globalConfig[PATH_MAX], localConfig[PATH_MAX];
    sprintf(globalConfig, "/home/pardis/.neogitconfig/info.txt");
    sprintf(localConfig, "%s/.neogit/config.txt", ProjPath);
    FILE *config;
    switch (compareFileTimes(globalConfig, localConfig))
    {
    case 1:
        config = fopen(globalConfig, "r");
        break;
    case 2:
        config = fopen(localConfig, "r");
        break;
    }

    fgets(Username, sizeof(Username), config);
    fgets(Useremail, sizeof(Useremail), config);
    char *tokName = strtok(Username, "=");
    tokName = strtok(NULL, "=");

    char *tokEmail = strtok(Useremail, "=");
    tokEmail = strtok(NULL, "=");
    fclose(config);

    char Head[PATH_MAX];
    sprintf(Head, "%s/.neogit/head.txt", ProjPath);
    config = fopen(Head, "r");
    fgets(branch, sizeof(branch), config);
    fclose(config);
    fprintf(f, "Username =%sUseremail =%sBranch =%sMessage =%s", tokName, tokEmail, branch, message);
    fclose(f);
    char ptr[PATH_MAX];
    sprintf(ptr, "%s/.neogit/commitPointer.txt", ProjPath);
    f = fopen(ptr, "w");
    fprintf(f, "%d\n", ID);
    fclose(f);

    if (ID > 10000)
    {
        char Dest[PATH_MAX];
        sprintf(Dest, "%s/.neogit/commits/%d", ProjPath, ID);
        char Src[PATH_MAX];
        sprintf(Src, "%s/.neogit/commits/%d", ProjPath, ID - 1);
        CreateLn(ProjPath, Src, Dest);
    }
    sprintf(ptr, "%s/.neogit/commits/%d", ProjPath, ID);
    struct stat folder;
    stat(ptr, &folder);
    time_t crt = folder.st_ctime;
    printf("Committed successfully in time" _SGR_BLUEF " %s"_SGR_RESET _SGR_REDF "ID = %d" _SGR_RESET "\nMessage =" _SGR_YELLOWF " \"%s\"\n" _SGR_RESET, ctime(&crt), ID, message);

    return;
}

void CreateLn(char ProjPath[], char Src[], char Dest[])
{
    DIR *cur_dir = opendir(Src);
    struct dirent *entry;
    char folder_name[PATH_MAX];
    while ((entry = readdir(cur_dir)) != NULL)
    {
        strcpy(folder_name, entry->d_name);
        if (entry->d_type == DT_DIR && strncmp(folder_name, ".", 1))
        {
            strcat(Src, "/");
            strcat(Src, folder_name);
            strcat(Dest, "/");
            strcat(Dest, folder_name);
            CreateLn(ProjPath, Src, Dest);
        }

        else if (entry->d_type != DT_DIR)
        {
            char WDPath[PATH_MAX];
            sprintf(WDPath, "%s%s", ProjPath, Src + strlen(ProjPath) + 22);
            strcat(WDPath, "/");
            strcat(WDPath, folder_name);

            char CMPath[PATH_MAX];
            strcpy(CMPath, Src);
            strcat(CMPath, "/");
            strcat(CMPath, folder_name);

            strcat(Dest, "/");
            strcat(Dest, folder_name);
            if (IsModified(CMPath, WDPath))
            {
                FILE *p = fopen("/home/pardis/tmp.sh", "w");
                fprintf(p, "cp -r %s %s", CMPath, Dest);
                fclose(p);
                chmod("/home/pardis/tmp.sh", 0x777);
                system("/home/pardis/tmp.sh");
                remove("/home/pardis/tmp.sh");
            }
            while (Dest[strlen(Dest) - 1] != '/')
                Dest[strlen(Dest) - 1] = '\0';
            Dest[strlen(Dest) - 1] = '\0';
        }
    }
    while (Src[strlen(Src) - 1] != '/')
        Src[strlen(Src) - 1] = '\0';
    Src[strlen(Src) - 1] = '\0';
    while (Dest[strlen(Dest) - 1] != '/')
        Dest[strlen(Dest) - 1] = '\0';
    Dest[strlen(Dest) - 1] = '\0';
    closedir(cur_dir);
    return;
}

void SetShortcut(char *message, char *shortcut, char *ProjPath)
{
    char shcPath[PATH_MAX];
    sprintf(shcPath, "%s/.neogit/shortcuts/%s", ProjPath, shortcut);
    FILE *f = fopen(shcPath, "w");
    fputs(message, f);
    fclose(f);
    return;
}

void Log(char *ProjPath, int n)
{
    char commits[PATH_MAX];
    sprintf(commits, "%s/.neogit/commits", ProjPath);
    int ID = Get_commitID(ProjPath) - 1;
    for (int i = ID; i >= ID - n + 1; i--)
    {
        char adrs[PATH_MAX];
        sprintf(adrs, "%s/%d", commits, i);
        int num = 0;
        char curAddress[PATH_MAX];
        strcpy(curAddress, adrs);
        CountFiles(curAddress, &num);
        struct stat folder;
        stat(adrs, &folder);
        time_t crt = folder.st_ctime;
        strcat(adrs, "/info.txt");
        FILE *f = fopen(adrs, "r");
        char line[1024];
        while (fgets(line, sizeof(line), f) != NULL)
            printf("%s", line);
        fclose(f);
        printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
        printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
    }
}

void CountFiles(char *curAddress, int *num)
{
    DIR *cur_dir = opendir(curAddress);
    struct dirent *entry;
    while ((entry = readdir(cur_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            strcat(curAddress, "/");
            strcat(curAddress, entry->d_name);
            CountFiles(curAddress, num);
        }
        else if (entry->d_type != DT_DIR)
            (*num)++;
    }
    while (curAddress[strlen(curAddress) - 1] != '/')
        curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cur_dir);
    return;
}

void LogExtra(char *ProjPath, char *TYPEt, char *WORDt)
{
    char commits[PATH_MAX];
    sprintf(commits, "%s/.neogit/commits", ProjPath);
    int ID = Get_commitID(ProjPath) - 1;
    for (int i = ID; i >= 10000; i--)
    {
        char adrs[PATH_MAX];
        sprintf(adrs, "%s/%d", commits, i);

        int num = 0;
        char curAddress[PATH_MAX];
        strcpy(curAddress, adrs);
        CountFiles(curAddress, &num);
        struct stat folder;
        stat(adrs, &folder);
        time_t crt = folder.st_ctime;
        strcat(adrs, "/info.txt");
        FILE *f = fopen(adrs, "r");
        char line[1024];

        while (fgets(line, sizeof(line), f) != NULL)
        {
            if (!strncmp(line, TYPEt, strlen(TYPEt)))
                if (!strncmp(line + strlen(TYPEt) + 2, WORDt, strlen(WORDt)))
                {
                    fclose(f);
                    FILE *g = fopen(adrs, "r");
                    char tmp[1024];
                    while (fgets(tmp, sizeof(tmp), g) != NULL)
                        printf("%s", tmp);
                    fclose(g);
                    printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
                    printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
                    break;
                }
        }
    }
}

void LogSearch(char *ProjPath, char *TYPEt, char *WORDt)
{
    char commits[PATH_MAX];
    sprintf(commits, "%s/.neogit/commits", ProjPath);
    int ID = Get_commitID(ProjPath) - 1;
    for (int i = ID; i >= 10000; i--)
    {
        char adrs[PATH_MAX];
        sprintf(adrs, "%s/%d", commits, i);

        int num = 0;
        char curAddress[PATH_MAX];
        strcpy(curAddress, adrs);
        CountFiles(curAddress, &num);
        struct stat folder;
        stat(adrs, &folder);
        time_t crt = folder.st_ctime;
        strcat(adrs, "/info.txt");
        FILE *f = fopen(adrs, "r");
        char line[1024];

        while (fgets(line, sizeof(line), f) != NULL)
        {
            if (!strncmp(line, TYPEt, strlen(TYPEt)))
                if (strstr(line + strlen(TYPEt) + 2, WORDt) != NULL)
                {
                    fclose(f);
                    FILE *g = fopen(adrs, "r");
                    char tmp[1024];
                    while (fgets(tmp, sizeof(tmp), g) != NULL)
                        printf("%s", tmp);
                    fclose(g);
                    printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
                    printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
                    break;
                }
        }
    }
}

void LogTime(char *ProjPath, char *time, char sign)
{
    char commits[PATH_MAX];
    sprintf(commits, "%s/.neogit/commits", ProjPath);
    int ID = Get_commitID(ProjPath) - 1;
    for (int i = ID; i >= 10000; i--)
    {
        char adrs[PATH_MAX];
        sprintf(adrs, "%s/%d", commits, i);

        if (sign == '-' && CompareCommitTime(adrs, time) == 1) // before
        {
            int num = 0;
            char curAddress[PATH_MAX];
            strcpy(curAddress, adrs);
            CountFiles(curAddress, &num);
            struct stat folder;
            stat(adrs, &folder);
            time_t crt = folder.st_ctime;
            strcat(adrs, "/info.txt");
            FILE *f = fopen(adrs, "r");
            char line[1024];
            while (fgets(line, sizeof(line), f) != NULL)
                printf("%s", line);
            fclose(f);
            printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
            printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
        }
        else if (sign == '+' && CompareCommitTime(adrs, time) == -1) // since
        {
            int num = 0;
            char curAddress[PATH_MAX];
            strcpy(curAddress, adrs);
            CountFiles(curAddress, &num);
            struct stat folder;
            stat(adrs, &folder);
            time_t crt = folder.st_ctime;
            strcat(adrs, "/info.txt");
            FILE *f = fopen(adrs, "r");
            char line[1024];
            while (fgets(line, sizeof(line), f) != NULL)
                printf("%s", line);
            fclose(f);
            printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
            printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
        }
    }
}

// 1 if file is before time, -1 if reverse
int CompareCommitTime(const char *filename, const char *time)
{
    struct stat fileStat;
    if (stat(filename, &fileStat) == -1)
    {
        perror("Error getting file information");
        return -1;
    }
    time_t fileTime = fileStat.st_ctime;
    struct tm inputTime;
    if (strptime(time, "%Y/%m/%d %H:%M:%S", &inputTime) == NULL)
    {
        perror("Error parsing input timestamp");
        return -2;
    }
    time_t inputTimeValue = mktime(&inputTime);
    if (inputTimeValue < fileTime)
        return -1;
    else if (inputTimeValue > fileTime)
        return 1;
    return 0;
}

void Status(char *ProjPath)
{
    int ID = Get_commitID(ProjPath) - 1;
    char commitPath[PATH_MAX];
    sprintf(commitPath, "%s/.neogit/commits/%d", ProjPath, ID);
    WalkOnCommit(ProjPath, commitPath);
    char curAddress[PATH_MAX];
    strcpy(curAddress, ProjPath);
    WalkOnWD(ProjPath, curAddress);
}

void WalkOnCommit(char *ProjPath, char *curAddress)
{
    DIR *cm = opendir(curAddress);
    struct dirent *entry;
    while ((entry = readdir(cm)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit"))
        {
            strcat(curAddress, "/");
            strcat(curAddress, entry->d_name);
            WalkOnCommit(ProjPath, curAddress);
        }
        else if (entry->d_type != DT_DIR && strcmp(entry->d_name, "info.txt"))
        {
            strcat(curAddress, "/");
            strcat(curAddress, entry->d_name);
            char SAPath[PATH_MAX];
            sprintf(SAPath, "%s/.neogit/.staged%s", ProjPath, curAddress + strlen(ProjPath) + 22);
            char WDPath[PATH_MAX];
            sprintf(WDPath, "%s%s", ProjPath, curAddress + strlen(ProjPath) + 22);
            if (access(WDPath, F_OK) == 0)
            {
                if (!CompareMode(WDPath, curAddress))
                    printf(_SGR_ITALIC "%s\t\t" _SGR_NOITALIC _SGR_BOLD _SGR_GREENF "T" _SGR_RESET "\n", WDPath + strlen(ProjPath));
                if (!IsModified(curAddress, WDPath))
                {
                    if (access(SAPath, F_OK) == 0)
                        printf("%s\t\t" _SGR_BOLD _SGR_MAGENF "+M" _SGR_RESET "\n", WDPath + strlen(ProjPath));
                    else
                        printf("%s\t\t" _SGR_BOLD _SGR_MAGENF "-M" _SGR_RESET "\n", WDPath + strlen(ProjPath));
                }
            }
            else
            {
                if (access(SAPath, F_OK) == 0)
                    printf("%s\t\t" _SGR_BOLD _SGR_REDF "+D" _SGR_RESET "\n", WDPath + strlen(ProjPath));
                else
                    printf("%s\t\t" _SGR_BOLD _SGR_REDF "-D" _SGR_RESET "\n", WDPath + strlen(ProjPath));
            }

            while (curAddress[strlen(curAddress) - 1] != '/')
                curAddress[strlen(curAddress) - 1] = '\0';
            curAddress[strlen(curAddress) - 1] = '\0';
        }
    }
    while (curAddress[strlen(curAddress) - 1] != '/')
        curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cm);
    return;
}

void WalkOnWD(char *ProjPath, char *curAddress)
{
    DIR *cm = opendir(curAddress);
    struct dirent *entry;
    while ((entry = readdir(cm)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit"))
        {
            strcat(curAddress, "/");
            strcat(curAddress, entry->d_name);
            WalkOnCommit(ProjPath, curAddress);
        }
        else if (entry->d_type != DT_DIR)
        {
            strcat(curAddress, "/");
            strcat(curAddress, entry->d_name);
            char SAPath[PATH_MAX];

            sprintf(SAPath, "%s/.neogit/.staged%s", ProjPath, curAddress + strlen(ProjPath));
            char CMPath[PATH_MAX];
            int ID = Get_commitID(ProjPath) - 1;
            sprintf(CMPath, "%s/.neogit/commits/%d%s", ProjPath, ID, curAddress + strlen(ProjPath));
            if (access(CMPath, F_OK) != 0) // A
            {
                if (access(SAPath, F_OK) == 0)
                    printf("%s\t\t" _SGR_BOLD _SGR_YELLOWF "+A" _SGR_RESET "\n", curAddress + strlen(ProjPath));
                else
                    printf("%s\t\t" _SGR_BOLD _SGR_YELLOWF "-A" _SGR_RESET "\n", curAddress + strlen(ProjPath));
            }
            while (curAddress[strlen(curAddress) - 1] != '/')
                curAddress[strlen(curAddress) - 1] = '\0';
            curAddress[strlen(curAddress) - 1] = '\0';
        }
    }
    while (curAddress[strlen(curAddress) - 1] != '/')
        curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cm);
    return;
}

void BranchCreate(char *name) { // when committing change head's commit id in branch.txt
    char ProjPath[1024];
    getcwd(ProjPath, sizeof(ProjPath));
    CHECK_IF_REPO(ProjPath);
    char HEADBAddress[1024];
    sprintf(HEADBAddress, "%s/.neogit/head.txt", ProjPath);
    FILE *head = fopen(HEADBAddress, "r");
    char HEAD[1000];
    fgets(HEAD, 1000, head);
    fclose(head);
    char HEADCAddress[1024];
    sprintf(HEADCAddress, "%s/.neogit/branch.txt", ProjPath);
    FILE *branch = fopen(HEADCAddress, "r");
    char COMMITID[1000];
    char *ID;
    while (fgets(COMMITID, 1000, branch) != NULL) {
        strtok(COMMITID, "_");
        if (!strcmp(COMMITID, HEAD)) {
            strtok(NULL, "_");
            ID = strtok(NULL, "_");
            break;
        }
    }
    fclose(branch);
    // HEAD -> branchName , ID -> branch's HEAD commit ID
    branch = fopen(HEADCAddress, "r");
    while (fgets(COMMITID, 1000, branch) != NULL) {
        strtok(COMMITID, "_");
        if (!strcmp(COMMITID, name)) {
            printf("Branch Already Exists!\n");
            return;
        }
    }
    fclose(branch);
    branch = fopen(HEADCAddress, "a");
    char newBranch[1000];
    sprintf(newBranch, "%s_%s_0\n", name, ID);
    fputs(newBranch, branch);
    close(branch);
    printf("New Branch %s successfully created!\n", name);
    return;
}

void BranchList () {
    char ProjPath[1024];
    getcwd(ProjPath, sizeof(ProjPath));
    CHECK_IF_REPO(ProjPath);
    char branchPath[1024];
    sprintf(branchPath, "%s/.neogit/branch.txt", ProjPath);
    FILE *branch = fopen(branchPath, "r");
    char name[1000];
    char HEADBAddress[1024];
    sprintf(HEADBAddress, "%s/.neogit/head.txt", ProjPath);
    FILE *head = fopen(HEADBAddress, "r");
    char HEAD[1000];
    fgets(HEAD, 1000, head);
    fclose(head);
    while (fgets(name, 1000, branch) != NULL) {
        strtok(name, "_");
        printf("\t%s", name);
        if (!strcmp(name, HEAD)) printf("\t--> HEAD");
        printf("\n");
    }
    return;
}

int WalkOnWDForCheckout(char *ProjPath, char *curAddress, char *ID)
{
    DIR *cm = opendir(curAddress);
    struct dirent *entry;
    while ((entry = readdir(cm)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name,".neogit"))
        {
            strcat(curAddress, "/");
            strcat(curAddress, entry->d_name);
            if (WalkOnWDForCheckout(ProjPath, curAddress) == 0) return 0;
        }
        else if (entry->d_type != DT_DIR)
        {
            strcat(curAddress, "/");
            strcat(curAddress, entry->d_name);
            char CMPath[PATH_MAX];

            sprintf(CMPath, "%s/.neogit/commits/%s%s", ProjPath, ID, curAddress + strlen(ProjPath));
            if (access(CMPath, F_OK) == 0)
                if (IsModified(CMPath, curAddress)) {
                    closedir(cm);
                    return 0;
                }
            else {
                closedir(cm);
                return 0;
            }
            while (curAddress[strlen(curAddress) - 1] != '/')
                curAddress[strlen(curAddress) - 1] = '\0';
            curAddress[strlen(curAddress) - 1] = '\0';
        }
    }
    while (curAddress[strlen(curAddress) - 1] != '/')
        curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cm);
    return 1;
}

void Checkout(char *where) { // branch names mustn't have numbers within
    char ProjPath[1024];
    getcwd(ProjPath, sizeof(ProjPath));
    CHECK_IF_REPO(ProjPath);
    // Having Changes from last commit in Working Directory?
    char HEADBAddress[1024];
    sprintf(HEADBAddress, "%s/.neogit/head.txt", ProjPath);
    FILE *head = fopen(HEADBAddress, "r");
    char HEAD[1000];
    fgets(HEAD, 1000, head);
    fclose(head);
    char HEADCAddress[1024];
    sprintf(HEADCAddress, "%s/.neogit/branch.txt", ProjPath);
    FILE *branch = fopen(HEADCAddress, "r");
    char COMMITID[1000];
    char *ID;
    while (fgets(COMMITID, 1000, branch) != NULL) {
        strtok(COMMITID, "_");
        if (!strcmp(COMMITID, HEAD)) {
            strtok(NULL, "_");
            ID = strtok(NULL, "_");
            break;
        }
    }
    fclose(branch);
    char here[1024];
    strcpy(here, ProjPath);
    bool IsEqual = WalkOnWDForCheckout(ProjPath, here, ID);

    if (!IsEqual) {
        printf("There are Modified/Untracked files in working directory so you can't checkout. Commit or Stash and try again!\n");
        return;
    }

    if (where[0] < '0' || where[0] > '9') {
        head = fopen(HEADBAddress, "w");
        fputs(where, head);
        fclose(head);
    }
    DIR *WorDir = opendir(ProjPath);
    struct dirent *walking;
    while ((walking = readdir(WorDir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            char add[1024];
            sprintf(add, "%s/%s", ProjPath, walking->d_name);
            FILE *f = fopen("/home/pardis/tmp.sh", "w");
            fprintf(f, "rm -r %s", add);
            fclose(f);
            chmod("/home/pardis/tmp.sh", 0x777);
            system("/home/pardis/tmp.sh");
            remove("/home/pardis/tmp.sh");
        }
    }
    closedir(WorDir);
    char commitAddress[1024];
    sprintf(commitAddress, "%s/.neogit/commits/%s", ProjPath, where);
    DIR *ComDir = opendir(commitAddress);
    while ((walking = readdir(ComDir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt")) {
            char add[1024];
            sprintf(add, "%s/%s", commitAddress, walking->d_name);
            FILE *f = fopen("/home/pardis/tmp.sh", "w");
            fprintf(f, "cp -r %s %s", add, ProjPath);
            fclose(f);
            chmod("/home/pardis/tmp.sh", 0x777);
            system("/home/pardis/tmp.sh");
            remove("/home/pardis/tmp.sh");
        }
    }
    closedir(ComDir);
    char CurIDPath[1024];
    sprintf(CurIDPath, "%s/.neogit/current_IDs.txt", ProjPath);
    FILE *CurID = fopen(CurIDPath, "r");
    char addID[10], commitID[10];
    fgets(addID, 10, CurID);
    fgets(commitAddress, 10, CurID);
    fclose(CurID);
    CurID = fopen(CurIDPath, "w");
    fputs(addID, CurID);
    fputs(commitID, CurID);
    fputs(where, CurID);
    fclose(CurID);
    return;
}

void CheckoutHead(char *where) {
    char ProjPath[1024];
    getcwd(ProjPath, sizeof(ProjPath));
    CHECK_IF_REPO(ProjPath);
    char HEADBAddress[1024];
    sprintf(HEADBAddress, "%s/.neogit/head.txt", ProjPath);
    FILE *head = fopen(HEADBAddress, "r");
    char HEAD[1000];
    fgets(HEAD, 1000, head);
    fclose(head);
    char HEADCAddress[1024];
    sprintf(HEADCAddress, "%s/.neogit/branch.txt", ProjPath);
    FILE *branch = fopen(HEADCAddress, "r");
    char COMMITID[1000];
    char *ID;
    while (fgets(COMMITID, 1000, branch) != NULL) {
        strtok(COMMITID, "_");
        if (!strcmp(COMMITID, HEAD)) {
            strtok(NULL, "_");
            ID = strtok(NULL, "_");
            break;
        }
    }
    fclose(branch);
    if (strlen(where) != 4) {
        int n = 0;
        for (int i = 5; i < strlen(where); i++) {
            n *= 10;
            n += where[i] - '0';
        }
        char commits[PATH_MAX];
        sprintf(commits, "%s/.neogit/commits", ProjPath);
        int comID = Get_commitID(ProjPath) - 1;
        char tmpHead;
        sprintf(tmpHead, "%s\n", HEAD);
        int i;
        for (i = comID; i >= 10000; i--)
        {
            if (n == 0) break;
            char adrs[PATH_MAX];
            sprintf(adrs, "%s/%d/info.txt", commits, i);
            FILE *info = fopen(adrs, "r");
            char line[1000];
            fgets(line, sizeof(line), info);
            fgets(line, sizeof(line), info);
            fgets(line, sizeof(line), info);
            if (!strcmp(line + 8, tmpHead)) // Branch =''
                n--;
            fclose(info);
            
        }
        sprintf(ID, "%d", i);
    }

    DIR *WorDir = opendir(ProjPath);
    struct dirent *walking;
    while ((walking = readdir(WorDir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            char add[1024];
            sprintf(add, "%s/%s", ProjPath, walking->d_name);
            FILE *f = fopen("/home/pardis/tmp.sh", "w");
            fprintf(f, "rm -r %s", add);
            fclose(f);
            chmod("/home/pardis/tmp.sh", 0x777);
            system("/home/pardis/tmp.sh");
            remove("/home/pardis/tmp.sh");
        }
    }
    closedir(WorDir);
    char commitAddress[1024];
    sprintf(commitAddress, "%s/.neogit/commits/%s", ProjPath, ID);
    DIR *ComDir = opendir(commitAddress);
    while ((walking = readdir(ComDir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt")) {
            char add[1024];
            sprintf(add, "%s/%s", commitAddress, walking->d_name);
            FILE *f = fopen("/home/pardis/tmp.sh", "w");
            fprintf(f, "cp -r %s %s", add, ProjPath);
            fclose(f);
            chmod("/home/pardis/tmp.sh", 0x777);
            system("/home/pardis/tmp.sh");
            remove("/home/pardis/tmp.sh");
        }
    }
    closedir(ComDir);

    return;
}

// alias funcs start

int IsValid(char *command)
{
    if (!strncmp(command, "neogit revert ", 14))
        return 1;
    if (!strncmp(command, "neogit tag", 10))
        return 1;
    if (!strcmp(command, "neogit tree"))
        return 1;
    if (!strncmp(command, "neogit stash push", 17))
        return 1;
    if (!strncmp(command, "neogit stash show ", 18))
        return 1;
    if (!strncmp(command, "neogit stash branch ", 20))
        return 1;
    if (!strcmp(command, "neogit stash drop"))
        return 1;
    if (!strcmp(command, "neogit stash clear"))
        return 1;
    if (!strcmp(command, "neogit stash pop"))
        return 1;
    if (!strcmp(command, "neogit stash list"))
        return 1;
    if (!strcmp(command, "neogit pre-commit hooks list"))
        return 1;
    if (!strcmp(command, "neogit pre-commit applied hooks"))
        return 1;
    if (!strncmp(command, "neogit pre-commit add hook ", 27))
        return 1;
    if (!strncmp(command, "neogit pre-commit remove hook ", 30))
        return 1;
    if (!strcmp(command, "neogit pre-commit"))
        return 1;
    if (!strcmp(command, "neogit pre-commit -u"))
        return 1;
    if (!strncmp(command, "neogit pre-commit -f ", 21))
        return 1;
    if (!strncmp(command, "neogit grep -f ", 15))
        return 1;
    if (!strncmp(command, "neogit diff -f ", 15))
        return 1;
    if (!strncmp(command, "neogit diff -c ", 15))
        return 1;
    if (!strncmp(command, "neogit merge -b ", 16))
        return 1;
    if (!strncmp(command, "neogit config -global user.name ", 32))
        return 1;
    if (!strncmp(command, "neogit config -global user.email ", 33))
        return 1;
    if (!strncmp(command, "neogit config user.name ", 24))
        return 1;
    if (!strncmp(command, "neogit config user.email ", 25))
        return 1;
    if (!strncmp(command, "neogit config alias.", 20))
        return 1;
    if (!strncmp(command, "neogit config -global alias.", 29))
        return 1;
    if (!strcmp(command, "neogit init"))
        return 1;
    if (!strncmp(command, "neogit add ", 11))
        return 1;
    if (!strncmp(command, "neogit reset ", 13))
        return 1;
    if (!strcmp(command, "neogit status"))
        return 1;
    if (!strncmp(command, "neogit commit -m ", 17))
        return 1;
    if (!strncmp(command, "neogit commit -s ", 17))
        return 1;
    if (!strncmp(command, "neogit set -m ", 14))
        return 1;
    if (!strncmp(command, "neogit replace =m ", 18))
        return 1;
    if (!strncmp(command, "neogit remove -s ", 17))
        return 1;
    if (!strncmp(command, "neogit log ", 11))
        return 1;
    if (!strncmp(command, "neogit checkout ", 16))
        return 1;
    if (!strncmp(command, "neogit branch", 13))
        return 1;
    return 0;
}

void IsAlias(char *mammad, char *ProjPath)
{
    char *homePath = getenv("HOME");
    char gtmp[PATH_MAX];
    snprintf(gtmp, sizeof(gtmp), "%s/.neogitconfig/alias", homePath);
    DIR *globDir = opendir(gtmp);
    char globalAD[PATH_MAX];
    struct dirent *entry;
    while ((entry = readdir(globDir)) != NULL)
        if (!strcmp(entry->d_name, mammad))
            sprintf(globalAD, "%s/%s", gtmp, entry->d_name);
    closedir(globDir);
    char ltmp[PATH_MAX];
    snprintf(ltmp, sizeof(ltmp), "%s/.neogit/alias", ProjPath);
    DIR *locDir = opendir(ltmp);
    char locAD[PATH_MAX];
    struct dirent *walking;
    while ((walking = readdir(locDir)) != NULL)
        if (!strcmp(walking->d_name, mammad))
            sprintf(locAD, "%s/%s", ltmp, walking->d_name);
    closedir(locDir);
    FILE *f;
    if (globalAD && locAD)
    {
        switch (compareFileTimes(globalAD, locAD))
        {
        case 1:
            f = fopen(globalAD, "r");
            break;
        case 2:
            f = fopen(locAD, "r");
            break;
        }
    }
    else if (globalAD)
        f = fopen(globalAD, "r");
    else if (locAD)
        f = fopen(locAD, "r");
    else
        return;
    char line[1024];
    fgets(line, sizeof(line), f);
    fclose(f);
    f = fopen("/home/saraghazavi/tmp.sh", "w");
    fprintf(f, "%s", line);
    fclose(f);
    chmod("/home/saraghazavi/tmp.sh", 0x777);
    system("/home/saraghazavi/tmp.sh");
    remove("/home/saraghazavi/tmp.sh");
}