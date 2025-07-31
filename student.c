#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 struct st
 {
    int roll;
    char name[20];
    float subject1;
    float subject2;
    float subject3;
    float sum;
    float avg;
 };

 void students(struct st *p)
 {    
        printf("\nEnter the roll number:");
        scanf("%d",&p->roll);
        printf("Eter the name:");
        scanf("%s",p->name);
        printf("Enter subject1 marks:");
        scanf("%f",&p->subject1);
        printf("Enter subject2 marks:");
        scanf("%f",&p->subject2);
        printf("Enter subject3 marks:");
        scanf("%f",&p->subject3);

        p->sum=p->subject1+p->subject2+p->subject3;
        p->avg=p->sum/3;

 }

 void display(struct st *p)
 {
     printf("Roll:%d Name:%s Subject1:%.2f Subject2:%.2f Subject3:%.2f Total:%.2f Average:%.2f\n",p->roll,p->name,p->subject1,p->subject2,p->subject3,p->sum,p->avg);

 }

 void export(struct st *p,int num)
 {
        FILE *fp=fopen("studentfile.txt","w");

       for(int i=0;i<num;i++)
    {
        fprintf(fp,"roll:%d name:%s subject1:%.2f subject2:%.2f subject3:%.2f Total:%.2f Average:%.2f\n",p[i].roll,p[i].name,p[i].subject1,p[i].subject2,p[i].subject3,p[i].sum,p[i].avg);
    }
    fclose(fp);

    printf("\nstudent details exported!");
 }

    float classaverage(struct st *p,int num)
    {
        float total=0,result;
        for(int i=0;i<num;i++)
        {
            total=total+p[i].avg;
        }
        result=total/num;
        return result;
    }

    float passPercentage(struct st *p,int num)
    {
        int count=0;
        float result;
        for(int i=0;i<num;i++)
        {
            if(p[i].sum>150)
            count++;

        }
        result=((float)count/num)*100;
        return result;
    }

 void findStudentByRoll(struct st *p,int num,int searchroll)
{
int found=0;
for(int i=0;i<num;i++)
{
    if(p[i].roll==searchroll)
    {
        found=1;
        printf("student found!!\nDetails:");
             printf("Roll:%d Name:%s Subject1:%.2f Subject2:%.2f Subject3:%.2f Total:%.2f Average:%.2f\n",p[i].roll,p[i].name,p[i].subject1,p[i].subject2,p[i].subject3,p[i].sum,p[i].avg);
             break;
    }
}
    if(found==0)
    printf("Student not found!");
}

void findStudentByName(struct st *p,int num,char *Name)
{
    int found=0;
    for(int i=0;i<num;i++)
    {
        if(strcmp(p[i].name,Name)==0)
        {
            found=1;
            printf("student found!!\nDetails:");
            printf("Roll:%d Name:%s Subject1:%.2f Subject2:%.2f Subject3:%.2f Total:%.2f Average:%.2f\n",p[i].roll,p[i].name,p[i].subject1,p[i].subject2,p[i].subject3,p[i].sum,p[i].avg);
             break;

        }
        
    }
    if(found==0)
    printf("Student not found!");
}
int numberOfStudentsPassed(struct st *p,int num)
{
    int count=0;
        for(int i=0;i<num;i++)
        {
            if(p[i].sum>150)
            count++;

        }
        
        return count;
}



 void main()
 {
    int numberOfstudents;
    printf("Enter the number of students:");
    scanf("%d",&numberOfstudents);

    struct st *p;
    int i;
    p=malloc(sizeof(struct st )*numberOfstudents);

    for(i=0;i<numberOfstudents;i++)
    {
        printf("Enter the details of student:%d",i+1);
        students(&p[i]);
    }
    int options;
    while(1)
    {
    printf("\n1-Student Details\n2-Add new student\n3-Class Average\n4-Pass Percentage\n5-Export\n6-Find students\n7-Number of students passed\n8-Exit");
    printf("\nEnter the option:");
    scanf("%d",&options);   

    switch(options)
    {
        case 1:
            {
                printf("\nstudent details:\n");
                for(i=0;i<numberOfstudents;i++)
                {
                    display(&p[i]);
                }
                break;
            }   
        case 2:
                {
                    numberOfstudents=numberOfstudents+1;
                    p=realloc(p,sizeof(struct st)*numberOfstudents);
                    printf("Enter the details of New students:");
                    students(&p[numberOfstudents-1]);
                break;
                }       
        case 3: {float classAverage;
                    classAverage=classaverage(p,numberOfstudents);
                    printf("\nclass average:%.2f",classAverage);
                    break;
                }
        case 4: 
                {
                    float passpercentage=passPercentage(p,numberOfstudents);
                    printf("\npass percentage:%.2f",passpercentage);
                    break;
                }
        case 5: {
                export(p,numberOfstudents);
                break;
                }
        case 6:{
                int op;
                printf("1-By rollnumber\n2-By name:");
                printf("\nEnter any choice:");
                scanf("%d",&op);
                if(op==1)
                {
                    int searchroll;
                    printf("Enter the Rollnumber:");
                    scanf("%d",&searchroll);

                    findStudentByRoll(p,numberOfstudents,searchroll);
                }
                else if(op==2)
                {
                    char searchname[20];
                    printf("Enter the name:");
                    scanf("%s",&searchname);

                    findStudentByName(p,numberOfstudents,searchname);

                }
                break;
                }
        case 7: { int studedntPassed=numberOfStudentsPassed(p,numberOfstudents);
                    printf("Total number Of students passed=%d",studedntPassed);
                    break;
                }

        case 8: {printf("Exiting");
                    free(p);
                    exit(0);
                }
        default:  { printf("invalid options");}
    
    }

    }
        

    //printing student details


   
    //exporting student details


 }