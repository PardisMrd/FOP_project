#include "header.h"

bool checkHook = true;

int Find_Repo(char *path) { // checks wether there exists a repository in current or parent directories
    if (!path || !(*path)) return 0;
    DIR *dir = opendir(path);
    struct dirent *entry;
    if (dir == NULL) return 0;

    while ((entry = readdir(dir)) != NULL) if (!strcmp(entry->d_name, ".neogit")) return 1;
    closedir(dir);
    path[strlen(path)] = '\0';

    char *token = strrchr(path, '/');
    if (token != NULL)
        *token = '\0';

    return Find_Repo(path);
}

void Create(char *path, char *info, int mode) {
    // mode 0 is for creating directory and mode 1 is for creating file
    if (mode) {
        for (int i = 1; i < strlen(path); i++) {
            if (path[i] == '/') {
                path[i] = '\0';
                if (access(path, F_OK) != 0) mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
                path[i] = '/';
            }
        }
        FILE *file = fopen(path, "w");
        if (info != NULL) fputs(info, file);
        fclose(file);
    }
    else if (access(path, F_OK) != 0) {
        for (int i = 1; i < strlen(path); i++) {
            if (path[i] == '/') {
                path[i] = '\0';
                if (access(path, F_OK) != 0) mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
                path[i] = '/';
            }
        }
        mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    }
    return;
}

int IsModified(char *path1, char *path2) { // checks wether a file in WD is different with the exact one in CM
    int character1, character2;
    FILE *file1 = fopen(path1, "rb");
    if (file1 == NULL) return 0;
    FILE *file2 = fopen(path2, "rb");
    if (file2 == NULL) {
        fclose(file1);
        return 0;
    }

    while ((character1 = fgetc(file1)) != EOF && (character2 = fgetc(file2)) != EOF)
        if (character1 != character2) {
            fclose(file1);
            fclose(file2);
            return 0;
        }
    character2 = fgetc(file2);
    if (character1 != character2) {
        fclose(file1);
        fclose(file2);
        return 0;
    }
    fclose(file1);
    fclose(file2);
    return 1;
}

char *Get_HEAD_ID (char *ProjPath) { // returns the head commit id
    char headPath[MAX_PATH]; sprintf(headPath, "%s/.neogit/head.txt", ProjPath);
    FILE *headfile = fopen(headPath, "r");
    char HEAD[100];
    fgets(HEAD, sizeof(HEAD), headfile);
    if (HEAD[strlen(HEAD) - 1] == '\n') HEAD[strlen(HEAD) - 1] = '\0';
    strcat(HEAD, "_");
    fclose(headfile);
    char branchPath[MAX_PATH]; sprintf(branchPath, "%s/.neogit/branch.txt", ProjPath);
    FILE *branchfile = fopen(branchPath, "r");
    char b_line[1000];
    char *headID;
    while (1) {
        fgets(b_line, sizeof(b_line), branchfile);
        if (!strncmp(b_line, HEAD, strlen(HEAD))) {
            strtok(b_line, "_"); strtok(NULL, "_");
            headID = strtok(NULL, "\n");
            break;
        }
    }
    fclose(branchfile);
    // without \n
    return headID;
}

char *Get_Current_Commit_ID (char *ProjPath) { // returns id of the commit which we're on
    char CurIDPath[MAX_PATH]; sprintf(CurIDPath, "%s/.neogit/current_IDs.txt", ProjPath);
    FILE *CurIDfile = fopen(CurIDPath, "r");
    char *commitID = (char *) malloc (10 * sizeof(char));
    fgets(commitID, sizeof(commitID), CurIDfile);
    fgets(commitID, sizeof(commitID), CurIDfile);
    fgets(commitID, sizeof(commitID), CurIDfile);
    if (commitID[strlen(commitID) - 1] == '\n') commitID[strlen(commitID) - 1] = '\0';
    fclose(CurIDfile);
    // without \n
    return commitID;
}

int Get_addID(char *ProjPath) {
    char filePath[MAX_PATH]; strcpy(filePath, ProjPath); strcat(filePath, "/.neogit/current_IDs.txt");
    int toPass_ID = 0;
    char *addID = (char *) malloc (10 * sizeof(char));
    FILE *current_IDs = fopen(filePath, "r");
    fgets(addID, sizeof(addID), current_IDs);
    for (int i = 0; i < strlen(addID) - 1; i++) {
        toPass_ID *= 10;
        toPass_ID += (addID[i] - '0');
    }
    fclose(current_IDs);
    return toPass_ID;
}

int Get_commitID(char *ProjPath) {
    char filePath[MAX_PATH]; strcpy(filePath, ProjPath); strcat(filePath, "/.neogit/current_IDs.txt");
    int toPass_ID = 0;
    char *commitID = (char *) malloc (10 * sizeof(char));
    FILE *current_IDs = fopen(filePath, "r");
    fgets(commitID, sizeof(commitID), current_IDs);
    fgets(commitID, sizeof(commitID), current_IDs);
    for (int i = 0; i < strlen(commitID) - 1; i++) {
        toPass_ID *= 10;
        toPass_ID += (commitID[i] - '0');
    }
    fclose(current_IDs);
    return toPass_ID;
}

void Update_Current_Commit_ID (char *ProjPath, char *newID) {
    char commitIDPath[MAX_PATH];
    sprintf(commitIDPath, "%s/.neogit/current_IDs.txt", ProjPath);
    FILE *CurIDfile = fopen(commitIDPath, "r");
    char addID[10], commitID[10];
    fgets(addID, sizeof(addID), CurIDfile);
    fgets(commitID, sizeof(commitID), CurIDfile);
    fclose(CurIDfile);
    CurIDfile = fopen(commitIDPath, "w");
    fputs(addID, CurIDfile);
    fputs(commitID, CurIDfile);
    fprintf(CurIDfile, "%s\n", newID);
    fclose(CurIDfile);
    // appends a \n at the end
    return;
}

void Update_Branch(char *ProjPath, char *branch, int ID) {
    char findBranch[1000]; strncpy(findBranch, branch, strlen(branch)); strcat(findBranch, "_");
    char branchPath[MAX_PATH]; sprintf(branchPath, "%s/.neogit/branch.txt", ProjPath);
    FILE *bTXT = fopen(branchPath, "r");
    char line[100][1000];
    int len = -1;
    while (fgets(line[++len], sizeof(line[len]), bTXT) != NULL);
    fclose(bTXT);
    bTXT = fopen(branchPath, "w");
    for (int i = 0; i < len; i++) {
        if (!strncmp(line[i], findBranch, strlen(findBranch))) {
            int tok = strlen(line[i]) - 1;
            while (line[i][tok] != '_') tok--;
            sprintf(line[i] + tok, "_%d\n", ID);
        }
        fputs(line[i], bTXT);
    }
    fclose(bTXT);
    return;
}

void Increase_Decrease_ID (char *ProjPath, char sign, int mode) { // updates next add/commit id
    // mode 0 is for changing add id and mode 1 is for changing commit id
    char filePath[MAX_PATH]; strcpy(filePath, ProjPath); strcat(filePath, "/.neogit/current_IDs.txt");

    FILE *current_IDs = fopen(filePath, "r");
    char addID[10]; char commitID[10]; char curCommit[10];
    fgets(addID, sizeof(addID), current_IDs);
    fgets(commitID, sizeof(commitID), current_IDs);
    fgets(curCommit, sizeof(curCommit), current_IDs);
    fclose(current_IDs);

    if (!mode) {
        int pos = strlen(addID) - 2;
        if (sign == '+') {
            while (addID[pos] == '9') {
                addID[pos] = '0';
                pos--;
            }
            addID[pos]++;
        }
        else {
            while (addID[pos] == '0') {
                addID[pos] = '9';
                pos--;
            }
            addID[pos]--;
        }
    }
    else {
        int pos = strlen(commitID) - 2;
        if (sign == '+') {
            while (commitID[pos] == '9') {
                commitID[pos] = '0';
                pos--;
            }
            commitID[pos]++;
        }
        else {
            while (commitID[pos] == '0') {
                commitID[pos] = '9';
                pos--;
            }
            commitID[pos]--;
        }
    }
    current_IDs = fopen(filePath, "w");
    fputs(addID, current_IDs); fputs(commitID, current_IDs); fputs(curCommit, current_IDs);
    fclose(current_IDs);
    return;
}

int CompareFileTimes(char *file1, char *file2) { // compare files due to last time of modification
    struct stat stat1, stat2;
    stat(file1, &stat1);
    stat(file2, &stat2);
    if (stat1.st_mtime > stat2.st_mtime) return 1;
    else if (stat1.st_mtime < stat2.st_mtime) return 2;
    else return 0;
}

void ChangeConfig_UserName_GLOBAL(char *name) {
    Create("/home/pardis/.neogitconfig", NULL, 0);
    char infoPath[MAX_PATH]; 
    sprintf(infoPath, "/home/pardis/.neogitconfig/info.txt");
    if (access(infoPath, F_OK) != 0) Create(infoPath, "name =\nemail =\n", 1);
    FILE *config = fopen(infoPath, "r");
    char line[1000];
    fgets(line, sizeof(line), config); fgets(line, sizeof(line), config);
    fclose(config);
    config = fopen(infoPath, "w");
    fprintf(config, "name =%s\n%s", name, line);
    fclose(config);
    printf("Global user name added successfully!\n");
    return;
}

void ChangeConfig_UserName(char *infoPath, char *name) {
    FILE *config = fopen(infoPath, "r");
    char line[1000];
    fgets(line, sizeof(line), config); fgets(line, sizeof(line), config);
    fclose(config);
    config = fopen(infoPath, "w");
    fprintf(config, "name =%s\n%s", name, line);
    fclose(config);
    printf("User name added successfully!\n");
    return;
}

void ChangeConfig_UserEmail_GLOBAL(char *email) {
    Create("/home/pardis/.neogitconfig", NULL, 0);
    char infoPath[MAX_PATH]; 
    sprintf(infoPath, "/home/pardis/.neogitconfig/info.txt");
    if (access(infoPath, F_OK) != 0) Create(infoPath, "name =\nemail =\n", 1);
    FILE *config = fopen(infoPath, "r");
    char line[1000];
    fgets(line, sizeof(line), config);
    fclose(config);
    config = fopen(infoPath, "w");
    fprintf(config, "%semail =%s\n", line, email);
    fclose(config);
    printf("Global user email added successfully!\n");
    return;
}

void ChangeConfig_UserEmail(char *infoPath, char *email) {
    FILE *config = fopen(infoPath, "r");
    char line[1000];
    fgets(line, sizeof(line), config);
    fclose(config);
    config = fopen(infoPath, "w");
    fprintf(config, "%semail =%s\n", line, email);
    fclose(config);
    printf("User email added successfully!\n");
    return;
}

