#include "header.h"

int main()
{
    key_t key = ftok("chat.h", 65); // same key
    if (key == -1)
    {
        die("ftok");
    }

    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        die("msgget");
    }

    Message msg;
    pid_t pid = fork();

    if (pid == 0)
    {
        // Child process: receive messages from user1
        while (1)
        {
            if (msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_USER1_TO_USER2, 0) == -1)
            {
                die("msgrcv");
            }
            printf("[User1]: %s\n", msg.mtext);
        }
    }
    else
    {
        // Parent process: send messages to user1
        while (1)
        {
            printf("[User2]: ");
            fflush(stdout);
            if (!fgets(msg.mtext, MAX_TEXT, stdin))
            {
                break;
            }

            msg.mtext[strcspn(msg.mtext, "\n")] = '\0'; // remove newline
            msg.mtype = MSG_USER2_TO_USER1;

            if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) == -1)
            {
                die("msgsnd");
            }
        }
    }

    return 0;
}
