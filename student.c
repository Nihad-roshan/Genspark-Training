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

 void students(struct st *p,struct st *s,int num)
 {    
        printf("\nEnter the roll number:");
       // scanf("%d",&p->roll);
        if(scanf("%d",&p->roll)!=1 || p->roll<=0 || p->roll>100)
        {
            printf("\nEnter actual numbers!!\n\n");
            p->roll=-1;
            while (getchar() != '\n'); 
            return;
        }

        for(int i=0;i<num;i++)
        {
            if(s[i].roll==p->roll)
            {
            printf("\nstudent with this roll number is already present!!\n");
            p->roll=-1;
            return;
            
            }
        }
        printf("Enter the name:");
        scanf("%s",p->name);
        
        for(int i=0;i<num;i++)
        {
            if(strcmp(s[i].name,p->name)==0)
            {
            printf("\nstudent with this name  is already present!!\n");
            strcpy(p->name,"x");
            return;
            
            }
        }
        
        printf("Enter subject1 marks:");
        //scanf("%f",&p->subject1);
         if(scanf("%f",&p->subject1)!=1 || p->subject1<=0 || p->subject1>101)
        {
            printf("\nEnter actual numbers!! or between 1-100\n");
            p->subject1=-1;
            while (getchar() != '\n'); 
            return;
        }
        
        printf("Enter subject2 marks:");
       // scanf("%f",&p->subject2);
        if(scanf("%f",&p->subject2)!=1 || p->subject2<=0 || p->subject2>101)
        {
            printf("\nEnter actual numbers!! or between 1-100\n");
            p->subject2=-1;
            while (getchar() != '\n'); 
            return;
        }
        printf("Enter subject3 marks:");
        //scanf("%f",&p->subject3);
        if(scanf("%f",&p->subject3)!=1 || p->subject3<=0 || p->subject3>101)
        {
            printf("\nEnter actual numbers!! or between 1-100\n");
            p->subject3=-1;
            while (getchar() != '\n'); 
            return;
        }

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
    //scanf("%d",&numberOfstudents);
    l:
    if(scanf("%d",&numberOfstudents)!=1 || numberOfstudents<=0 || numberOfstudents>100)
        {
            printf("\nEnter actual numbers!!\nNumber of students");
            numberOfstudents=-1;
            while (getchar() != '\n'); 
            goto l;
        }
    
    

    struct st *p,*s;
    int i;
    p=malloc(sizeof(struct st )*numberOfstudents);

    for(i=0;i<numberOfstudents;i++)
    {
        printf("Enter the details of student:%d",i+1);
        students(&p[i],p,i);

        if(p[i].roll==-1)
        i--;
        if(strcmp(p[i].name,"x")==0)
        i--;
        if(p[i].subject1==-1)
        i--;
        if(p[i].subject2==-1)
        i--;
        if(p[i].subject2==-1)
        i--;
    }
    int options;
    while(1)
    {
    ll:
    printf("\n\n1-Student Details\n2-Add new student\n3-Class Average\n4-Pass Percentage\n5-Export\n6-Find students\n7-Number of students passed\n8-Exit");
    printf("\n\nEnter the option:");
    //scanf("%d",&options);
    if(scanf("%d",&options)!=1 || options<=0 || options>100)
        {
            printf("\nEnter actual numbers!!\n");
           // options=-1;
            while (getchar() != '\n'); 
            goto ll;
        }
    


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
                    students(&p[numberOfstudents-1],p,numberOfstudents-1);
                    if(p[numberOfstudents-1].roll==-1)
                    {
                        numberOfstudents--;
                    }
                    if(strcmp(p[numberOfstudents-1].name,"x")==0)
                    numberOfstudents--;

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