int IsValid(char *command) {
    // commands of phase 1
    if (!strncmp(command, "neogit2 config -global user.name ", 32)) return 1;
    if (!strncmp(command, "neogit2 config -global user.email ", 33)) return 1;
    if (!strncmp(command, "neogit2 config user.name ", 24)) return 1;
    if (!strncmp(command, "neogit2 config user.email ", 25)) return 1;
    if (!strncmp(command, "neogit2 config alias.", 20)) return 1;
    if (!strncmp(command, "neogit2 config -global alias.", 29)) return 1;
    if (!strcmp(command, "neogit2 init")) return 1;
    if (!strncmp(command, "neogit2 add ", 11)) return 1;
    if (!strncmp(command, "neogit2 reset ", 13)) return 1;
    if (!strcmp(command, "neogit2 status")) return 1;
    if (!strncmp(command, "neogit2 commit -m ", 17)) return 1;
    if (!strncmp(command, "neogit2 commit -s ", 17)) return 1;
    if (!strncmp(command, "neogit2 set -m ", 14)) return 1;
    if (!strncmp(command, "neogit2 replace -m ", 18)) return 1;
    if (!strncmp(command, "neogit2 remove -s ", 17)) return 1;
    if (!strncmp(command, "neogit2 log ", 11)) return 1;
    if (!strncmp(command, "neogit2 branch", 13)) return 1;
    if (!strncmp(command, "neogit2 checkout ", 16)) return 1;
    // commands of phase 2
    if (!strncmp(command, "neogit2 revert ", 14)) return 1; // hasn's been implemented.
    if (!strncmp(command, "neogit2 tag", 10)) return 1;
    if (!strcmp(command, "neogit2 tree")) return 1; // hasn's been implemented.
    if (!strncmp(command, "neogit2 stash push", 17)) return 1;
    if (!strncmp(command, "neogit2 stash show ", 18)) return 1;
    if (!strncmp(command, "neogit2 stash branch ", 20)) return 1;
    if (!strcmp(command, "neogit2 stash drop")) return 1;
    if (!strcmp(command, "neogit2 stash clear")) return 1;
    if (!strcmp(command, "neogit2 stash pop")) return 1;
    if (!strcmp(command, "neogit2 stash list")) return 1;
    if (!strcmp(command, "neogit2 pre-commit hooks list")) return 1;
    if (!strcmp(command, "neogit2 pre-commit applied hooks")) return 1;
    if (!strncmp(command, "neogit2 pre-commit add hook ", 27)) return 1;
    if (!strncmp(command, "neogit2 pre-commit remove hook ", 30)) return 1;
    if (!strcmp(command, "neogit2 pre-commit")) return 1;
    if (!strcmp(command, "neogit2 pre-commit -u")) return 1;
    if (!strncmp(command, "neogit2 pre-commit -f ", 21)) return 1;
    if (!strncmp(command, "neogit2 grep -f ", 15)) return 1;
    if (!strncmp(command, "neogit2 diff -f ", 15)) return 1;
    if (!strncmp(command, "neogit2 diff -c ", 15)) return 1;
    if (!strncmp(command, "neogit2 merge -b ", 16)) return 1;
    
    return 0;
}

int IsAlias(char *entrance, char *ProjPath) {
    char GlobalAliasPath[MAX_PATH];
    sprintf(GlobalAliasPath, "/home/pardis/.neogitconfig/alias");
    if (access(GlobalAliasPath, F_OK) != 0) Create(GlobalAliasPath, NULL, 0);
    char Command[100]; sprintf(Command, "%s.txt", entrance);
    char GlobalAliasFile[MAX_PATH]; sprintf(GlobalAliasFile, "%s/%s", GlobalAliasPath, Command);
    char *GlobAD = NULL;
    if (access(GlobalAliasPath, F_OK) == 0) GlobAD = GlobalAliasFile;
    char LocalAliasPath[MAX_PATH]; sprintf(LocalAliasPath, "%s/.neogit/alias", ProjPath);
    char LocalAliasFile[MAX_PATH]; sprintf(LocalAliasFile, "%s/%s", LocalAliasPath, Command);
    char *LocAD = NULL;
    if (access(LocalAliasFile, F_OK) == 0) LocAD = LocalAliasFile;

    FILE *f;
    if (GlobAD != NULL && LocAD != NULL) {
        switch (CompareFileTimes(GlobAD, LocAD)) {
        case 1:
            f = fopen(GlobAD, "r");
            break;
        case 2:
            f = fopen(LocAD, "r");
            break;
        }
    }
    else if (GlobAD != NULL) f = fopen(GlobAD, "r");
    else if (LocAD != NULL) f = fopen(LocAD, "r");
    else return 0;
    if (f == NULL) return 0;
    char line[1024];
    fgets(line, sizeof(line), f);
    fclose(f);
    f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "%s", line);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");
    return 1;
}

void init() {
    char cur_dir[MAX_PATH];
    getcwd(cur_dir, sizeof(cur_dir));
    if (Find_Repo(cur_dir)) {
        printf("Repository already exists!\n");
        return;
    }
    getcwd(cur_dir, sizeof(cur_dir));
    strcat(cur_dir, "/.neogit");
    mkdir(cur_dir, S_IRWXU | S_IRWXG | S_IRWXO);
    // saving configs
    char config[MAX_PATH];
    sprintf(config, "%s/config.txt", cur_dir);
    if (access("/home/pardis/.neogitconfig/info.txt", F_OK) != 0) {
        Create("/home/pardis/.neogitconfig", NULL, 0);
        Create("/home/pardis/.neogitconfig/info.txt", "name =\nemail =\n", 1);
        Create(config, "name =\nemail =\n", 1);
    }
    else {
        Create(config, "name =\nemail =\n", 1);
        FILE *f = fopen("/home/pardis/bash.sh", "w");
        fprintf(f, "cp -r /home/pardis/.neogitconfig/info.txt %s", cur_dir);
        fclose(f);
        chmod("/home/pardis/bash.sh", 0x777);
        system("/home/pardis/bash.sh");
        remove("/home/pardis/bash.sh");
    }
    // create default files and directories
    Create(".neogit/branch.txt", "master_0_0\n", 1);
    Create(".neogit/head.txt", "master\n", 1);
    Create(".neogit/current_IDs.txt", "20000\n10000\n0\n", 1);
    mkdir(".neogit/.stash", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/alias", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/shortcuts", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/commits", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/.staged", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/.records", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/.records/19999", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/tags", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/applied_hook", S_IRWXU | S_IRWXG | S_IRWXO);
    printf("Repository initialized successfully!\n");
    return;
}

void AddToStaging(char *Relpath, char *ProjPath) {
    char FilePath[MAX_PATH];
    getcwd(FilePath, sizeof(FilePath)); strcat(FilePath, "/"); strcat(FilePath, Relpath);
    if (access(FilePath, F_OK) != 0) {
        printf("File/Directory " _SGR_REDF "not found!\n" _SGR_RESET);
        return;
    }
    char StagingPath[MAX_PATH];
    sprintf(StagingPath, "%s/.neogit/.staged%s", ProjPath, FilePath + strlen(ProjPath));
    while (StagingPath[strlen(StagingPath) - 1] != '/') StagingPath[strlen(StagingPath) - 1] = '\0';
    StagingPath[strlen(StagingPath) - 1] = '\0';
    Create(StagingPath, NULL, 0); // check two or more parent folders can be created or we need recursion?
    FILE *f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "cp -r %s %s", FilePath, StagingPath);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");
    printf(_SGR_ITALIC "File added successfully!\n" _SGR_NOITALIC);
    return;
}

void ListFolders_n(char *ProjPath, char *curAddress, int depth, int Cdepth, bool check) { // compares files in Staging Area with Working Directory
    if (!depth) return;
    DIR *cur_dir = opendir(curAddress);
    struct dirent *walking;
    while ((walking = readdir(cur_dir)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            strcat(curAddress, "/"); strcat(curAddress, walking->d_name);
            printf("\n");
            for (int i = 0; i < Cdepth - depth + 1; i++) printf("  ");
            printf("Entering " _SGR_BOLD _SGR_YELLOWF "%s : \n"_SGR_RESET, walking->d_name);
            ListFolders_n(ProjPath, curAddress, depth - 1, Cdepth, true);
        }

        else if (walking->d_type != DT_DIR) {
            for (int i = 0; i < Cdepth - depth + 1; i++) printf("  ");
            char WDPath[MAX_PATH]; strcpy(WDPath, curAddress); strcat(WDPath, "/"); strcat(WDPath, walking->d_name);
            char SAPath[MAX_PATH]; strcpy(SAPath, ProjPath);
            strcat(SAPath, "/.neogit/.staged"); strcat(SAPath, curAddress + strlen(ProjPath)); strcat(SAPath, "/"); strcat(SAPath, walking->d_name);
            printf("%s", walking->d_name);
            if (!IsModified(SAPath, WDPath)) {
                printf(_SGR_REDF _SGR_BOLD "\tUnstaged\n" _SGR_RESET);
                check = false;
            }
            else printf(_SGR_GREENF _SGR_BOLD "\tStaged\n" _SGR_RESET);
        }
    }
    for (int i = 0; i < Cdepth - depth; i++) printf("  ");
    if (check) printf("Directory is " _SGR_BLUEF "staged\n\n" _SGR_RESET);
    else printf("Directory is " _SGR_BLUEF "NOT staged\n\n" _SGR_RESET);
    while (curAddress[strlen(curAddress) - 1] != '/') curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cur_dir);
    return;
}

void AddToRecords(char *ProjPath) // keeps records of Staging Area for UNDO
{
    char SAPath[MAX_PATH]; sprintf(SAPath, "%s/.neogit/.staged", ProjPath);
    char RCPath[MAX_PATH]; sprintf(RCPath, "%s/.neogit/.records", ProjPath);
    FILE *f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "cp -r %s %s", SAPath, RCPath);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");
    int ID = Get_addID(ProjPath);
    char NewName[MAX_PATH]; sprintf(NewName, "%s/%d", RCPath, ID); strcat(RCPath, "/.staged");
    rename(RCPath, NewName);
    Increase_Decrease_ID(ProjPath, '+', 0);
    return;
}

void AddRedo(char *ProjPath, char *Staged) {
    DIR *cur_dir = opendir(Staged);
    struct dirent *walking;
    while ((walking = readdir(cur_dir)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) {
            char WDPath[MAX_PATH]; sprintf(WDPath, "%s%s/%s", ProjPath, Staged + strlen(ProjPath) + 16, walking->d_name);
            char SAPath[MAX_PATH]; sprintf(SAPath, "%s/%s", Staged, walking->d_name);
            if (access(WDPath, F_OK) != 0) {
                FILE *f = fopen("/home/pardis/bash.sh", "w");
                fprintf(f, "rm -r %s", SAPath);
                fclose(f);
                chmod("/home/pardis/bash.sh", 0x777);
                system("/home/pardis/bash.sh");
                remove("/home/pardis/bash.sh");
                return;
            }
            strcat(Staged, "/"); strcat(Staged, walking->d_name);
            AddRedo(ProjPath, Staged);
        }
        else if (walking->d_type != DT_DIR) {
            char WDPath[MAX_PATH]; sprintf(WDPath, "%s%s/%s", ProjPath, Staged + strlen(ProjPath) + 16, walking->d_name);
            char SAPath[MAX_PATH]; sprintf(SAPath, "%s/%s", Staged, walking->d_name);
            if (access(WDPath, F_OK) != 0) {
                remove(SAPath);
                return;
            }
            FILE *f = fopen("/home/pardis/bash.sh", "w");
            fprintf(f, "cp -r %s %s", WDPath, SAPath);
            fclose(f);
            chmod("/home/pardis/bash.sh", 0x777);
            system("/home/pardis/bash.sh");
            remove("/home/pardis/bash.sh");
        }
    }
    while (Staged[strlen(Staged) - 1] != '/') Staged[strlen(Staged) - 1] = '\0';
    Staged[strlen(Staged) - 1] = '\0';
    closedir(cur_dir);
    return;
}

void RemoveFromStaging(char *name, char *ProjPath) {
    char FileName[MAX_PATH]; strcpy(FileName, name);
    char FilePath[MAX_PATH]; getcwd(FilePath, sizeof(FilePath));
    char StagedPath[MAX_PATH]; sprintf(StagedPath, "%s/.neogit/.staged%s/%s", ProjPath, FilePath + strlen(ProjPath), FileName);
    if (access(StagedPath, F_OK) != 0) {
        printf("File/Directory " _SGR_YELLOWF "NOT staged!\n" _SGR_RESET);
        return;
    }

    FILE *f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "rm -r %s", StagedPath);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");

    printf(_SGR_ITALIC "File unstaged successfully!"_SGR_NOITALIC "\n");
    return;
}

