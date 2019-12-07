#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define O 1001



void nacitaj(char a[],FILE *f)
{
    int b;
    int i=0;
    while((b=fgetc(f))!=EOF)
    {
        a[i]=b;
        i++;
        if (i>999)
            break;
    }

}
int main()
{
    char ppole[O]="",upole[O]="";
    int c;
    while((c=fgetc(stdin))!='k')
    {

        if(c=='n')
        {
            FILE *fr;
            if((fr=fopen("sifra.txt","r"))==NULL)
            {
                printf(" Spravu sa nepodarilo nacitat\n");


            }

           else
            {
                nacitaj(ppole,fr);

            }



            if(fclose(fr)==EOF)
            {
                printf("Subor sa nepodarilo zatvorit\n");

            }
        }
       
    return 0;
}



