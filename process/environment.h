/*
  Name: environment.c
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 04/01/04 05:04
  Description: Handles environment strings with concurrency support
    As of the moment, implementation is on a global basis only, a per process
    implementation will be implemented in the future.
*/

typedef struct _env_strings {
char *name;
char *value;
struct _env_strings *next;
struct _env_strings *prev; 
} env_strings;

int env_busywait = 0;
env_strings *env_head = 0;

//function prototypes starts hee
char *env_getenv(const char *name,char *buf);
env_strings *env_getstring(const char *name);
void env_showenv();
int env_setenv(const char *name, const char *value, int replace);
int env_unsetenv(const char *name);