void ResetUndo(char *ProjPath) {
    int ID = Get_addID(ProjPath);
    if (ID == 20000) {
        printf("You should add something first and then try to undo it!\n");
        return;
    }
    Increase_Decrease_ID(ProjPath, '-', 0);
    ID--;
    char stagePath[MAX_PATH]; sprintf(stagePath, "%s/.neogit/.records/%d", ProjPath, ID);
    FILE *f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "rm -r %s", stagePath);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");
    sprintf(stagePath, "%s/.neogit/.staged/", ProjPath); // slash neccessary?
    f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "rm -r %s", stagePath);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");
    sprintf(stagePath, "%s/.neogit/.records/%d", ProjPath, --ID);
    char projNeo[MAX_PATH]; sprintf(projNeo, "%s/.neogit", ProjPath);
    f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "cp -r %s %s", stagePath, projNeo);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");
    sprintf(stagePath, "%s/.neogit/%d", ProjPath, ID);
    strcat(projNeo, "/.staged");
    rename(stagePath, projNeo);
    printf("Reset undone successfully!\n");
    return;
}

mode_t CompareMode(char *file1, char *file2) {
    struct stat stat1, stat2;
    stat(file1, &stat1);
    stat(file2, &stat2);
    return stat1.st_mode == stat2.st_mode;
}

void WalkOnCommit(char *ProjPath, char *curAddress) {
    DIR *cm = opendir(curAddress);
    struct dirent *walking;
    while ((walking = readdir(cm)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            strcat(curAddress, "/"); strcat(curAddress, walking->d_name);
            WalkOnCommit(ProjPath, curAddress);
        }
        else if (walking->d_type != DT_DIR && strcmp(walking->d_name, "info.txt")) {
            strcat(curAddress, "/"); strcat(curAddress, walking->d_name);
            char SAPath[MAX_PATH]; sprintf(SAPath, "%s/.neogit/.staged%s", ProjPath, curAddress + strlen(ProjPath) + 22);
            char WDPath[MAX_PATH]; sprintf(WDPath, "%s%s", ProjPath, curAddress + strlen(ProjPath) + 22);
            if (access(WDPath, F_OK) == 0) {
                if (!CompareMode(WDPath, curAddress)) printf(_SGR_ITALIC "%s\t\t" _SGR_NOITALIC _SGR_BOLD _SGR_GREENF "T" _SGR_RESET "\n", WDPath + strlen(ProjPath));
                if (!IsModified(curAddress, WDPath)) {
                    if (access(SAPath, F_OK) == 0) printf("%s\t\t" _SGR_BOLD _SGR_MAGENF "+M" _SGR_RESET "\n", WDPath + strlen(ProjPath) + 1);
                    else printf("%s\t\t" _SGR_BOLD _SGR_MAGENF "-M" _SGR_RESET "\n", WDPath + strlen(ProjPath) + 1);
                }
            }
            else {
                if (access(SAPath, F_OK) == 0) printf("%s\t\t" _SGR_BOLD _SGR_REDF "+D" _SGR_RESET "\n", WDPath + strlen(ProjPath) + 1);
                else printf("%s\t\t" _SGR_BOLD _SGR_REDF "-D" _SGR_RESET "\n", WDPath + strlen(ProjPath) + 1);
            }
            while (curAddress[strlen(curAddress) - 1] != '/') curAddress[strlen(curAddress) - 1] = '\0';
            curAddress[strlen(curAddress) - 1] = '\0';
        }
    }
    while (curAddress[strlen(curAddress) - 1] != '/') curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cm);
    return;
}

void WalkOnWD(char *ProjPath, char *curAddress) {
    DIR *cm = opendir(curAddress);
    struct dirent *walking;
    while ((walking = readdir(cm)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            strcat(curAddress, "/"); strcat(curAddress, walking->d_name);
            WalkOnWD(ProjPath, curAddress);
        }
        else if (walking->d_type != DT_DIR) {
            strcat(curAddress, "/"); strcat(curAddress, walking->d_name);
            char SAPath[MAX_PATH]; sprintf(SAPath, "%s/.neogit/.staged%s", ProjPath, curAddress + strlen(ProjPath));
            char CMPath[MAX_PATH];
            int ID = Get_commitID(ProjPath) - 1;
            sprintf(CMPath, "%s/.neogit/commits/%d%s", ProjPath, ID, curAddress + strlen(ProjPath));
            if (access(CMPath, F_OK) != 0) {
                if (access(SAPath, F_OK) == 0) printf("%s\t\t" _SGR_BOLD _SGR_YELLOWF "+A" _SGR_RESET "\n", curAddress + strlen(ProjPath) + 1);
                else printf("%s\t\t" _SGR_BOLD _SGR_YELLOWF "-A" _SGR_RESET "\n", curAddress + strlen(ProjPath) + 1);
            }
            while (curAddress[strlen(curAddress) - 1] != '/') curAddress[strlen(curAddress) - 1] = '\0';
            curAddress[strlen(curAddress) - 1] = '\0';
        }
    }
    while (curAddress[strlen(curAddress) - 1] != '/') curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cm);
    return;
}

void Status(char *ProjPath) {
    char filePath[MAX_PATH]; sprintf(filePath, "%s/.neogit/current_IDs.txt", ProjPath);
    char *ID = Get_Current_Commit_ID(ProjPath);
    if (!strcmp(ID, "0")) {
        printf("You should at least commit once to check status!\n");
        return;
    }
    char commitPath[MAX_PATH]; sprintf(commitPath, "%s/.neogit/commits/%s", ProjPath, ID);
    WalkOnCommit(ProjPath, commitPath);
    char curAddress[MAX_PATH]; strcpy(curAddress, ProjPath);
    WalkOnWD(ProjPath, curAddress);
    return;
}

void SetShortcut(char *message, char *shortcut, char *ProjPath) {
    char path[MAX_PATH]; sprintf(path, "%s/.neogit/shortcuts/%s.txt", ProjPath, shortcut);
    FILE *f = fopen(path, "w");
    fputs(message, f);
    fclose(f);
    return;
}

void CreateLn(char *ProjPath, char *Src, char *Dest) { // copies files that has stayed unchaged since last commit into new commit
    DIR *cur_dir = opendir(Src);
    struct dirent *walking;
    while ((walking = readdir(cur_dir)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) {
            strcat(Src, "/"); strcat(Src, walking->d_name);
            strcat(Dest, "/"); strcat(Dest, walking->d_name);
            if (access(Dest, F_OK) != 0) Create(Dest, NULL, 0);
            CreateLn(ProjPath, Src, Dest);
        }
        else if (walking->d_type != DT_DIR) {
            char WDPath[MAX_PATH];
            sprintf(WDPath, "%s%s", ProjPath, Src + strlen(ProjPath) + 22); strcat(WDPath, "/"); strcat(WDPath, walking->d_name);
            char CMPath[MAX_PATH]; strcpy(CMPath, Src); strcat(CMPath, "/"); strcat(CMPath, walking->d_name);
            strcat(Dest, "/"); strcat(Dest, walking->d_name);
            if (IsModified(CMPath, WDPath)) {
                FILE *p = fopen("/home/pardis/bash.sh", "w");
                fprintf(p, "cp -r %s %s", CMPath, Dest);
                fclose(p);
                chmod("/home/pardis/bash.sh", 0x777);
                system("/home/pardis/bash.sh");
                remove("/home/pardis/bash.sh");
            }
            while (Dest[strlen(Dest) - 1] != '/') Dest[strlen(Dest) - 1] = '\0';
            Dest[strlen(Dest) - 1] = '\0';
        }
    }
    while (Src[strlen(Src) - 1] != '/') Src[strlen(Src) - 1] = '\0';
    Src[strlen(Src) - 1] = '\0';
    while (Dest[strlen(Dest) - 1] != '/') Dest[strlen(Dest) - 1] = '\0';
    Dest[strlen(Dest) - 1] = '\0';
    closedir(cur_dir);
    return;
}

void Commit(char *ProjPath, char *message) {
    char *headID = Get_HEAD_ID(ProjPath);
    char *commitID = Get_Current_Commit_ID(ProjPath);
    if (strcmp(headID, commitID)) {
        printf("You can only commit changes on HEAD.\n");
        return;
    }
    char StagePath[MAX_PATH]; strcpy(StagePath, ProjPath); strcat(StagePath, "/.neogit/.staged");
    int count = 0;
    DIR *StageDir = opendir(StagePath);
    struct dirent *walking;
    while ((walking = readdir(StageDir)) != NULL) if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) count++;
    closedir(StageDir);
    if (count == 0) {
        printf("Nothing has been staged since last commit!\n");
        return;
    }
    Launch_pre(ProjPath, 1);
    if (!checkHook) {
        printf("You can't commit because of violating the above hooks!\n");
        return;
    }
    char Username[100], Useremail[100], branch[100];
    char globalConfig[MAX_PATH], localConfig[MAX_PATH];
    sprintf(globalConfig, "/home/pardis/.neogitconfig/info.txt"); sprintf(localConfig, "%s/.neogit/config.txt", ProjPath);
    FILE *config;
    if (access(globalConfig, F_OK)) {
        Create("/home/pardis/.neogitconfig", NULL, 0);
        Create(globalConfig, "name =\nemail =\n", 1);
        config = fopen(localConfig, "r");
    }
    else {
        switch (CompareFileTimes(globalConfig, localConfig)) {
        case 1:
            config = fopen(globalConfig, "r");
            break;
        case 2:
            config = fopen(localConfig, "r");
            break;
        case 0:
            config = fopen(localConfig, "r");
            break;
        }
    }
    fgets(Username, sizeof(Username), config);
    fgets(Useremail, sizeof(Useremail), config);
    strtok(Username, "=");
    char *tokName = strtok(NULL, "\n");
    strtok(Useremail, "=");
    char *tokEmail = strtok(NULL, "\n");
    fclose(config);
    if (tokName == NULL || tokEmail == NULL) {
        printf("You haven't set config yet! " _SGR_YELLOWF "Please set and try again!\n" _SGR_RESET);
        return;
    }
    int ID = Get_commitID(ProjPath);
    Increase_Decrease_ID(ProjPath, '+', 1);
    char commitPath[MAX_PATH]; sprintf(commitPath, "%s/.neogit/commits", ProjPath);
    FILE *f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "cp -r %s %s", StagePath, commitPath);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");
    f = fopen("/home/pardis/bash.sh", "w");
    fprintf(f, "rm -r %s", StagePath);
    fclose(f);
    chmod("/home/pardis/bash.sh", 0x777);
    system("/home/pardis/bash.sh");
    remove("/home/pardis/bash.sh");
    Create(StagePath, NULL, 0);
    sprintf(StagePath, "%s/.neogit/commits/.staged", ProjPath); sprintf(commitPath, "%s/.neogit/commits/%d", ProjPath, ID);
    rename(StagePath, commitPath);
    strcat(commitPath, "/info.txt");
    f = fopen(commitPath, "w");
    commitPath[strlen(commitPath) - 9] = '\0';
    char Head[MAX_PATH]; sprintf(Head, "%s/.neogit/head.txt", ProjPath);
    config = fopen(Head, "r");
    fgets(branch, sizeof(branch), config);
    if (branch[strlen(branch) - 1] == '\n') branch[strlen(branch) - 1] = '\0';
    fclose(config);
    Update_Branch(ProjPath, branch, ID);
    fprintf(f, "Username =%s\nUseremail =%s\nBranch =%s\nMessage =%s\n", tokName, tokEmail, branch, message);
    fclose(f);
    if (ID > 10000) {
        char Dest[MAX_PATH]; sprintf(Dest, "%s/.neogit/commits/%d", ProjPath, ID);
        char Src[MAX_PATH]; sprintf(Src, "%s/.neogit/commits/%s", ProjPath, headID);
        CreateLn(ProjPath, Src, Dest);
    }
    struct stat folder; stat(commitPath, &folder);
    time_t crt = folder.st_ctime;
    printf("Committed successfully in time" _SGR_BLUEF " %s"_SGR_RESET _SGR_REDF "ID = %d" _SGR_RESET "\nMessage =" _SGR_YELLOWF " \"%s\"\n" _SGR_RESET, ctime(&crt), ID, message);
    char *newID = (char *) malloc (10 * sizeof(char));
    sprintf(newID, "%d", ID);
    Update_Current_Commit_ID(ProjPath, newID);
    return;
}

