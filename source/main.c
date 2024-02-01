#include "header.h"

// #define __DEBUG_MOD__

#ifdef __DEBUG_MOD__
int main()
{
    int argc = 2;
    char argv[][50] = {"neogit", "status"};
    chdir("/home/saraghazavi/tmp");
#else
int main(int argc, char const *argv[])
{
#endif
    prepareANSI();

    // if (1)
    // {
    //     char ProjPath[PATH_MAX];
    //     getcwd(ProjPath, sizeof(ProjPath));
    //     CHECK_IF_REPO(Find_Repo(ProjPath));
    //     IsAlias(argv[1],ProjPath);
    // }

    if (argc >= 4 && !strcmp(argv[1], "config"))
    {
        if (argc == 5 && !strcmp(argv[2], "-global") && !strncmp(argv[3], "alias.", 6)) // --global
        {
            char *homePath = getenv("HOME");
            char aliasPath[PATH_MAX];
            snprintf(aliasPath, sizeof(aliasPath), "%s/.neogitconfig/alias", homePath);
            CreateDir(aliasPath);

            char infoPath[PATH_MAX];
            snprintf(infoPath, sizeof(infoPath) + 9, "%s/%s.txt", aliasPath, argv[3] + 6);

            if (IsValid(argv[4]))
            {
                FILE *fp = fopen(infoPath, "w");
                fprintf(fp, argv[4]);
                fclose(fp);
            }
            else
                printf("Not a valid command to create alias!\n");
        }
        else if (argc == 5 && !strcmp(argv[2], "-global")) // --global
        {
            char *homePath = getenv("HOME");
            char configPath[PATH_MAX];
            snprintf(configPath, sizeof(configPath), "%s/.neogitconfig", homePath);
            CreateDir(configPath);

            char infoPath[PATH_MAX];
            snprintf(infoPath, sizeof(infoPath) + 9, "%s/info.txt", configPath);

            if (access(infoPath, F_OK) != 0)
            {
                FILE *fp = fopen(infoPath, "w");
                if (!fp)
                {
                    perror("fopen");
                    return 1;
                }
                fprintf(fp, "name =\nemail =\n");
                fclose(fp);
            }

            if (!strcmp(argv[3], "user.name"))
                ChangeConfig_UserName(infoPath, argv[4]);
            else if (!strcmp(argv[3], "user.email"))
                ChangeConfig_UserEmail(infoPath, argv[4]);
            else
                INVALID_CMD;
        }

        else if (argc == 4 && !strncmp(argv[2], "alias.", 6))
        {
            char ProjPath[PATH_MAX];
            getcwd(ProjPath, sizeof(ProjPath));
            CHECK_IF_REPO(Find_Repo(ProjPath));
            strcat(ProjPath, "/.neogit/alias/");
            strcat(ProjPath, argv[2] + 6);
            strcat(ProjPath, ".txt");

            if (IsValid(argv[3]))
            {
                FILE *fp = fopen(ProjPath, "w");
                fprintf(fp, argv[3]);
                fclose(fp);
            }
            else
                printf("Not a valid command to create alias!\n");
        }

        else if (argc == 4)
        {
            char ProjPath[PATH_MAX];
            getcwd(ProjPath, sizeof(ProjPath));
            CHECK_IF_REPO(Find_Repo(ProjPath));
            strcat(ProjPath, "/.neogit/config.txt");

            if (access(ProjPath, F_OK) != 0)
            {
                FILE *fp = fopen(ProjPath, "w");
                if (!fp)
                {
                    perror("fopen");
                    return 1;
                }
                fprintf(fp, "name =\nemail =\n");
                fclose(fp);
            }

            if (!strcmp(argv[2], "user.name"))
                ChangeConfig_UserName(ProjPath, argv[3]);
            else if (!strcmp(argv[2], "user.email"))
                ChangeConfig_UserEmail(ProjPath, argv[3]);
            else
                INVALID_CMD;
        }
        else
            INVALID_CMD;
    }

    else if (!strcmp(argv[1], "init"))
    {
        if (argc == 2)
        {
            init();
        }
        else
            INVALID_CMD;
    }

    else if (argc > 2 && !strcmp(argv[1], "add"))
    {
        char ProjPath[PATH_MAX];
        getcwd(ProjPath, sizeof(ProjPath));
        CHECK_IF_REPO(Find_Repo(ProjPath));
        if (argc > 3 && !strcmp(argv[2], "-f"))
        {
            for (int i = 3; i < argc; i++)
                AddToStaging(argv[i], ProjPath);
            AddToRecords(ProjPath);
        }

        else if (argc > 3 && !strcmp(argv[2], "-n"))
        {
            int depth = 0;
            for (int i = 0; i < strlen(argv[3]); i++)
            {
                depth *= 10;
                depth += (argv[3][i] - '0');
            }
            char currentPath[PATH_MAX];
            getcwd(currentPath, sizeof(currentPath));
            char *lastSlash = strrchr(currentPath, '/');
            printf("Searching " _SGR_BOLD _SGR_YELLOWF "%s :\n\n"_SGR_RESET, lastSlash + 1);
            ListFolders_n(ProjPath, currentPath, depth, depth, true);
        }
        else if (argc == 3 && !strcmp(argv[2], "-redo"))
        {
            char Staged[PATH_MAX];
            strcpy(Staged, ProjPath);
            strcat(Staged, "/.neogit/.staged");
            AddRedo(ProjPath, Staged);
            AddToRecords(ProjPath);
        }
        else
        {
            for (int i = 2; i < argc; i++)
                AddToStaging(argv[i], ProjPath);
            AddToRecords(ProjPath);
        }
    }

    else if (argc > 2 && !strcmp(argv[1], "reset"))
    {
        char ProjPath[PATH_MAX];
        getcwd(ProjPath, sizeof(ProjPath));
        CHECK_IF_REPO(Find_Repo(ProjPath));
        if (argc > 3 && !strcmp(argv[2], "-f"))
        {
            for (int i = 3; i < argc; i++)
                RemoveFromStaging(argv[i], ProjPath);
            AddToRecords(ProjPath);
        }
        else if (argc == 3 && !strcmp(argv[2], "-undo"))
            ResetUndo(ProjPath);
        else
        {
            for (int i = 2; i < argc; i++)
                RemoveFromStaging(argv[i], ProjPath);
            AddToRecords(ProjPath);
        }
    }

    else if (argc > 2 && !strcmp(argv[1], "commit"))
    {
        char ProjPath[PATH_MAX];
        getcwd(ProjPath, sizeof(ProjPath));
        CHECK_IF_REPO(Find_Repo(ProjPath));
        if (!strcmp(argv[2], "-m"))
        {
            if (argc > 4)
                printf("You message should be in qoutations!\n");
            else if (argc < 4)
                printf("You commit should have a message!\n");
            else if (strlen(argv[3]) > 72)
                printf("You message can have 72 character at most!\n");
            else
                Commit(ProjPath, argv[3]);
        }
        else if (argc == 4 && !strcmp(argv[2], "-s"))
        {
            char adrs[PATH_MAX];
            sprintf(adrs, "%s/.neogit/shortcuts/%s.txt", ProjPath, argv[3]);
            FILE *f = fopen(adrs, "r");
            if (f == NULL)
                printf("Shortcut doesn't exist!\n");
            else
            {
                char message[100];
                fgets(message, sizeof(message), f);
                Commit(ProjPath, message);
            }
        }
        else
            INVALID_CMD;
    }

    else if (argc == 6 && !strcmp(argv[1], "set") && !strcmp(argv[2], "-m") && !strcmp(argv[4], "-s"))
    {
        char ProjPath[PATH_MAX];
        getcwd(ProjPath, sizeof(ProjPath));
        CHECK_IF_REPO(Find_Repo(ProjPath));
        SetShortcut(argv[3], argv[5], ProjPath);
        printf("Shortcut message set successfully!\n");
    }

    else if (argc == 6 && !strcmp(argv[1], "replace") && !strcmp(argv[2], "-m") && !strcmp(argv[4], "-s"))
    {
        char ProjPath[PATH_MAX];
        getcwd(ProjPath, sizeof(ProjPath));
        CHECK_IF_REPO(Find_Repo(ProjPath));
        char shcPath[PATH_MAX];
        sprintf(shcPath, "%s/.neogit/shortcuts/%s.txt", ProjPath, argv[5]);
        if (access(shcPath, F_OK) != 0)
            printf("Shortcut doesn't exist!\n");
        else
        {
            SetShortcut(argv[3], argv[5], ProjPath);
            printf("Shortcut message reset successfully!\n");
        }
    }

    else if (argc == 4 && !strcmp(argv[1], "remove") && !strcmp(argv[2], "-s"))
    {
        char ProjPath[PATH_MAX];
        getcwd(ProjPath, sizeof(ProjPath));
        CHECK_IF_REPO(Find_Repo(ProjPath));
        char shcPath[PATH_MAX];
        sprintf(shcPath, "%s/.neogit/shortcuts/%s.txt", ProjPath, argv[3]);
        if (access(shcPath, F_OK) != 0)
            printf("Shortcut doesn't exist!\n");
        else
        {
            remove(shcPath);
            printf("Shortcut message removed successfully!\n");
        }
    }

    else if (argc > 1 && !strcmp(argv[1], "log"))
    {
        char ProjPath[PATH_MAX];
        getcwd(ProjPath, sizeof(ProjPath));
        CHECK_IF_REPO(Find_Repo(ProjPath));
        if (argc == 4 && !strcmp(argv[2], "-n"))
        {
            int depth = 0;
            for (int i = 0; i < strlen(argv[3]); i++)
            {
                depth *= 10;
                depth += (argv[3][i] - '0');
            }
            Log(ProjPath, depth);
        }
        else if (argc == 4 && !strcmp(argv[2], "-branch"))
            LogExtra(ProjPath, "Branch", argv[3]);
        else if (argc == 4 && !strcmp(argv[2], "-author"))
            LogExtra(ProjPath, "Username", argv[3]);
        else if (argc == 4 && !strcmp(argv[2], "-since"))
            LogTime(ProjPath, argv[3], '+');
        else if (argc == 4 && !strcmp(argv[2], "-before"))
            LogTime(ProjPath, argv[3], '-');
        else if (!strcmp(argv[2], "-search"))
            for (int i = 3; i < argc; i++)
                LogSearch(ProjPath, "Message", argv[i]);
        else if (argc == 2)
            Log(ProjPath, Get_commitID(ProjPath) - 10000);
        else
            INVALID_CMD;
    }

    else if (argc == 2 && !strcmp(argv[1], "status"))
    {
        char ProjPath[PATH_MAX];
        getcwd(ProjPath, sizeof(ProjPath));
        CHECK_IF_REPO(Find_Repo(ProjPath));
        Status(ProjPath);
    }

    else if (!strcmp(argv[1], "branch"))
    {
        if (argc == 2) {
            BranchList();
        }
        else {
            BranchCreate(argv[2]);
        }
    }

    else if (!strcmp(argv[1], "checkout")) {
        if (argc != 3) INVALID_CMD;
        else if (!strncmp(argv[2], "HEAD", 4)) {
            CheckoutHead(argv[2]);
        }
        else {
            Checkout(argv[2]);
        }
    }

    else
        INVALID_CMD;
    return 0;
}