#include "header.h"

int main()
{
    key_t key = ftok("chat.h", 65); // generate unique key
    if (key == -1)
    {
        die("ftok");
    }

    int msgid = msgget(key, 0666 | IPC_CREAT); // create/get message queue
    if (msgid == -1)
    {
        die("msgget");
    }

    Message msg; //struct variable
    pid_t pid = fork(); //child created

    if (pid == 0)
    {
        // Child process: receive messages from mq-2
        while (1)
        {
            if (msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_USER2_TO_USER1, 0) == -1)
            {
                die("msgrcv");
            }
            printf("[User2]: %s\n", msg.mtext);
        }
    }
    else
    {
        // Parent process: send messages to user2-mq-2
        while (1)
        {
            printf("[User1]: ");
            fflush(stdout); //It flushes (forces writing/clearing) the output buffer of a stream.
            if (!fgets(msg.mtext, MAX_TEXT, stdin))
            {
                break;
            }

            msg.mtext[strcspn(msg.mtext, "\n")] = '\0'; // remove newline  
            /*
            strcspn-> //It searches str1 (the main string) for the first occurrence of any character from str2
            It returns the index (length) of the segment at the start of str1 that contains none of the characters in str2.
            */
            msg.mtype = MSG_USER1_TO_USER2;

            if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) == -1)
            {
                die("msgsnd");
            }
        }
    }

    return 0;
}