void CountFiles(char *address, int *num) {
    DIR *dir = opendir(address);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit")) {
            strcat(address, "/"); strcat(address, entry->d_name);
            CountFiles(address, num);
        }
        else if (entry->d_type != DT_DIR) (*num)++;
    }
    while (address[strlen(address) - 1] != '/') address[strlen(address) - 1] = '\0';
    address[strlen(address) - 1] = '\0';
    closedir(dir);
    return;
}

int CompareCommitTime(char *filename, char *time) { // returns 1 if file's creation is before time and -1 otherwise
    struct stat fileStat; stat(filename, &fileStat);
    time_t fileTime = fileStat.st_ctime;
    struct tm inputTime; strptime(time, "%Y/%m/%d %H:%M:%S", &inputTime);
    time_t inputTimeValue = mktime(&inputTime);
    if (inputTimeValue < fileTime) return -1;
    else if (inputTimeValue > fileTime) return 1;
    return 0;
}

void Log(char *ProjPath, int n) {
    char commits[MAX_PATH]; sprintf(commits, "%s/.neogit/commits", ProjPath);
    int ID = Get_commitID(ProjPath) - 1;
    for (int i = ID; i >= ID - n + 1; i--) {
        char adrs[MAX_PATH]; sprintf(adrs, "%s/%d", commits, i);
        int num = 0;
        char curAddress[MAX_PATH]; strcpy(curAddress, adrs);
        CountFiles(curAddress, &num);
        struct stat folder;
        stat(adrs, &folder);
        time_t crt = folder.st_ctime;
        strcat(adrs, "/info.txt");
        FILE *f = fopen(adrs, "r");
        char line[1000];
        while (fgets(line, sizeof(line), f) != NULL) printf("%s", line);
        fclose(f);
        printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
        printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
    }
    return;
}

void LogExtra(char *ProjPath, char *TYPEt, char *WORDt) { // show commit logs due to Branch/Author
    if (!strcmp(TYPEt, "Branch")) {
        char branchPath[MAX_PATH]; sprintf(branchPath, "%s/.neogit/branch.txt", ProjPath);
        FILE *branch = fopen(branchPath, "r");
        char line[1000];
        bool hasBranch = false;
        while (fgets(line, sizeof(line), branch) != NULL) {
            if (!strncmp(WORDt, line, strlen(WORDt))) {
                hasBranch = true;
                break;
            }
        }
        fclose(branch);
        if (!hasBranch) {
            printf("Branch doesn't exist!\n");
            return;
        }
    }
    char commits[MAX_PATH]; sprintf(commits, "%s/.neogit/commits", ProjPath);
    int ID = Get_commitID(ProjPath) - 1;
    for (int i = ID; i >= 10000; i--) {
        char adrs[MAX_PATH]; sprintf(adrs, "%s/%d", commits, i);
        char folder[MAX_PATH]; strcpy(folder, adrs); strcat(adrs, "/info.txt");
        FILE *f = fopen(adrs, "r");
        char line[1000];
        while (fgets(line, sizeof(line), f) != NULL) {
            if (!strncmp(line, TYPEt, strlen(TYPEt)))
                if (!strncmp(line + strlen(TYPEt) + 2, WORDt, strlen(WORDt))) {
                    rewind(f);
                    char tmp[1000];
                    while (fgets(tmp, sizeof(tmp), f) != NULL) printf("%s", tmp);
                    int num = 0;
                    CountFiles(folder, &num);
                    struct stat folder; stat(adrs, &folder);
                    time_t crt = folder.st_ctime;
                    printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
                    printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
                    break;
                }
        }
        fclose(f);
    }
    return;
}

void LogSearch(char *ProjPath, char *TYPEt, char *WORDt) {
    char commits[MAX_PATH]; sprintf(commits, "%s/.neogit/commits", ProjPath);
    int ID = Get_commitID(ProjPath) - 1;
    for (int i = ID; i >= 10000; i--) {
        char adrs[MAX_PATH]; sprintf(adrs, "%s/%d", commits, i);
        int num = 0;
        char curAddress[MAX_PATH]; strcpy(curAddress, adrs);
        CountFiles(curAddress, &num);
        struct stat folder; stat(adrs, &folder);
        time_t crt = folder.st_ctime;
        strcat(adrs, "/info.txt");
        FILE *f = fopen(adrs, "r");
        char line[1000];
        while (fgets(line, sizeof(line), f) != NULL) {
            if (!strncmp(line, TYPEt, strlen(TYPEt)))
                if (strstr(line + strlen(TYPEt) + 2, WORDt) != NULL) {
                    fclose(f);
                    FILE *g = fopen(adrs, "r");
                    char tmp[1000];
                    while (fgets(tmp, sizeof(tmp), g) != NULL) printf("%s", tmp);
                    fclose(g);
                    printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
                    printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
                    break;
                }
        }
    }
    return;
}

void LogTime(char *ProjPath, char *time, char sign) { // sign - for log since and + for log before
    char commits[MAX_PATH]; sprintf(commits, "%s/.neogit/commits", ProjPath);
    int ID = Get_commitID(ProjPath) - 1;
    for (int i = ID; i >= 10000; i--) {
        char adrs[MAX_PATH]; sprintf(adrs, "%s/%d", commits, i);
        if (sign == '-' && CompareCommitTime(adrs, time) == 1) {
            int num = 0;
            char curAddress[MAX_PATH]; strcpy(curAddress, adrs);
            CountFiles(curAddress, &num);
            struct stat folder; stat(adrs, &folder);
            time_t crt = folder.st_ctime;
            strcat(adrs, "/info.txt");
            FILE *f = fopen(adrs, "r");
            char line[1000];
            while (fgets(line, sizeof(line), f) != NULL) printf("%s", line);
            fclose(f);
            printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
            printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
        }
        else if (sign == '+' && CompareCommitTime(adrs, time) == -1) {
            int num = 0;
            char curAddress[MAX_PATH]; strcpy(curAddress, adrs);
            CountFiles(curAddress, &num);
            struct stat folder; stat(adrs, &folder);
            time_t crt = folder.st_ctime;
            strcat(adrs, "/info.txt");
            FILE *f = fopen(adrs, "r");
            char line[1000];
            while (fgets(line, sizeof(line), f) != NULL) printf("%s", line);
            fclose(f);
            printf("\nCommit ID =" _SGR_REDF "%d\n" _SGR_RESET "Commit Time =" _SGR_BLUEF "%s" _SGR_RESET, i, ctime(&crt));
            printf("Number of commited files = " _SGR_YELLOWF "%d\n\n"_SGR_RESET, num);
        }
    }
}

void BranchCreate(char *name, char *ProjPath) {
    char branchPath[MAX_PATH]; sprintf(branchPath, "%s/.neogit/branch.txt", ProjPath);
    FILE *branchFile = fopen(branchPath, "r");
    char commitID[1000];
    while (fgets(commitID, sizeof(commitID), branchFile)) {
        strtok(commitID, "_");
        if (!strcmp(commitID, name)) {
            printf("Branch Already Exists!\n");
            return;
        }
    }
    fclose(branchFile);
    char *ID = Get_HEAD_ID(ProjPath);
    branchFile = fopen(branchPath, "a");
    fprintf(branchFile, "%s_%s_%s\n", name, ID, ID);
    fclose(branchFile);
    printf("New Branch %s created successfully!\n", name);
    return;
}

void BranchList(char *ProjPath) {
    char branchPath[MAX_PATH]; sprintf(branchPath, "%s/.neogit/branch.txt", ProjPath);
    FILE *branchFile = fopen(branchPath, "r");
    char branch[1000];
    char headPath[MAX_PATH]; sprintf(headPath, "%s/.neogit/head.txt", ProjPath);
    FILE *headFile = fopen(headPath, "r");
    char HEAD[1000];
    fgets(HEAD, sizeof(HEAD), headFile);
    if (HEAD[strlen(HEAD) - 1] == '\n') HEAD[strlen(HEAD) - 1] = '\0';
    fclose(headFile);
    while (fgets(branch, sizeof(branch), branchFile)) {
        strtok(branch, "_");
        printf("\t%s", branch);
        if (!strcmp(branch, HEAD)) printf("\t-> " _SGR_REDF "HEAD"_SGR_RESET);
        printf("\n");
    }
    return;
}

int WalkOnWDForCheckout(char *ProjPath, char *curAddress, char *ID) {
    DIR *cm = opendir(curAddress);
    struct dirent *walking;
    while ((walking = readdir(cm)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            strcat(curAddress, "/"); strcat(curAddress, walking->d_name);
            if (WalkOnWDForCheckout(ProjPath, curAddress, ID) == 0) return 0;
        }
        else if (walking->d_type != DT_DIR) {
            strcat(curAddress, "/"); strcat(curAddress, walking->d_name);
            char CMPath[MAX_PATH]; sprintf(CMPath, "%s/.neogit/commits/%s%s", ProjPath, ID, curAddress + strlen(ProjPath));
            if (access(CMPath, F_OK) == 0) {
                if (!IsModified(CMPath, curAddress)) {
                    closedir(cm);
                    return 0;
                }
            }
            else {
                closedir(cm);
                return 0;
            }
            while (curAddress[strlen(curAddress) - 1] != '/') curAddress[strlen(curAddress) - 1] = '\0';
            curAddress[strlen(curAddress) - 1] = '\0';
        }
    }
    while (curAddress[strlen(curAddress) - 1] != '/') curAddress[strlen(curAddress) - 1] = '\0';
    curAddress[strlen(curAddress) - 1] = '\0';
    closedir(cm);
    return 1;
}

