#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct m
{
    int memoryid;
    void *address;
    int size;
    char type[20];
    struct m *next;
};

int id=1;

struct m *head=NULL;

void add(void *adr,unsigned int size,char *type)
{
    struct m *newnode=malloc(sizeof(struct m));
    newnode->memoryid=id++;
    newnode->address=adr;
    newnode->size=size;
    strcpy(newnode->type,type);
    newnode->next=head;
    head=newnode;

}

void *mallocAllocation(unsigned int size)
{
    void *ptr=malloc(size);

    if(ptr!=NULL)
    {
        add(ptr,size,"malloc");  //adding to structue 
    }
    return ptr;   //returned address to main
}

void *callocAllocation(unsigned int num,unsigned int size)
{
    void *ptr=calloc(num,size);

    if(ptr!=0)
    {
        add(ptr,num*size,"calloc");
    }
    return ptr;
}

int printmemory()
{
    struct m *temp=head;
    if(temp==NULL)
    {
        printf("Memory is not allocated yet!\n");
        return -1;
    }

    printf("Details:\n");

    while(temp!=NULL)
    {
        printf(" memoryid:%d | address:%p | size=%d | Type=%s\n\n",temp->memoryid,temp->address,temp->size,temp->type);
        temp=temp->next;
    }
    return 1;
}

void removememory(void *ptr)
{
    struct m *temp=head,*prev=NULL;
    while(temp!=NULL)
    {
        if(temp->address==ptr)
        {
        if(prev==NULL)
        {
            head=temp->next;
        }
        else
        {
            prev->next=temp->next;
        }

        free(temp);
        return;
        }
        prev=temp;
        temp=temp->next;
    }
    printf("Trying to free Memory which is not Tracked yet or memory which has already been freed\n ");
}

void freememory(void *ptr)
{
    if(ptr!=NULL)
    {
        removememory(ptr);
        printf("memory address:--%p-- freed\n",ptr);
        free(ptr);
    }
}

void *detailsofmemoryid(int id)
{
    struct m *temp=head;
    while(temp!=NULL)
    {
        if(temp->memoryid==id)
        {
            return temp->address;
        }
        temp=temp->next;
    }
    return NULL;
}

void exportmemory()
{
    FILE *fp=fopen("memoryfile.txt","w");
    struct m *temp=head;
    if(fp==NULL)
    {
        printf("can't open file..\n");
        return;
    }
    if(temp==NULL)
    {
        printf("No memory is allocated yet!\n");
        return;
    }
    while(temp!=NULL){

    fprintf(fp,"memoryid=%d | address=%p | size=%d | Type=%s\n",temp->memoryid,temp->address,temp->size,temp->type);
    temp=temp->next;
    }
    fclose(fp);
    printf("momeory log is exported into file!\n");
}

int main()
{
    int options;
    l1:
    while(1)
    {
    printf("1-Allocate memory\n2-Display Memory log\n3-Free Memory\n4-Export to file\n5-Exit\n");
    printf("Enter any options:");
    if(scanf("%d",&options)!=1 ||options<=0 ||options>4)
    {
        printf("Invalid input Try again..\n");
        while(getchar()!='\n');
        goto l1;
    }
    switch(options)
    {
        case 1:
        {

            int moc;
            mc:
            printf("1-malloc (memory not initialised to zero)\n2-calloc (memory initialized to zero)\nEnter:");
            if(scanf("%d",&moc)!=1 ||moc<=0 || moc>3)
            {
                printf("Invalid input Try again..\n");
                while(getchar()!='\n');
                goto mc;
            }
                int datatype;
                dt:
                printf("Enter the Datatype\n1-int 2-char 3-float 4-double\nEnter:");
                if(scanf("%d",&datatype)!=1 || datatype<=0 || datatype>5)
                {
                printf("Invalid input Try again..\n");
                while(getchar()!='\n');
                goto dt;
                }
                int size;
                sz:
                printf("Enter the size:");
                if(scanf("%d",&size)!=1)
                {
                printf("Invalid input Try again..\n");
                while(getchar()!='\n');
                goto sz;
                }
                void *a;
                if(moc==1)
                {
                if(datatype==1)
                {
                    a=mallocAllocation(size * sizeof(int));
                    printf("memory created!\n");
                }
                else if(datatype==2)
                {
                    a=mallocAllocation(size * sizeof(char));
                    printf("memory created!\n");
                }
                else if(datatype==3)
                {
                    a=mallocAllocation(size * sizeof(float));
                    printf("memory created!\n");
                }
                else if(datatype==4)
                {
                    a=mallocAllocation(size * sizeof(double));
                    printf("memory created!\n");
                }
                else
                {
                    printf("wrong option!");
                }
                }
                else if(moc==2)
                {
                    if(datatype==1)
                {
                    a=callocAllocation(size,sizeof(int));
                    printf("memory created!\n");
                }
                else if(datatype==2)
                {
                    a=callocAllocation(size,sizeof(char));
                    printf("memory created!\n");
                }
                else if(datatype==3)
                {
                    a=callocAllocation(size,sizeof(float));
                    printf("memory created!\n");
                }
                else if(datatype==4)
                {
                    a=callocAllocation(size,sizeof(double));
                    printf("memory created!\n");
                }
                else
                {
                    printf("wrong option!");
                }

                }
        }      
        case 2:
        {
            printmemory();
            break;
        }
        case 3:
        {
            int id,p=0;
            p=printmemory(); 
            printf("-----(%d)---\n",p);
            if(p==1)
            {
            printf("Enter the memoryid to be freed:");
            if(scanf("%d",&id)!=1)
            {
                printf("Invalid input!\n");
                while(getchar()!='\n');
                break;
            }

            void *ptr=detailsofmemoryid(id);
            printf("------\n");
            if(ptr!=NULL)
            {
                    freememory(ptr); 
            }
            else
            {
                printf("no such memory founded!\n");
            }
        }
            else
            {
                printf("\n");
            }
           
        break;
        }
        case 4:
        {
                exportmemory();
                break;
        }
        case 5:
        {
            printf("Exiting..\n");
            struct m *temp=head;
            while(temp!=NULL)
            {
                struct m *sub=temp->next;
                freememory(temp->address);
                free(temp);
                temp=sub;
            }
            head=NULL;
            exit(0);
        }

        default:
        {
            printf("invalid option!");
                break;
        }
    }
    }
    //int *a=mallocAllocation(5 * sizeof(int));

    //int *b=callocAllocation(10,sizeof(int));
    /*
    printf("1\n");
    printmemory();
    

   // printf("2\n");
   // freememory(a);
    printf("memory a freed\n");
    printmemory();
    

    printf("3\n");
    freememory(b);
    printf("memory b freed\n");

    printmemory();

    printf("4\n");
    printmemory();*/










    return 0;
}