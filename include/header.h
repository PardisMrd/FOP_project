#ifndef __HEADER_H__
#define __HEADER_H__

/* Include header files. */
#include "ansi_color.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <time.h>

/* Defines */
#define DEBUG(x) printf("%s\n", x);
#define BUF_SIZE 65536
#define INVALID_CMD printf("Invalid Command!\n")
#define CHECK_IF_REPO(var)                                                     \
    if (!var)                                                                  \
    {                                                                          \
        printf("Not a neogit repository or any of the parent directories!\n"); \
        return 0;                                                              \
    }

/* Function Prototypes */
mode_t CompareMode(char *file1, char *file2);
int Find_Repo(char *path);
void ChangeConfig_UserName(char *infoPath, const char name[]);
void ChangeConfig_UserEmail(char *infoPath, const char email[]);
int IsModified(char *path1, char *path2);
void ListFolders_n(char ProjPath[], char curAddress[], int depth, int Cdepth, bool check);
void CreateFile(char FileName[], char ToWrite[]);
void init();
int Get_addID(char *ProjPath);
int Get_commitID(char *ProjPath);
void Increase_Decrease_addID(char *ProjPath, char sign);
void Increase_Decrease_commitID(char *ProjPath, char sign);
void CreateDir(const char *dirPath);
int FindFile(char *SearchPath);
void AddToStaging(const char *name, char *ProjPath);
void AddToRecords(char *ProjPath);
void AddRedo(char *ProjPath, char *Staged);
void RemoveFromStaging(const char *name, char *ProjPath);
void ResetUndo(char *ProjPath);
int compareFileTimes(const char *file1, const char *file2);
void Commit(char *ProjPath, char *message);
void CreateLn(char ProjPath[], char Src[], char Dest[]);
void SetShortcut(char *message, char *shortcut, char *ProjPath);
void Log(char *ProjPath, int n);
void CountFiles(char *curAddress, int *num);
void LogExtra(char *ProjPath, char *TYPEt, char *WORDt);
void LogSearch(char *ProjPath, char *TYPEt, char *WORDt);
void LogTime(char *ProjPath, char *time, char sign);
int CompareCommitTime(const char *filename, const char *time);
void Status(char *ProjPath);
void WalkOnCommit(char *ProjPath, char *curAddress);
void WalkOnWD(char *ProjPath, char *curAddress);
void BranchCreate(char *name);
void BranchList ();
int WalkOnWDForCheckout(char *ProjPath, char *curAddress, char *ID);
void Checkout(char *where);
void CheckoutHead(char *where);
int IsValid(char *command);
void IsAlias(char *mammad, char *ProjPath);

#endif