void Checkout(char *where, char *ProjPath) {
    char *commitID = Get_Current_Commit_ID(ProjPath);
    char here[MAX_PATH]; strcpy(here, ProjPath);
    bool IsEqual = WalkOnWDForCheckout(ProjPath, here, commitID);
    if (!IsEqual) {
        printf("There are Modified/Untracked files in working directory. You " _SGR_REDF "CAN'T CHECKOUT" _SGR_RESET ". " _SGR_YELLOWF "Commit" _SGR_RESET " or " _SGR_BLUEF "Stash" _SGR_RESET " and try again!\n");
        return;
    }
    if (where[0] > '9' || where[0] < '0') {
        char headPath[MAX_PATH]; sprintf(headPath, "%s/.neogit/head.txt", ProjPath);
        FILE *h_f = fopen(headPath, "w");
        fprintf(h_f, "%s\n", where);
        fclose(h_f);
        printf("You've checked out on " _SGR_YELLOWF "%s" _SGR_RESET "!\n", where);
        char branchPath[MAX_PATH]; sprintf(branchPath, "%s/.neogit/branch.txt", ProjPath);
        FILE *b_f = fopen(branchPath, "r");
        char b_line[1000];
        char *ID;
        while (fgets(b_line, sizeof(b_line), b_f)) {
            strtok(b_line, "_");
            if (!strcmp(b_line, where)) {
                strtok(NULL, "_");
                where = strtok(NULL, "\n");
                break;
            }
        }
        fclose(b_f);
    }
    else {
        char infoPath[MAX_PATH]; sprintf(infoPath, "%s/.neogit/commits/%s/info.txt", ProjPath, where);
        FILE *i_f = fopen(infoPath, "r");
        char b_line[1000];
        fgets(b_line, sizeof(b_line), i_f);
        fgets(b_line, sizeof(b_line), i_f);
        fgets(b_line, sizeof(b_line), i_f);
        strtok(b_line, "=");
        char *curBranch = strtok(NULL, "\n");
        char headPath[MAX_PATH]; sprintf(headPath, "%s/.neogit/head.txt", ProjPath);
        FILE *h_f = fopen(headPath, "r");
        char HEAD[100];
        fgets(HEAD, sizeof(HEAD), h_f);
        if (HEAD[strlen(HEAD) - 1] == '\n') HEAD[strlen(HEAD) - 1] = '\0';
        if (strcmp(HEAD, curBranch)) {
            printf("You should be on the " _SGR_GREENF "branch of this commit ID" _SGR_RESET
                   " to checkout!\n");
            return;
        }
        printf("You've checked out on " _SGR_YELLOWF "%s" _SGR_RESET "!\n", where);
    }

    DIR *WDdir = opendir(ProjPath);
    struct dirent *walking;
    while ((walking = readdir(WDdir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            char tmp[MAX_PATH]; sprintf(tmp, "%s/%s", ProjPath, walking->d_name);
            FILE *f = fopen("/home/pardis/bash.sh", "w");
            fprintf(f, "rm -r %s", tmp);
            fclose(f);
            chmod("/home/pardis/bash.sh", 0x777);
            system("/home/pardis/bash.sh");
            remove("/home/pardis/bash.sh");
        }
    }
    closedir(WDdir);

    char commitPath[MAX_PATH]; sprintf(commitPath, "%s/.neogit/commits/%s", ProjPath, where);
    if (commitPath[strlen(commitPath) - 1] == '\n') commitPath[strlen(commitPath) - 1] = '\0';
    DIR *CMdir = opendir(commitPath);
    while ((walking = readdir(CMdir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt")) {
            char tmp[MAX_PATH]; sprintf(tmp, "%s/%s", commitPath, walking->d_name);
            FILE *f = fopen("/home/pardis/bash.sh", "w");
            fprintf(f, "cp -r %s %s", tmp, ProjPath);
            fclose(f);
            chmod("/home/pardis/bash.sh", 0x777);
            system("/home/pardis/bash.sh");
            remove("/home/pardis/bash.sh");
        }
    }
    closedir(CMdir);
    Update_Current_Commit_ID(ProjPath, where);
    return;
}

void CheckoutHead(char *where, char *ProjPath) {
    char headPath[MAX_PATH]; sprintf(headPath, "%s/.neogit/head.txt", ProjPath);
    FILE *headFile = fopen(headPath, "r");
    char HEAD[1000];
    fgets(HEAD, sizeof(HEAD), headFile);
    HEAD[strlen(HEAD) - 1] = '\0';
    fclose(headFile);

    char *commitID = Get_Current_Commit_ID(ProjPath);
    char here[MAX_PATH]; strcpy(here, ProjPath);
    bool IsEqual = WalkOnWDForCheckout(ProjPath, here, commitID);
    if (!IsEqual) {
        printf("There are Modified/Untracked files in working directory. You " _SGR_REDF "CAN'T CHECKOUT" _SGR_RESET ". " _SGR_YELLOWF "Commit" _SGR_RESET " or " _SGR_BLUEF "Stash" _SGR_RESET " and try again!\n");
        return;
    }
    if (strlen(where) != 4) {
        int n = 0;
        for (int i = 5; i < strlen(where); i++) {
            n *= 10;
            n += where[i] - '0';
        }
        printf("You've checked out on %d commit(s) before " _SGR_YELLOWF "HEAD!\n" _SGR_RESET, n);
        char commits[MAX_PATH]; sprintf(commits, "%s/.neogit/commits", ProjPath);
        int comID = Get_commitID(ProjPath) - 1;
        int i;
        for (i = comID; i >= 10000; i--) {
            char cmPath[MAX_PATH]; sprintf(cmPath, "%s/%d/info.txt", commits, i);
            FILE *info = fopen(cmPath, "r");
            char line[1000];
            fgets(line, sizeof(line), info);
            fgets(line, sizeof(line), info);
            fgets(line, sizeof(line), info);
            if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
            if (!strcmp(line + 8, HEAD)) n--;
            fclose(info);
            if (n == -1) break;
        }
        sprintf(commitID, "%d", i);
    }
    else {
        char branchPath[MAX_PATH]; sprintf(branchPath, "%s/.neogit/branch.txt", ProjPath);
        FILE *branchFile = fopen(branchPath, "r");
        char line[100];
        while (fgets(line, sizeof(line), branchFile)) {
            strtok(line, "_");
            if (!strcmp(line, HEAD)) {
                strtok(NULL, "_");
                strcpy(commitID, strtok(NULL, "\n"));
                break;
            }
        }
        fclose(branchFile);
        printf("You've checked out on " _SGR_YELLOWF "HEAD!\n" _SGR_RESET);
    }

    DIR *WDdir = opendir(ProjPath);
    struct dirent *walking;
    while ((walking = readdir(WDdir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            char tmp[MAX_PATH]; sprintf(tmp, "%s/%s", ProjPath, walking->d_name);
            FILE *f = fopen("/home/pardis/bash.sh", "w");
            fprintf(f, "rm -r %s", tmp);
            fclose(f);
            chmod("/home/pardis/bash.sh", 0x777);
            system("/home/pardis/bash.sh");
            remove("/home/pardis/bash.sh");
        }
    }
    closedir(WDdir);

    char commitPath[MAX_PATH]; sprintf(commitPath, "%s/.neogit/commits/%s", ProjPath, commitID);
    DIR *CMdir = opendir(commitPath);
    while ((walking = readdir(CMdir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt")) {
            char tmp[MAX_PATH]; sprintf(tmp, "%s/%s", commitPath, walking->d_name);
            FILE *f = fopen("/home/pardis/bash.sh", "w");
            fprintf(f, "cp -r %s %s", tmp, ProjPath);
            fclose(f);
            chmod("/home/pardis/bash.sh", 0x777);
            system("/home/pardis/bash.sh");
            remove("/home/pardis/bash.sh");
        }
    }
    closedir(CMdir);
    Update_Current_Commit_ID(ProjPath, commitID);
    return;
}

int IsValidLine(char *line) {
    for (int i = 0; i < strlen(line); i++) if (line[i] != '\n' && line[i] != '\t' && line[i] != ' ' && line[i] != '\r') return 1;
    return 0;
}

int CompareLines(char *line_1, char *line_2, char *file_1, char *file_2, int which_1, int which_2) {
    char *token;
    char **words_1 = NULL;
    int idx_1 = 0;
    token = strtok(line_1, " \n\t\r");
    while (token != NULL) {
        words_1 = (char **)realloc(words_1, (idx_1 + 1) * sizeof(char *));
        words_1[idx_1] = (char *) malloc (strlen(token) + 3);
        strcpy(words_1[idx_1], token);
        idx_1++;
        token = strtok(NULL, " \n\t\r");
    }
    char **words_2 = NULL;
    int idx_2 = 0;
    token = strtok(line_2, " \n\t\r");
    while (token != NULL) {
        words_2 = (char **)realloc(words_2, (idx_2 + 1) * sizeof(char *));
        words_2[idx_2] = (char *)malloc(strlen(token) + 3);
        strcpy(words_2[idx_2], token);
        idx_2++;
        token = strtok(NULL, " \n\t\r");
    }

    int minIDX = (idx_1 > idx_2) ? idx_2 : idx_1;
    int tmp_idx, diffWordsNum = 0;
    for (int i = 0; i < minIDX; i++)
        if (strcmp(words_1[i], words_2[i])) {
            diffWordsNum++;
            tmp_idx = i;
        }

    int wordsLeft = (idx_1 > idx_2) ? idx_1 - idx_2 : idx_2 - idx_1;

    if (wordsLeft == 0) {
        if (diffWordsNum == 0) return 0;
        else if (diffWordsNum == 1) {
            char tmp[50];
            sprintf(tmp, ">%s<", words_1[tmp_idx]);
            strncpy(words_1[tmp_idx], tmp, strlen(tmp));
            sprintf(tmp, ">%s<", words_2[tmp_idx]);
            strcpy(words_2[tmp_idx], tmp);

            printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                "%d\n" _SGR_RESET,
                   file_1, which_1);
            printf("\t");
            for (int i = 0; i < idx_1; i++) {
                if (!strncmp(">", words_1[i], 1)) printf(_SGR_REDF "%s " _SGR_RESET, words_1[i]);
                else printf(_SGR_CYANF "%s "_SGR_RESET, words_1[i]);
            }
            printf("\n");
            printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                "%d" _SGR_RESET "\n\t",
                   file_2, which_2);
            for (int i = 0; i < idx_2; i++) {
                if (!strncmp(">", words_2[i], 1)) printf(_SGR_REDF "%s " _SGR_RESET, words_2[i]);
                else printf(_SGR_BLUEF "%s " _SGR_RESET, words_2[i]);
            }
            printf("\n\n");
        }
        else {
            printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                "%d\n" _SGR_RESET,
                   file_1, which_1);
            printf("\t");
            for (int i = 0; i < idx_1; i++) printf(_SGR_CYANF "%s "_SGR_RESET, words_1[i]);
            printf("\n");
            printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                "%d" _SGR_RESET "\n\t",
                   file_2, which_2);
            for (int i = 0; i < idx_2; i++) printf(_SGR_BLUEF "%s " _SGR_RESET, words_2[i]);
            printf("\n\n");
        }
    }
    else if (wordsLeft == 1) { 
        if (diffWordsNum != 0) {
            printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                "%d\n" _SGR_RESET,
                   file_1, which_1);
            printf("\t");
            for (int i = 0; i < idx_1; i++) printf(_SGR_CYANF "%s "_SGR_RESET, words_1[i]);
            printf("\n");
            printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                "%d" _SGR_RESET "\n\t",
                   file_2, which_2);
            for (int i = 0; i < idx_2; i++) printf(_SGR_BLUEF "%s " _SGR_RESET, words_2[i]);
            printf("\n\n");
        }
        else {
            if (idx_1 > idx_2) {
                printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                    "%d\n" _SGR_RESET,
                       file_1, which_1);
                printf("\t");
                for (int i = 0; i < idx_1 - 1; i++) printf(_SGR_CYANF "%s "_SGR_RESET, words_1[i]);
                printf(_SGR_REDF ">%s< " _SGR_RESET, words_1[idx_1 - 1]);
                printf("\n");
                printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                    "%d" _SGR_RESET "\n\t",
                       file_2, which_2);
                for (int i = 0; i < idx_2; i++) printf(_SGR_BLUEF "%s " _SGR_RESET, words_2[i]);
                printf("\n\n");
            }
            else {
                printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                    "%d\n" _SGR_RESET,
                       file_1, which_1);
                printf("\t");
                for (int i = 0; i < idx_1; i++) printf(_SGR_CYANF "%s "_SGR_RESET, words_1[i]);
                printf("\n");
                printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                                    "%d" _SGR_RESET "\n\t",
                       file_2, which_2);
                for (int i = 0; i < idx_2 - 1; i++) printf(_SGR_BLUEF "%s " _SGR_RESET, words_2[i]);
                printf(_SGR_REDF ">%s< " _SGR_RESET, words_2[idx_2 - 1]);
                printf("\n\n");
            }
        }
    }
    else {
        printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                            "%d\n" _SGR_RESET,
               file_1, which_1);
        printf("\t");
        for (int i = 0; i < idx_1; i++) printf(_SGR_CYANF "%s "_SGR_RESET, words_1[i]);
        printf("\n");
        printf(_SGR_YELLOWF "%s" _SGR_RESET " - line: "_SGR_GREENF
                            "%d" _SGR_RESET "\n\t",
               file_2, which_2);
        for (int i = 0; i < idx_2; i++) printf(_SGR_BLUEF "%s " _SGR_RESET, words_2[i]);
        printf("\n\n");
    }

    for (int i = 0; i < idx_1; i++) free(words_1[i]);
    free(words_1);

    for (int i = 0; i < idx_2; i++) free(words_2[i]);
    free(words_2);
    return 1;
}

