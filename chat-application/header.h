#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAX_TEXT 128

// Message types (used by msgrcv to filter)
#define MSG_USER1_TO_USER2 1
#define MSG_USER2_TO_USER1 2

// Common message structure
typedef struct msgbuf
{
    int mtype;           // Message type (must be > 0)
    char mtext[MAX_TEXT]; // Message payload
} Message;

// Utility error handler
static void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

#endif


/*
The child process → continuously receives messages.
The parent process → continuously sends messages (takes user input).
*/