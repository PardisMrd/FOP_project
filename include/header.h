#ifndef __HEADER_H__
#define __HEADER_H__

/* Include header files. */
#include "ansi_color.h"

// Primary libs
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Datatype libs
#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>
#include <ctype.h>

// System Commands and File Processing libs
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// Other libs
#include <time.h>

/* Defines */
#ifndef DT_DIR
#define DT_DIR   	0x00000010
#endif
#define MAX_PATH 4096
#define INVALID_CMD printf("Invalid Command!\n");
#define CHECK_IF_REPO(var)                                                     \
    if (!var)                                                                  \
    {                                                                          \
        printf("Not a neogit repository or any of the parent directories!\n"); \
        return;                                                              \
    }

/* Function Prototypes */

// primary functions
int Find_Repo(char *path); //
void Create(char *path, char *info, int mode); //
int IsModified(char *path1, char *path2); //
char *Get_HEAD_ID (char *ProjPath); //
char *Get_Current_Commit_ID (char *ProjPath); //
int Get_addID(char *ProjPath); //
int Get_commitID(char *ProjPath); //
void Update_Current_Commit_ID (char *ProjPath, char *newID); //
void Update_Branch(char *ProjPath, char *branch, int ID); //
void Increase_Decrease_ID (char *ProjPath, char sign, int mode); //

// Config functions
int CompareFileTimes(char *file1, char *file2); //
void ChangeConfig_UserName_GLOBAL(char *name); //
void ChangeConfig_UserName(char *infoPath, char *name); //
void ChangeConfig_UserEmail_GLOBAL(char *email); //
void ChangeConfig_UserEmail(char *infoPath, char *email); //
int IsValid(char *command); //
int IsAlias(char *mammad, char *ProjPath); /* PROBLEM */

// Init function
void init(); //

// Add functions
void AddToStaging(char *name, char *ProjPath); // check needed
void ListFolders_n(char *ProjPath, char *curAddress, int depth, int Cdepth, bool check); //
void AddToRecords(char *ProjPath); //
void AddRedo(char *ProjPath, char *Staged); //

// Reset functions
void RemoveFromStaging(char *name, char *ProjPath); //
void ResetUndo(char *ProjPath); //

// Status functions
mode_t CompareMode(char *file1, char *file2); //
void WalkOnCommit(char *ProjPath, char *curAddress); //
void WalkOnWD(char *ProjPath, char *curAddress); //
void Status(char *ProjPath); //

// Commit functions
void SetShortcut(char *message, char *shortcut, char *ProjPath); //
void CreateLn(char *ProjPath, char *Src, char *Dest); //
void Commit(char *ProjPath, char *message); //

// Log functions
void CountFiles(char *curAddress, int *num); //
int CompareCommitTime(char *filename, char *time); //
void Log(char *ProjPath, int n); //
void LogExtra(char *ProjPath, char *TYPEt, char *WORDt); //
void LogSearch(char *ProjPath, char *TYPEt, char *WORDt); //
void LogTime(char *ProjPath, char *time, char sign); //

// Branch functions
void BranchCreate(char *name, char *ProjPath); //
void BranchList(char *ProjPath); //

// Checkout functions
int WalkOnWDForCheckout(char *ProjPath, char *curAddress, char *ID); //
void Checkout(char *where, char *ProjPath);
void CheckoutHead(char *where, char *ProjPath);

// Diff functions
int IsValidLine(char *line); //
int CompareLines(char *line_1, char *line_2, char *file_1, char *file_2, int which_1, int which_2); //
int Diff(char *file1, char *file2, int begin_1, int end_1, int begin_2, int end_2); //
void DiffInCommits_solo(char *add_1, char *add_2); //
void DiffInCommits_shared(char *add_1, char *add_2); //

// Grep functions
bool matchWildcard(char *wildcard, char *word); //
void Grep(char *path, char *target, int n_flag); //
void GrepCommit(char *cmPath, int n_flag, char *target, char *file); //

// Stash functions
int WalkForStash(char *mainPath, char *curPath, char *secPath, int mode);
void StashPush(bool hasMessage, char *Message, char *ProjPath);
void StashList (char *ProjPath);
void StashShow (char *index, char *ProjPath);
void StashPop(char *index, char *ProjPath);

// Merge function
void Merge(char *branch1, char *branch2, char *ProjPath);

// Tag functions
void TagShow(char *TagName, char *ProjPath);
void Tagging(char *Tagname, char *Message, char *CommitID, int state, char *ProjPath);
void Show_acsending(char *ProjPath);

// Pre-commit functions
void Hook_list(); //
void Applied_Hook(char *ProjPath); //
void AddHook(char *name, char *ProjPath); //
void RemoveHook(char *name, char *ProjPath); //
void TODO_check(const char *filename, int mode); //
void EOF_blank_space(char *Filename, int mode); //
void balance_braces(char *filename, int mode); //
void Format_check(char *Filename, int mode); //
void FileSize_check(char *Filename, int mode); //
void Character_check(char *filename, int mode); //
void RunHook(char *ProjPath, char *Filename, int mode); //
void Launch_pre(char *ProjPath, int mode); //

#endif