int Diff(char *file1, char *file2, int begin_1, int end_1, int begin_2, int end_2) {
    FILE *fptr_1 = fopen(file1, "r"); FILE *fptr_2 = fopen(file2, "r");

    char *name_1 = strrchr(file1, '/'); char *name_2 = strrchr(file2, '/');
    if (name_1 != NULL) name_1++;
    else name_1 = file1;
    if (name_2 != NULL) name_2++;
    else name_2 = file2;

    char line_1[1000], line_2[1000];
    int total_lines_1 = 0, total_lines_2 = 0;

    while (fgets(line_1, sizeof(line_1), fptr_1) != NULL) if (IsValidLine(line_1)) total_lines_1++;

    while (fgets(line_2, sizeof(line_2), fptr_2) != NULL) if (IsValidLine(line_2)) total_lines_2++;

    if (end_1 > total_lines_1) end_1 = total_lines_1;
    if (end_2 > total_lines_2) end_2 = total_lines_2;

    rewind(fptr_1); rewind(fptr_2);

    for (int i = 1; i < begin_1; i++)
        do
            fgets(line_1, sizeof(line_1), fptr_1);
        while (!IsValidLine(line_1));
    for (int i = 1; i < begin_2; i++)
        do
            fgets(line_2, sizeof(line_2), fptr_2);
        while (!IsValidLine(line_2));

    int len_1 = end_1 - begin_1; int len_2 = end_2 - begin_2;
    bool flag = false;

    if (len_2 > len_1) {
        for (int i = begin_1; i <= end_1; i++) {
            do
                fgets(line_1, sizeof(line_1), fptr_1);
            while (!IsValidLine(line_1));
            do
                fgets(line_2, sizeof(line_2), fptr_2);
            while (!IsValidLine(line_2));
            CompareLines(line_1, line_2, name_1, name_2, i, i - begin_1 + begin_2);
        }
        printf("\nLines in second file that does " _SGR_REDF "NOT" _SGR_RESET " exist in first one:\n");
        while (fgets(line_2, sizeof(line_2), fptr_2))
            if (IsValidLine(line_2))
            {
                printf(_SGR_BLUEF "%d:\t%s" _SGR_RESET, end_2 - 1, line_2);
                end_2++;
            }
        printf("\n");
        flag = true;
    }
    else if (len_2 < len_1) {
        for (int i = begin_2; i <= end_2; i++) {
            do
                fgets(line_1, sizeof(line_1), fptr_1);
            while (!IsValidLine(line_1));
            do
                fgets(line_2, sizeof(line_2), fptr_2);
            while (!IsValidLine(line_2));
            CompareLines(line_1, line_2, name_1, name_2, i - begin_2 + begin_1, i);
        }
        printf("\nLines in first file that does " _SGR_REDF "NOT" _SGR_RESET " exist in second one:\n");
        while (fgets(line_1, sizeof(line_1), fptr_1))
            if (IsValidLine(line_1)) {
                printf(_SGR_BLUEF "%d:\t%s" _SGR_RESET, end_1 - 1, line_1);
                end_1++;
            }
        printf("\n");
        flag = true;
    }
    else {
        for (int i = begin_1; i <= end_1; i++) {
            do
                fgets(line_1, sizeof(line_1), fptr_1);
            while (!IsValidLine(line_1));
            do
                fgets(line_2, sizeof(line_2), fptr_2);
            while (!IsValidLine(line_2));
            if (CompareLines(line_1, line_2, name_1, name_2, i, i - begin_1 + begin_2)) flag = true;
        }
    }
    return flag;
}

void DiffInCommits_solo(char *add_1, char *add_2) {
    DIR *cm = opendir(add_1);
    struct dirent *walking;
    while ((walking = readdir(cm)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) {
            strcat(add_1, "/"); strcat(add_1, walking->d_name);
            strcat(add_2, "/"); strcat(add_2, walking->d_name);
            DiffInCommits_solo(add_1, add_2);
        }
        else if (walking->d_type != DT_DIR && strcmp(walking->d_name, "info.txt")) {
            char file_1[MAX_PATH], file_2[MAX_PATH];
            sprintf(file_1, "%s/%s", add_1, walking->d_name); sprintf(file_2, "%s/%s", add_2, walking->d_name);

            if (access(file_2, F_OK) != 0) {
                char *name_1 = strrchr(file_1, '/');
                if (name_1 != NULL) name_1++;
                else name_1 = file_1;
                printf(_SGR_MAGENF "%s :\n" _SGR_RESET, name_1);
                FILE *f = fopen(file_1, "r");
                char line[1024];
                while (fgets(line, sizeof(line), f) != NULL) printf("%s", line);
                printf("\n\n");
                fclose(f);
            }
        }
    }
    while (add_1[strlen(add_1) - 1] != '/') add_1[strlen(add_1) - 1] = '\0';
    add_1[strlen(add_1) - 1] = '\0';
    while (add_2[strlen(add_2) - 1] != '/') add_2[strlen(add_2) - 1] = '\0';
    add_2[strlen(add_2) - 1] = '\0';
    closedir(cm);
    return;
}

void DiffInCommits_shared(char *add_1, char *add_2) {
    DIR *cm = opendir(add_1);
    struct dirent *walking;
    while ((walking = readdir(cm)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) {
            strcat(add_1, "/"); strcat(add_1, walking->d_name);
            strcat(add_2, "/"); strcat(add_2, walking->d_name);
            DiffInCommits_shared(add_1, add_2);
        }
        else if (walking->d_type != DT_DIR && strcmp(walking->d_name, "info.txt")) {
            char file_1[MAX_PATH], file_2[MAX_PATH];
            sprintf(file_1, "%s/%s", add_1, walking->d_name); sprintf(file_2, "%s/%s", add_2, walking->d_name);
            if (access(file_2, F_OK) == 0) Diff(file_1, file_2, 1, 10000, 1, 10000);
        }
    }
    while (add_1[strlen(add_1) - 1] != '/') add_1[strlen(add_1) - 1] = '\0';
    add_1[strlen(add_1) - 1] = '\0';
    while (add_2[strlen(add_2) - 1] != '/') add_2[strlen(add_2) - 1] = '\0';
    add_2[strlen(add_2) - 1] = '\0';
    closedir(cm);
    return;
}

void Grep(char *path, char *target, int n_flag) {
    char *name = strrchr(path, '/');
    if (name != NULL) name++;
    else name = path;
    printf("Found in " _SGR_YELLOWF "%s :\n" _SGR_RESET, name);

    FILE *fp = fopen(path, "r");
    int ln = 1;
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        bool check = false;
        char *token;
        char **words = NULL;
        int idx = 0;
        token = strtok(line, " \n\t\r");
        while (token != NULL) {
            words = (char **)realloc(words, (idx + 1) * sizeof(char *));
            words[idx] = (char *) malloc (strlen(token) + 2); strcpy(words[idx], token);
            idx++;
            token = strtok(NULL, " \n\t\r");
        }
        for (int i = 0; i < idx; i++) {
            if (strstr(target, "*")) {
                if (matchWildcard(target, words[i])) {
                    char tmp[50]; sprintf(tmp, ">%s", words[i]); strcpy(words[i], tmp);
                    check = true;
                }
            }
            else {
                if (!strcmp(target, words[i])) {
                    char tmp[50]; sprintf(tmp, ">%s", words[i]); strcpy(words[i], tmp);
                    check = true;
                }
            }
        }
        if (check) {
            if (n_flag) printf("%d:", ln);
            printf("\t");
            for (int i = 0; i < idx; i++) {
                if (!strncmp(words[i], ">", 1)) printf(_SGR_REDF "%s " _SGR_RESET, words[i] + 1);
                else printf("%s ", words[i]);
            }
            printf("\n");
        }
        for (int i = 0; i < idx; i++) free(words[i]);
        free(words);
        ln++;
    }
    return;
}

void GrepCommit(char *cmPath, int n_flag, char *target, char *file) {
    DIR *cm = opendir(cmPath);
    struct dirent *walking;
    while ((walking = readdir(cm)) != NULL) {
        if (walking->d_type == DT_DIR && strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) {
            strcat(cmPath, "/"); strcat(cmPath, walking->d_name);
            GrepCommit(cmPath, n_flag, target, file);
        }
        else if (walking->d_type != DT_DIR && strcmp(walking->d_name, "info.txt")) {
            if (!strcmp(walking->d_name, file)) {
                char tmp[MAX_PATH]; sprintf(tmp, "%s/%s", cmPath, walking->d_name);
                Grep(tmp, target, n_flag);
            }
        }
    }
    while (cmPath[strlen(cmPath) - 1] != '/') cmPath[strlen(cmPath) - 1] = '\0';
    cmPath[strlen(cmPath) - 1] = '\0';
    closedir(cm);
    return;
}

bool matchWildcard(char *wildcard, char *word) {
    while (*wildcard && *word) {
        if (*wildcard == *word || *wildcard == '?') {
            wildcard++;
            word++;
        }
        else if (*wildcard == '*') {
            while (*wildcard == '*') wildcard++;
            if (*wildcard == '\0') return true;
            while (*word && *word != *wildcard) word++;
        }
        else return false;
    }
    return (*wildcard == '\0' && *word == '\0');
}

