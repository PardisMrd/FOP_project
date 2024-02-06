#include "header.h"

// #define __DEBUG_MOD__

#ifdef __DEBUG_MOD__
void main()
{
    int argc = 5;
    char argv[][50] = {"neogit", "merge", "-b", "master", "develop"};
    chdir("/mnt/c/users/ml/desktop/testProject");
#else
void main(int argc, char *argv[]) {
#endif
    prepareANSI();

    if (argc < 2) { INVALID_CMD return; }

    else if (!strcmp(argv[1], "init")) {
        if (argc == 2) init();
        else INVALID_CMD
        return;
    }

    else if (argc == 5 && !strcmp(argv[1], "config") && !strcmp(argv[2], "-global") && !strncmp(argv[3], "alias.", 6)) {
        char aliasPath[MAX_PATH]; sprintf(aliasPath, "/home/pardis/.neogitconfig/alias");
        Create(aliasPath, NULL, 0);
        char infoPath[MAX_PATH]; snprintf(infoPath, sizeof(infoPath) + 9, "%s/%s.txt", aliasPath, argv[3] + 6);
        if (IsValid(argv[4])) {
            Create(infoPath, argv[4], 1);
            printf("Global alias created successfully!\n");
        }
        else printf("Not a valid command to create alias!\n");
        return;
    }

    else if (argc == 5 && !strcmp(argv[1], "config") && !strcmp(argv[2], "-global")) {
        if (!strcmp(argv[3], "user.name")) ChangeConfig_UserName_GLOBAL(argv[4]);
        else if (!strcmp(argv[3], "user.email")) ChangeConfig_UserEmail_GLOBAL(argv[4]);
        else INVALID_CMD
        return;
    }

    char ProjPath[MAX_PATH];
    getcwd(ProjPath, sizeof(ProjPath));
    CHECK_IF_REPO(Find_Repo(ProjPath));

    if (IsAlias(argv[1], ProjPath)) return;

    if (argc >= 4 && !strcmp(argv[1], "config")) {
        if (argc == 4 && !strncmp(argv[2], "alias.", 6)) {
            char infoPath[MAX_PATH];
            sprintf(infoPath, "%s/.neogit/alias/%s.txt", ProjPath, argv[2] + 6);
            if (IsValid(argv[3])) {
                Create(infoPath, argv[3], 1);
                printf("Alias created successfully!\n");
            }
            else printf("Not a valid command to create alias!\n");
        }
        else if (argc == 4) {
            char configPath[MAX_PATH];
            sprintf(configPath, "%s/.neogit/config.txt", ProjPath);
            if (!strcmp(argv[2], "user.name")) ChangeConfig_UserName(configPath, argv[3]);
            else if (!strcmp(argv[2], "user.email")) ChangeConfig_UserEmail(configPath, argv[3]);
            else INVALID_CMD
        }
        else INVALID_CMD
    }

    else if (argc > 2 && !strcmp(argv[1], "add")) {
        if (argc > 3 && !strcmp(argv[2], "-f")) {
            for (int i = 3; i < argc; i++) AddToStaging(argv[i], ProjPath);
            AddToRecords(ProjPath);
        }
        else if (argc > 3 && !strcmp(argv[2], "-n")) {
            int depth = 0;
            for (int i = 0; i < strlen(argv[3]); i++) {
                depth *= 10;
                depth += (argv[3][i] - '0');
            }
            char currentPath[MAX_PATH];
            getcwd(currentPath, sizeof(currentPath));
            char *lastSlash = strrchr(currentPath, '/');
            printf("Searching " _SGR_BOLD _SGR_YELLOWF "%s :\n\n"_SGR_RESET, lastSlash + 1);
            ListFolders_n(ProjPath, currentPath, depth, depth, true);
        }
        else if (argc == 3 && !strcmp(argv[2], "-redo")) {
            char Staged[MAX_PATH]; strcpy(Staged, ProjPath); strcat(Staged, "/.neogit/.staged");
            AddRedo(ProjPath, Staged);
            AddToRecords(ProjPath);
        }
        else {
            for (int i = 2; i < argc; i++)
                AddToStaging(argv[i], ProjPath);
            AddToRecords(ProjPath);
        }
    }

    else if (argc > 2 && !strcmp(argv[1], "reset")) {
        if (argc > 3 && !strcmp(argv[2], "-f")) {
            for (int i = 3; i < argc; i++) RemoveFromStaging(argv[i], ProjPath);
            AddToRecords(ProjPath);
        }
        else if (argc == 3 && !strcmp(argv[2], "-undo")) ResetUndo(ProjPath);
        else {
            for (int i = 2; i < argc; i++) RemoveFromStaging(argv[i], ProjPath);
            AddToRecords(ProjPath);
        }
    }

    else if (argc == 2 && !strcmp(argv[1], "status")) Status(ProjPath);

    else if (argc > 2 && !strcmp(argv[1], "commit")) {
        if (!strcmp(argv[2], "-m")) {
            if (argc > 4) printf("Your message has more than one word and should be in qoutations!\n");
            else if (argc < 4) printf("Your commit should have a message!\n");
            else if (strlen(argv[3]) > 72) printf("Your message can have 72 character at most!\n");
            else Commit(ProjPath, argv[3]);
        }
        else if (argc == 4 && !strcmp(argv[2], "-s")) {
            char adrs[MAX_PATH];
            sprintf(adrs, "%s/.neogit/shortcuts/%s.txt", ProjPath, argv[3]);
            FILE *f = fopen(adrs, "r");
            if (f == NULL) printf("Shortcut doesn't exist!\n");
            else {
                char message[100]; fgets(message, sizeof(message), f);
                fclose(f);
                Commit(ProjPath, message);
            }
        }
        else INVALID_CMD
    }

    else if (argc == 6 && !strcmp(argv[1], "set") && !strcmp(argv[2], "-m") && !strcmp(argv[4], "-s")) {
        SetShortcut(argv[3], argv[5], ProjPath);
        printf("Shortcut message set successfully!\n");
    }

    else if (argc == 6 && !strcmp(argv[1], "replace") && !strcmp(argv[2], "-m") && !strcmp(argv[4], "-s")) {
        char shcPath[MAX_PATH]; sprintf(shcPath, "%s/.neogit/shortcuts/%s.txt", ProjPath, argv[5]);
        if (access(shcPath, F_OK) != 0) printf("Shortcut doesn't exist!\n");
        else {
            SetShortcut(argv[3], argv[5], ProjPath);
            printf("Shortcut message reset successfully!\n");
        }
    }

    else if (argc == 4 && !strcmp(argv[1], "remove") && !strcmp(argv[2], "-s")) {
        char shcPath[MAX_PATH]; sprintf(shcPath, "%s/.neogit/shortcuts/%s.txt", ProjPath, argv[3]);
        if (access(shcPath, F_OK) != 0) printf("Shortcut doesn't exist!\n");
        else {
            remove(shcPath);
            printf("Shortcut message removed successfully!\n");
        }
    }

    else if (argc > 1 && !strcmp(argv[1], "log")) {
        if (argc == 2) Log(ProjPath, Get_commitID(ProjPath) - 10000);
        else if (argc == 4 && !strcmp(argv[2], "-n")){
            int depth = 0;
            for (int i = 0; i < strlen(argv[3]); i++) {
                depth *= 10;
                depth += (argv[3][i] - '0');
            }
            if (Get_commitID(ProjPath) - 9999 < depth) Log(ProjPath, Get_commitID(ProjPath) - 10000);
            else Log(ProjPath, depth);
        }
        else if (argc == 4 && !strcmp(argv[2], "-branch")) LogExtra(ProjPath, "Branch", argv[3]);
        else if (argc == 4 && !strcmp(argv[2], "-author")) LogExtra(ProjPath, "Username", argv[3]);
        else if (argc == 4 && !strcmp(argv[2], "-since")) LogTime(ProjPath, argv[3], '+');
        else if (argc == 4 && !strcmp(argv[2], "-before")) LogTime(ProjPath, argv[3], '-');
        else if (!strcmp(argv[2], "-search")) LogSearch(ProjPath, "Message", argv[3]);
        else INVALID_CMD
    }

    else if (!strcmp(argv[1], "branch")) {
        if (argc == 2) BranchList(ProjPath);
        else if (argc == 3) BranchCreate(argv[2], ProjPath);
        else INVALID_CMD
    }

    else if (!strcmp(argv[1], "checkout")) {
        if (argc != 3) INVALID_CMD
        else if (!strncmp(argv[2], "HEAD", 4)) CheckoutHead(argv[2], ProjPath);
        else Checkout(argv[2], ProjPath);
    }

    else if (!strcmp(argv[1], "diff")) {
        if (!strcmp(argv[2], "-f")) {
            char curDir[MAX_PATH];
            getcwd(curDir, sizeof(curDir));
            char File1[MAX_PATH]; sprintf(File1, "%s/%s", curDir, argv[3]);
            char File2[MAX_PATH]; sprintf(File2, "%s/%s", curDir, argv[4]);
            if (argc == 5) Diff(File1, File2, 1, 10000, 1, 10000);
            else if (argc == 7) {
                if (!strcmp(argv[5], "-line1")) {
                    int begin = 0;
                    char *token = strtok(argv[6], "-");
                    for (int i = 0; i < strlen(token); i++) {
                        begin *= 10;
                        begin += (token[i] - '0');
                    }
                    int end = 0;
                    token = strtok(NULL, "-");
                    for (int i = 0; i < strlen(token); i++) {
                        end *= 10;
                        end += (token[i] - '0');
                    }
                    Diff(File1, File2, begin, end, 1, 10000);
                }
                else if (!strcmp(argv[5], "-line2")) {
                    int begin = 0;
                    char *token = strtok(argv[6], "-");
                    for (int i = 0; i < strlen(token); i++) {
                        begin *= 10;
                        begin += (token[i] - '0');
                    }
                    int end = 0;
                    token = strtok(NULL, "-");
                    for (int i = 0; i < strlen(token); i++) {
                        end *= 10;
                        end += (token[i] - '0');
                    }
                    Diff(File1, File2, 1, 10000, begin, end);
                }
                else INVALID_CMD
            }
            else if (argc == 9) {
                if (!strcmp(argv[5], "-line1") && !strcmp(argv[7], "-line2")) {
                    int begin_1 = 0;
                    char *token = strtok(argv[6], "-");
                    for (int i = 0; i < strlen(token); i++) {
                        begin_1 *= 10;
                        begin_1 += (token[i] - '0');
                    }
                    int end_1 = 0;
                    token = strtok(NULL, "-");
                    for (int i = 0; i < strlen(token); i++) {
                        end_1 *= 10;
                        end_1 += (token[i] - '0');
                    }

                    int begin_2 = 0;
                    token = strtok(argv[8], "-");
                    for (int i = 0; i < strlen(token); i++) {
                        begin_2 *= 10;
                        begin_2 += (token[i] - '0');
                    }
                    int end_2 = 0;
                    token = strtok(NULL, "-");
                    for (int i = 0; i < strlen(token); i++) {
                        end_2 *= 10;
                        end_2 += (token[i] - '0');
                    }
                    Diff(File1, File2, begin_1, end_1, begin_2, end_2);
                }
                else INVALID_CMD
            }
            else INVALID_CMD
        }
        else if (!strcmp(argv[2], "-c")) {
            if (argc == 5) {
                char commit_1[MAX_PATH]; sprintf(commit_1, "%s/.neogit/commits/%s", ProjPath, argv[3]);
                char commit_2[MAX_PATH]; sprintf(commit_2, "%s/.neogit/commits/%s", ProjPath, argv[4]);
                if (access(commit_1, F_OK) != 0 || access(commit_2, F_OK) != 0) printf("Invalid commit ID!\n");
                printf(_SGR_BOLD "Files in commit %s that are not in commit %s :\n\n" _SGR_RESET, argv[3], argv[4]);
                DiffInCommits_solo(commit_1, commit_2);
                printf(_SGR_BOLD "Files in commit %s that are not in commit %s :\n\n" _SGR_RESET, argv[4], argv[3]);
                sprintf(commit_1, "%s/.neogit/commits/%s", ProjPath, argv[3]);
                sprintf(commit_2, "%s/.neogit/commits/%s", ProjPath, argv[4]);
                DiffInCommits_solo(commit_2, commit_1);
                printf(_SGR_BOLD "Comparing between common files :\n\n" _SGR_RESET);
                sprintf(commit_1, "%s/.neogit/commits/%s", ProjPath, argv[3]);
                sprintf(commit_2, "%s/.neogit/commits/%s", ProjPath, argv[4]);
                DiffInCommits_shared(commit_1, commit_2);
            }
            else INVALID_CMD
        }
        else INVALID_CMD
    }

    else if (argc >= 5 && !strcmp(argv[1], "grep") && !strcmp(argv[2], "-f") && !strcmp(argv[4], "-p")) {
        if (argc == 6) {
            char curDir[MAX_PATH];
            getcwd(curDir, sizeof(curDir));
            char File[MAX_PATH];
            sprintf(File, "%s/%s", curDir, argv[3]);
            Grep(File, argv[5], 0);
        }
        else if (argc == 7) {
            char curDir[MAX_PATH];
            getcwd(curDir, sizeof(curDir));
            char File[MAX_PATH];
            sprintf(File, "%s/%s", curDir, argv[3]);
            Grep(File, argv[5], 1);
        }
        else if (argc == 8) {
            char cmPath[MAX_PATH];
            sprintf(cmPath, "%s/.neogit/commits/%s", ProjPath, argv[7]);
            GrepCommit(cmPath, 0, argv[5], argv[3]);
        }
        else if (argc == 9) {
            char cmPath[MAX_PATH];
            sprintf(cmPath, "%s/.neogit/commits/%s", ProjPath, argv[7]);
            GrepCommit(cmPath, 1, argv[5], argv[3]);
        }
        else INVALID_CMD
    }

    else if (!strcmp(argv[1], "tag")) {
        if (argc == 2) Show_acsending(ProjPath);
        else if (!strcmp(argv[2], "-a")) {
            if (argc == 4) Tagging(argv[3], NULL, NULL, 0, ProjPath);
            else if (argc == 5 && !strcmp(argv[4], "-f")) Tagging(argv[3], NULL, NULL, 1, ProjPath);
            else if (argc == 6) {
                if (!strcmp(argv[4], "-m")) Tagging(argv[3], argv[5], NULL, 0, ProjPath);
                else if (!strcmp(argv[4], "-c")) Tagging(argv[3], NULL, argv[5], 0, ProjPath);
            }
            else if (argc == 7) {
                if (!strcmp(argv[4], "-m") && !strcmp(argv[6], "-f")) Tagging(argv[3], argv[5], NULL, 1, ProjPath);
                else if (!strcmp(argv[4], "-c") && !strcmp(argv[6], "-f")) Tagging(argv[3], NULL, argv[5], 1, ProjPath);
            }
            else if (argc == 8 && !strcmp(argv[4], "-m") && !strcmp(argv[6], "-c")) Tagging(argv[3], argv[5], argv[7], 0, ProjPath);
            else if (argc == 9 && !strcmp(argv[4], "-m") && !strcmp(argv[6], "-c") && !strcmp(argv[8], "-f")) Tagging(argv[3], argv[5], argv[7], 1, ProjPath);
        }
        else if (!strcmp(argv[2], "show")) {
            if (argc != 4) INVALID_CMD
            else TagShow(argv[3], ProjPath);
        }
        else INVALID_CMD
    }

    else if (!strcmp(argv[1], "merge")) {
        if (argc < 3) INVALID_CMD
        else if (!strcmp(argv[2], "-b")) {
            if (argc != 5) INVALID_CMD
            else Merge(argv[3], argv[4], ProjPath);
        }
    }

    else if (!strcmp(argv[1], "stash")) {
        if (argc < 3) INVALID_CMD
        else if (!strcmp(argv[2], "push")) {
            if (argc == 5) StashPush(true, argv[4], ProjPath);
            else StashPush(false, NULL, ProjPath);
        }
        else if (!strcmp(argv[2], "list")) {
            if (argc == 3) StashList(ProjPath);
            else INVALID_CMD
        }
        else if (!strcmp(argv[2], "show")) {
            if (argc != 4) INVALID_CMD
            else StashShow(argv[3], ProjPath);
        }
        else if (!strcmp(argv[2], "pop")) {
            if (argc == 4) StashPop(argv[3], ProjPath);
            else if (argc == 3) StashPop("0", ProjPath);
            else INVALID_CMD
        }
        else INVALID_CMD
    }

    else if (!strcmp(argv[1], "pre-commit")) {
        if (argc == 2) Launch_pre(ProjPath, 1);
        else if (!strcmp(argv[2], "hooks") && !strcmp(argv[3], "list")) Hook_list(ProjPath);
        else if (!strcmp(argv[2], "applied") && !strcmp(argv[3], "hooks")) Applied_Hook(ProjPath);
        else if (!strcmp(argv[2], "add") && !strcmp(argv[3], "hook")) AddHook(argv[4], ProjPath);
        else if (!strcmp(argv[2], "remove") && !strcmp(argv[3], "hook")) RemoveHook(argv[4], ProjPath);
    }

    else INVALID_CMD
    return;
}