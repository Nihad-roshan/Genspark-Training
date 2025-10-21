#include <stdio.h>
#include <stdlib.h>

typedef enum
{
    RED,
    BLACK
} color;

typedef struct rbnode
{
    int data;
    color color;
    struct rbnode *left, *right, *parent;
} rbnode;

rbnode *root = NULL;

int rotationcount = 0;

rbnode *createnode(int data)
{
    rbnode *newnode = malloc(sizeof(rbnode));
    newnode->data = data;
    newnode->color = RED;
    newnode->left = newnode->right = newnode->parent = NULL;
    return newnode;
}

void rotateleft(rbnode **root, rbnode *x)
{
    rbnode *y = x->right;
    x->right = y->left;
    if (y->left)
    {
        y->left->parent = x;
    }
    y->parent = x->parent;

    if (x->parent == NULL)
    {
        *root = y;
    }
    else if (x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;

    rotationcount++;
}

void rotateright(rbnode **root, rbnode *y)
{
    rbnode *x = y->left;
    y->left = x->right;
    if (x->right)
    {
        x->right->parent = y;
    }
    x->parent = y->parent;

    if (y->parent == NULL)
    {
        *root = x;
    }
    else if (y == y->parent->left)
    {
        y->parent->left = x;
    }
    else
    {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;

    rotationcount++;
}

void fixinsertion(rbnode **root, rbnode *newnode)
{
    rbnode *parentnode, *grandparnt, *unclenode;

    while (newnode != *root && newnode->color == RED && newnode->parent->color == RED)
    {
        parentnode = newnode->parent;
        grandparnt = parentnode->parent;

        if (parentnode == grandparnt->left)
        {
            unclenode = grandparnt->right;

            if (unclenode && unclenode->color == RED)
            {
                grandparnt->color = RED;
                parentnode->color = BLACK;
                unclenode->color = BLACK;
                newnode = grandparnt;
            }
            else
            {
                if (newnode == parentnode->right)
                {
                    newnode = parentnode;
                    rotateleft(root, newnode);
                }
                parentnode->color = BLACK;
                grandparnt->color = RED;
                rotateright(root, grandparnt);
            }
        }
        else
        {
            unclenode = grandparnt->left;

            if (unclenode && unclenode->color == RED)
            {
                grandparnt->color = RED;
                parentnode->color = BLACK;
                unclenode->color = BLACK;
                newnode = grandparnt;
            }
            else
            {
                if (newnode == parentnode->left)
                {
                    newnode = parentnode;
                    rotateright(root, newnode);
                }
                parentnode->color = BLACK;
                grandparnt->color = RED;
                rotateleft(root, grandparnt);
            }
        }
    }
    (*root)->color = BLACK;
}

void insertion(int data)
{
    rbnode *newnode = createnode(data);
    rbnode *parentnode = NULL;
    rbnode *current = root;

    while (current != NULL)
    {
        parentnode = current;
        if (data < current->data)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    newnode->parent = parentnode;

    if (parentnode == NULL)
    {
        root = newnode;
    }
    else if (data < parentnode->data)
    {
        parentnode->left = newnode;
    }
    else
    {
        parentnode->right = newnode;
    }

    fixinsertion(&root, newnode);
}

void inorder(rbnode *root)
{
    if(root == NULL)
    {
        return;
    }
    inorder(root->left);
    printf("%d(%s) ", root->data, (root->color == RED) ? "R" : "B");
    inorder(root->right);
}

void printtree(rbnode *root, int level)
{
    if (root == NULL)
        return;

    // Print right subtree
    printtree(root->right, level + 1);

    // Print current node with indentation
    for (int i = 0; i < level; i++)
        printf("    "); // 4 spaces per level
    printf("%d(%s)\n", root->data, (root->color == RED) ? "R" : "B");

    // Print left subtree
    printtree(root->left, level + 1);
}

int main()
{
    FILE *fp=fopen("data.txt", "r");

    if(!fp){

        printf("file cannot be open\n");
        return 1;
    }

    int num,ele=0;
    while(fscanf(fp, "%d", &num) ==1)
    {
        ele++;
        insertion(num);
    }
    fclose(fp);

    printf("inorder traversal:\n");
    inorder(root);

    printf("\nnumber of rotations done:%d\n",rotationcount);

    printf("Tree structure:\n");

    printtree(root,0);
    return 0;
}