void Tagging(char *Tagname, char *Message, char *CommitID, int state, char *ProjPath) {
    char TagAddress[MAX_PATH]; sprintf(TagAddress, "%s/.neogit/tags/%s.txt", ProjPath, Tagname);
    if (access(TagAddress, F_OK) == 0 && !state) {
        printf("the tag already exists\n");
        return;
    }
    FILE *Tag_file = fopen(TagAddress, "w");
    char Username[100], Useremail[100];
    char globalConfig[MAX_PATH], localConfig[MAX_PATH];
    sprintf(globalConfig, "/home/pardis/.neogitconfig/info.txt"); sprintf(localConfig, "%s/.neogit/config.txt", ProjPath);
    FILE *config;
    switch (CompareFileTimes(globalConfig, localConfig)) {
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
    tokName = strtok(NULL, "\n\r");
    char *tokEmail = strtok(Useremail, "=");
    tokEmail = strtok(NULL, "\n\r");
    fclose(config);
    if (CommitID == NULL)
        CommitID = Get_Current_Commit_ID(ProjPath);
    if (Message == NULL) fprintf(Tag_file, "Authorname=%s\nAuthoremail =%s\nCommit_ID =%s\nMessage =-\n", tokName, tokEmail, CommitID);
    else fprintf(Tag_file, "Authorname=%s\nAuthoremail =%s\nCommit_ID =%s\nMessage =%s\n", tokName, tokEmail, CommitID, Message);
    fclose(Tag_file);
    return;
}

void TagShow(char *TagName, char *ProjPath) {
    char openTag[100]; strcpy(openTag, TagName); strcat(openTag, ".txt");
    char TagAddress[MAX_PATH]; sprintf(TagAddress, "%s/.neogit/tags/%s", ProjPath, openTag);
    if (access(TagAddress, F_OK)) {
        printf("the tag doesn't exist!\n");
        return;
    }
    printf("tag %s\n", TagName);
    FILE *Tag_File = fopen(TagAddress, "r");
    char info[200];
    while (fgets(info, sizeof(info), Tag_File) != NULL) {
        printf("%s", info);
    }
    fclose(Tag_File);
    struct stat folder; stat(TagAddress, &folder);
    time_t crt = folder.st_ctime;
    printf("DATE:" _SGR_BLUEF " %s"_SGR_RESET, ctime(&crt));
    return;
}

int alphasort2(const struct dirent **a, const struct dirent **b) {
    return strcoll((*a)->d_name, (*b)->d_name);
}

void Show_acsending(char *ProjPath) {
    char TagAddress[MAX_PATH]; sprintf(TagAddress, "%s/.neogit/tags", ProjPath);
    DIR *Tag = opendir(TagAddress);
    struct dirent **walking;
    int n = scandir(TagAddress, &walking, NULL, alphasort2);
    for (int i = 0; i < n; i++) {
        if (strcmp(walking[i]->d_name, ".") && strcmp(walking[i]->d_name, "..")) printf("%s\n", walking[i]->d_name);
        free(walking[i]);
    }
    free(walking);
    closedir(Tag);
    return;
}

void Merge(char *branch1, char *branch2, char *ProjPath) {
    char branchAddress[MAX_PATH]; sprintf(branchAddress, "%s/.neogit/branch.txt", ProjPath);
    FILE *branch = fopen(branchAddress, "r");
    char b1HEAD[100], b2HEAD[100];
    char line[1000];
    while (fgets(line, sizeof(line), branch) != NULL) {
        strtok(line, "_");
        if (!strcmp(line, branch1)) {
            strtok(NULL, "_");
            strcpy(b1HEAD, strtok(NULL, "\n\r"));
        }
        else if (!strcmp(line, branch2)) {
            strtok(NULL, "_");
            strcpy(b2HEAD, strtok(NULL, "\n\r"));
        }
    }
    char b1Add[MAX_PATH]; sprintf(b1Add, "%s/.neogit/commits/%s", ProjPath, b1HEAD);
    char b2Add[MAX_PATH]; sprintf(b2Add, "%s/.neogit/commits/%s", ProjPath, b2HEAD);
    char tmpBranch1[MAX_PATH]; strcpy(tmpBranch1, b1Add);
    if (WalkForStash(b1Add, tmpBranch1, b2Add, 2)) {
        char Username[100], Useremail[100];
        char globalConfig[MAX_PATH], localConfig[MAX_PATH];
        sprintf(globalConfig, "/home/pardis/.neogitconfig/info.txt");
        sprintf(localConfig, "%s/.neogit/config.txt", ProjPath);
        FILE *config;
        switch (CompareFileTimes(globalConfig, localConfig)) {
        case 1:
            config = fopen(globalConfig, "r");
            break;
        case 2:
            config = fopen(localConfig, "r");
            break;
        }
        fgets(Username, sizeof(Username), config);
        fgets(Useremail, sizeof(Useremail), config);
        strtok(Username, "=");
        char *tokName = strtok(NULL, "\n");
        strtok(Useremail, "=");
        char *tokEmail = strtok(NULL, "\n");
        fclose(config);
        int ID = Get_commitID(ProjPath);
        Increase_Decrease_ID(ProjPath, '+', 1);
        char commitPath[MAX_PATH]; sprintf(commitPath, "%s/.neogit/commits/%d", ProjPath, ID);
        Create(commitPath, NULL, 0);
        DIR *toCopy = opendir(b1Add);
        struct dirent *entry;
        while ((entry = readdir(toCopy)) != NULL) {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit") && strcmp(entry->d_name, "info.txt")) {
                char add[MAX_PATH]; sprintf(add, "%s/%s", b1Add, entry->d_name);
                FILE *f = fopen("/home/pardis/bash.sh", "w");
                fprintf(f, "cp -r %s %s", add, commitPath);
                fclose(f);
                chmod("/home/pardis/bash.sh", 0x777);
                system("/home/pardis/bash.sh");
                remove("/home/pardis/bash.sh");
            }
        }
        toCopy = opendir(b2Add);
        while ((entry = readdir(toCopy)) != NULL) {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit") && strcmp(entry->d_name, "info.txt")) {
                char add[MAX_PATH]; sprintf(add, "%s/%s", b2Add, entry->d_name);
                FILE *f = fopen("/home/pardis/bash.sh", "w");
                fprintf(f, "cp -r %s %s", add, commitPath);
                fclose(f);
                chmod("/home/pardis/bash.sh", 0x777);
                system("/home/pardis/bash.sh");
                remove("/home/pardis/bash.sh");
            }
        }
        strcat(commitPath, "/info.txt");
        FILE *f = fopen(commitPath, "w");
        commitPath[strlen(commitPath) - 9] = '\0';
        Update_Branch(ProjPath, branch1, ID);
        Update_Branch(ProjPath, branch2, ID);
        char message[100]; sprintf(message, "Merge of %s and %s", branch1, branch2);
        fprintf(f, "Username =%s\nUseremail =%s\nBranch =%s\nMessage =%s", tokName, tokEmail, branch1, message);
        fclose(f);
        struct stat folder; stat(commitPath, &folder);
        time_t crt = folder.st_ctime;
        printf("Committed successfully in time" _SGR_BLUEF " %s"_SGR_RESET _SGR_REDF "ID = %d" _SGR_RESET "\nMessage =" _SGR_YELLOWF " \"%s\"\n" _SGR_RESET, ctime(&crt), ID, message);
        char CurIDPath[MAX_PATH]; sprintf(CurIDPath, "%s/.neogit/current_IDs.txt", ProjPath);
        FILE *CurID = fopen(CurIDPath, "r");
        char addID[10], commitID[10];
        fgets(addID, 10, CurID);
        fgets(commitID, 10, CurID);
        fclose(CurID);
        CurID = fopen(CurIDPath, "w");
        fputs(addID, CurID);
        fputs(commitID, CurID);
        fprintf(CurID, "%d\n", ID);
        fclose(CurID);
    }
    else printf("You can't merge these branches because of the above conflicts. Please resolve them and try again!\n\n");
    return;
}

void StashPush(bool hasMessage, char *Message, char *ProjPath) {
    char *ID = Get_HEAD_ID(ProjPath);
    char StashAddress[MAX_PATH]; sprintf(StashAddress, "%s/.neogit/.stash", ProjPath);
    DIR *stash = opendir(StashAddress);
    struct dirent *walking;
    while ((walking = readdir(stash)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) {
            char add1[MAX_PATH]; sprintf(add1, "%s/%s", StashAddress, walking->d_name);
            int entry = 0;
            for (int i = 0; i < strlen(walking->d_name); i++) {
                entry *= 10;
                entry += (walking->d_name)[i] - '0';
            }
            entry++;
            char add2[MAX_PATH]; sprintf(add2, "%s/%d", StashAddress, entry);
            rename(add1, add2);
        }
    }
    closedir(stash);
    char newEntry[MAX_PATH]; sprintf(newEntry, "%s/0", StashAddress);
    mkdir(newEntry, S_IRWXU | S_IRWXG | S_IRWXO);
    DIR *WorDir = opendir(ProjPath);
    while ((walking = readdir(WorDir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
            char add[MAX_PATH]; sprintf(add, "%s/%s", ProjPath, walking->d_name);
            FILE *f = fopen("/home/pardis/bash.sh", "w");
            fprintf(f, "cp -r %s %s", add, newEntry);
            fclose(f);
            chmod("/home/pardis/bash.sh", 0x777);
            system("/home/pardis/bash.sh");
            remove("/home/pardis/bash.sh");
            f = fopen("/home/pardis/bash.sh", "w");
            fprintf(f, "rm -r %s", add);
            fclose(f);
            chmod("/home/pardis/bash.sh", 0x777);
            system("/home/pardis/bash.sh");
            remove("/home/pardis/bash.sh");
        }
    }
    closedir(WorDir);
    char commitAddress[MAX_PATH]; sprintf(commitAddress, "%s/.neogit/commits/%s", ProjPath, ID);
    DIR *ComDir = opendir(commitAddress);
    while ((walking = readdir(ComDir)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt")) {
            char add[MAX_PATH]; sprintf(add, "%s/%s", commitAddress, walking->d_name);
            FILE *f = fopen("/home/pardis/bash.sh", "w");
            fprintf(f, "cp -r %s %s", add, ProjPath);
            fclose(f);
            chmod("/home/pardis/bash.sh", 0x777);
            system("/home/pardis/bash.sh");
            remove("/home/pardis/bash.sh");
        }
    }
    closedir(ComDir);
    Update_Current_Commit_ID(ProjPath, ID);
    char HEAD[100];
    char headPath[MAX_PATH]; sprintf(headPath, "%s/.neogit/head.txt", ProjPath);
    FILE *headFile = fopen(headPath, "r"); 
    fgets(HEAD, sizeof(HEAD), headFile);
    HEAD[strlen(HEAD) - 1] = '\0';
    fclose(headFile);
    char info[MAX_PATH]; sprintf(info, "%s/info.txt", newEntry);
    char infWrite[1000];
    if (hasMessage) sprintf(infWrite, "%s:%s:%s", HEAD, Message, ID);
    else sprintf(infWrite, "%s:-:%s", HEAD, ID);
    Create(info, infWrite, 1);
    return;
}

void StashList(char *ProjPath) {
    char StashPath[MAX_PATH];
    sprintf(StashPath, "%s/.neogit/.stash", ProjPath);
    DIR *stash = opendir(StashPath);
    struct dirent *walking;
    while ((walking = readdir(stash)) != NULL) {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) {
            char infoPath[1024]; sprintf(infoPath, "%s/%s/info.txt", StashPath, walking->d_name);
            FILE *info = fopen(infoPath, "r");
            char line[1000];
            fgets(line, 1000, info);
            strtok(line, ":");
            printf("Stash Entry %s Stashed in Branch %s", walking->d_name, line);
            char *message = strtok(NULL, ":\r");
            printf(" and Message = %s\n", message);
            fclose(info);
        }
    }
    closedir(stash);
    return;
}

int WalkForStash(char *mainPath, char *curPath, char *secPath, int mode) {
    DIR *com = opendir(curPath);
    struct dirent *entry;
    bool flag = true;
    while ((entry = readdir(com)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit")) {
            strcat(curPath, "/"); strcat(curPath, entry->d_name);
            flag = WalkForStash(mainPath, curPath, secPath, mode);
            while (curPath[strlen(curPath) - 1] != '/') curPath[strlen(curPath) - 1] = '\0';
            curPath[strlen(curPath) - 1] = '\0';
        }
        else if (entry->d_type != DT_DIR && strcmp(entry->d_name, "info.txt")) {
            strcat(curPath, "/"); strcat(curPath, entry->d_name);
            char second[MAX_PATH]; sprintf(second, "%s%s", secPath, curPath + strlen(mainPath));
            if (mode == 1) {
                if (access(second, F_OK) == 0) Diff(curPath, second, 1, 100000, 1, 100000);
                else {
                    printf("%s is only in Stash and not in Commit!\n\n", entry->d_name);
                    FILE *f = fopen(curPath, "r");
                    char line[1000];
                    while (fgets(line, sizeof(line), f) != NULL) printf("%s", line);
                    fclose(f);
                    printf("\n\n");
                }
            }
            else if (mode == 0 && access(second, F_OK) != 0) {
                printf("%s is only in Commit and not in Stash\n\n", entry->d_name);
                FILE *f = fopen(curPath, "r");
                char line[1000];
                while (fgets(line, sizeof(line), f) != NULL) printf("%s", line);
                fclose(f);
                printf("\n\n");
            }
            else if (mode == 2) {
                if (access(second, F_OK) == 0) if (Diff(curPath, second, 1, 100000, 1, 100000) == 0) flag = false;
            }
            while (curPath[strlen(curPath) - 1] != '/') curPath[strlen(curPath) - 1] = '\0';
            curPath[strlen(curPath) - 1] = '\0';
        }
    }
    closedir(com);
    return flag;
}

void StashShow(char *index, char *ProjPath) {
    char StashAdd[MAX_PATH]; sprintf(StashAdd, "%s/.neogit/.stash/%s", ProjPath, index);
    char StashInfo[MAX_PATH]; sprintf(StashInfo, "%s/info.txt", StashAdd);
    char CommitID[MAX_PATH];
    FILE *info = fopen(StashInfo, "r");
    char line[MAX_PATH];
    fgets(line, sizeof(line), info);
    strtok(line, ":");
    strtok(NULL, ":");
    strcpy(CommitID, strtok(NULL, "\r"));
    fclose(info);
    char ComAdd[MAX_PATH]; sprintf(ComAdd, "%s/.neogit/commits/%s", ProjPath, CommitID);
    char tmpStash[MAX_PATH]; strcpy(tmpStash, StashAdd);
    WalkForStash(StashAdd, tmpStash, ComAdd, 1);
    char tmpCom[MAX_PATH]; strcpy(tmpCom, ComAdd);
    WalkForStash(ComAdd, tmpCom, StashAdd, 0);
    return;
}

void StashPop(char *index, char *ProjPath) {
    char StashAdd[MAX_PATH]; sprintf(StashAdd, "%s/.neogit/.stash/%s", ProjPath, index);
    char tmpStashAdd[MAX_PATH]; strcpy(tmpStashAdd, StashAdd);
    if (WalkForStash(StashAdd, tmpStashAdd, ProjPath, 2)) {
        DIR *WorDir = opendir(ProjPath);
        struct dirent *walking;
        while ((walking = readdir(WorDir)) != NULL) {
            if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit")) {
                char add[1024]; sprintf(add, "%s/%s", ProjPath, walking->d_name);
                FILE *f = fopen("/home/pardis/bash.sh", "w");
                fprintf(f, "rm -r %s", add);
                fclose(f);
                chmod("/home/pardis/bash.sh", 0x777);
                system("/home/pardis/bash.sh");
                remove("/home/pardis/bash.sh");
            }
        }
        closedir(WorDir);
        DIR *StashDir = opendir(StashAdd);
        while ((walking = readdir(StashDir)) != NULL) {
            if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt")) {
                char add[1024]; sprintf(add, "%s/%s", StashAdd, walking->d_name);
                FILE *f = fopen("/home/pardis/bash.sh", "w");
                fprintf(f, "cp -r %s %s", add, ProjPath);
                fclose(f);
                chmod("/home/pardis/bash.sh", 0x777);
                system("/home/pardis/bash.sh");
                remove("/home/pardis/bash.sh");
            }
        }
        closedir(StashDir);
        FILE *f = fopen("/home/pardis/bash.sh", "w");
        fprintf(f, "rm -r %s", StashAdd);
        fclose(f);
        chmod("/home/pardis/bash.sh", 0x777);
        system("/home/pardis/bash.sh");
        remove("/home/pardis/bash.sh");
        char StashAddress[MAX_PATH]; sprintf(StashAddress, "%s/.neogit/.stash", ProjPath);
        int idx = 0;
        for (int i = 0; i < strlen(index); i++) {
            idx *= 10;
            idx += index[i] - '0';
        }
        DIR *stash = opendir(StashAddress);
        while ((walking = readdir(stash)) != NULL) {
            if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..")) {
                char add1[MAX_PATH]; sprintf(add1, "%s/%s", StashAddress, walking->d_name);
                int entry = 0;
                for (int i = 0; i < strlen(walking->d_name); i++) {
                    entry *= 10;
                    entry += (walking->d_name)[i] - '0';
                }
                if (entry < idx) continue;
                entry--;
                char add2[MAX_PATH]; sprintf(add2, "%s/%d", StashAddress, entry);
                rename(add1, add2);
            }
        }
    }
    else printf("You can't pop the stash entry because of the above conflicts. Please resolve them and try again!\n");
    return;
}

void Hook_list() {
    printf("\n");
    printf("todo_check\n");
    printf("eof_blank_space\n");
    printf("balance_braces\n");
    printf("format_check\n");
    printf("file_size_check\n");
    printf("character-limit\n\n");
    return;
}

void Applied_Hook(char *ProjPath) {
    char hookpath[MAX_PATH]; sprintf(hookpath, "%s/.neogit/applied_hook", ProjPath);
    DIR *hookdir = opendir(hookpath);
    struct dirent *entry;
    while ((entry = readdir(hookdir)) != NULL) if (strncmp(entry->d_name, ".", 1)) printf("%s\n", entry->d_name);
    closedir(hookdir);
    return;
}

void AddHook(char *name, char *ProjPath) {
    char hookpath[MAX_PATH]; sprintf(hookpath, "%s/.neogit/applied_hook/%s.txt", ProjPath, name);
    FILE *f = fopen(hookpath, "w");
    fprintf(f, "%s\n", name);
    fclose(f);
}

void RemoveHook(char *name, char *ProjPath) {
    char hookpath[MAX_PATH]; sprintf(hookpath, "%s/.neogit/applied_hook/%s.txt", ProjPath, name);
    remove(hookpath);
    return;
}

void TODO_check(const char *filename, int mode) {
    if (!strstr(filename, ".c") && !strstr(filename, ".cpp") && !strstr(filename, ".txt")) {
        if (mode) {
            printf("\"todo_check\".................................................................................." _SGR_CYANF "SKIPPED\n" _SGR_RESET);
            return;
        }
    }
    FILE *file = fopen(filename, "r");
    int todoCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, "TODO") != NULL) {
            todoCount++;
        }
    }
    fclose(file);
    if (todoCount > 0) {
        if (mode)
            printf("\"todo_check\".................................................................................." _SGR_REDF "FAILED\n" _SGR_RESET);
        checkHook = false;
        return;
    }
    else {
        if (mode)
            printf("\"todo_check\".................................................................................." _SGR_GREENF "PASSED\n" _SGR_RESET);
        return;
    }
}

void EOF_blank_space(char *Filename, int mode) {
    if (!strstr(Filename, ".c") && !strstr(Filename, ".cpp") && !strstr(Filename, ".txt")) {
        if (mode) {
            printf("\"eof-blank-space\".................................................................................." _SGR_CYANF "SKIPPED\n" _SGR_RESET);
            return;
        }
    }
    FILE *file = fopen(Filename, "r");
    fseek(file, -1, SEEK_END);
    int lastChar = fgetc(file);
    fclose(file);
    if (lastChar == '\n' || lastChar == ' ' || lastChar == '\t') {
        if (mode)
            printf("\"eof-blank-space\".................................................................................." _SGR_REDF "FAILED\n" _SGR_RESET);
        checkHook = false;
        return;
    }
    else {
        if (mode)
            printf("\"eof-blank-space\".................................................................................." _SGR_GREENF "PASSED\n" _SGR_RESET);
        return;
    }
}

void balance_braces(char *filename, int mode) {
    if (!strstr(filename, ".c") && !strstr(filename, ".cpp") && !strstr(filename, ".txt")) {
        if (mode) {
            printf("\"balance-braces\".................................................................................." _SGR_CYANF "SKIPPED\n" _SGR_RESET);
            return;
        }
    }
    FILE *file = fopen(filename, "r");
    int openBraces = 0, closeBraces = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '{') openBraces++;
        else if (c == '}') closeBraces++;
    }
    fclose(file);
    if (openBraces == closeBraces) {
        if (mode)
            printf("\"balance-braces\".................................................................................." _SGR_GREENF "PASSED\n" _SGR_RESET);
        return;
    }
    else {
        if (mode)
            printf("\"balance-braces\".................................................................................." _SGR_REDF "FAILED\n" _SGR_RESET);
        checkHook = false;
        return;
    }
}

void Format_check(char *Filename, int mode) { // check
    if (!strstr(Filename, ".c") && !strstr(Filename, ".cpp") && !strstr(Filename, ".txt") && !strstr(Filename, ".mp4") && !strstr(Filename, ".mp3") && !strstr(Filename, ".wav") && !strstr(Filename, ".png")) {
        if (mode) {
            printf("\"format-check\".................................................................................." _SGR_CYANF "SKIPPED\n" _SGR_RESET);
            return;
        }
    }
    const char *validFormats[] = {".cpp", ".c", ".txt", ".mp4", ".wav", ".mp3", ".png"};
    int isValid = 0;
    for (int i = 0; i < 7; i++) {
        if (strstr(Filename, validFormats[i]) != NULL) {
            isValid = 1;
            break;
        }
    }
    if (isValid) {
        if (mode)
            printf("\"format-check\".................................................................................." _SGR_GREENF "PASSED\n" _SGR_RESET);
        return;
    }
    else {
        if (mode)
            printf("\"format-check\".................................................................................." _SGR_REDF "FAILED\n" _SGR_RESET);
        checkHook = false;
        return;
    }
}

void FileSize_check(char *Filename, int mode) {
    FILE *file = fopen(Filename, "rb");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fclose(file);
    if (fileSize < 5 * 1024 * 1024) {
        if (mode)
            printf("\"file-size-check\".................................................................................." _SGR_GREENF "PASSED\n" _SGR_RESET);
        return;
    }
    else {
        if (mode)
            printf("\"file-size-check\".................................................................................." _SGR_REDF "FAILED\n" _SGR_RESET);
        checkHook = false;
        return;
    }
}

void Character_check(char *filename, int mode) {
    if (!strstr(filename, ".c") && !strstr(filename, ".cpp") && !strstr(filename, ".txt")) {
        if (mode)
        {
            printf("\"character-limit\".................................................................................." _SGR_CYANF "SKIPPED\n" _SGR_RESET);
            return;
        }
    }
    FILE *file = fopen(filename, "r");
    int Count = 0;
    int c;
    while ((c = fgetc(file)) != EOF) Count++;
    fclose(file);
    if (Count > 20000) {
        if (mode)
            printf("\"character-limit\".................................................................................." _SGR_REDF "FAILED\n" _SGR_RESET);
        checkHook = false;
        return;
    }
    else {
        if (mode)
            printf("\"character-limit\".................................................................................." _SGR_GREENF "PASSED\n" _SGR_RESET);
        return;
    }
}

void RunHook(char *ProjPath, char *Filename, int mode) {
    char HookAddress[MAX_PATH]; sprintf(HookAddress, "%s/.neogit/applied_hook", ProjPath);
    DIR *HookDir = opendir(HookAddress);
    struct dirent *entry;
    printf("%s:\n", Filename);
    while ((entry = readdir(HookDir)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit")) {
            if (!strcmp(entry->d_name, "todo-check.txt")) TODO_check(Filename, mode);
            if (!strcmp(entry->d_name, "eof-blank-space.txt")) EOF_blank_space(Filename, mode);
            if (!strcmp(entry->d_name, "balance-braces.txt")) balance_braces(Filename, mode);
            if (!strcmp(entry->d_name, "format-check.txt")) Format_check(Filename, mode);
            if (!strcmp(entry->d_name, "file-size-check.txt")) FileSize_check(Filename, mode);
            if (!strcmp(entry->d_name, "character-limit.txt")) Character_check(Filename, mode);
        }
    }
}

void Launch_pre(char *ProjPath, int mode) {
    char StagePath[MAX_PATH]; sprintf(StagePath, "%s/.neogit/.staged", ProjPath);
    DIR *stage = opendir(StagePath);
    struct dirent *entry;
    while ((entry = readdir(stage)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit") && entry->d_type != DT_DIR) {
            char FileName[MAX_PATH]; sprintf(FileName, "%s/.neogit/.staged/%s", ProjPath, entry->d_name);
            RunHook(ProjPath, FileName, mode);
        }
    }
    closedir(stage);
    return;
}