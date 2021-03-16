#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#define SIZE 50
char* pointer_on_memory;
//kazdy zvysok pod 8 pripocitat k predchadzajucemu

//x/50db region
//x/100db region



void addblock(char* p, int len)
{
    int new_block_size = ((len + 1) >> 1) << 1; // zvysi sa o  o 1 a zarovna sa  (na 2)
    int old_block_size = *(int*)p & ~0x1; // zamaskujem najnizsi bit
    *(int*)p = new_block_size | 0x1; // nastavi sa  nova dlzku ako hlavicku
    *(int*)(p + new_block_size - 4) = new_block_size | 0x1; // nastavi sa  nova dlzku ako paticka

    //v nasledujucom if a else sa kontroluje ci zvysna velkost co ostane po alokovani je vacsia ako 8
    //pretoze to je minimum na hlavicku a paticku a ak nieje tak sa to pripocita k alokovanemu bloku
    //ak je
    if (new_block_size <= old_block_size)
    {

        if (old_block_size - new_block_size > 8)
        {

//ak je zvysok vacsi jak 8 tak sa vytvori samostatny volny blok
            *(int*)(p + new_block_size) = (old_block_size - new_block_size);
        }

        else
        {
            //inak sa to pripocita ku bloku co sa prave alokuje

            *(int*)p = (new_block_size + (old_block_size - new_block_size)) | 0x1;
            *(int*)(p + new_block_size - 4 + old_block_size - new_block_size) = (new_block_size + (old_block_size - new_block_size)) | 0x1;
        }
    }
    else
    {
        if (new_block_size - old_block_size > 8)
        {
            //ak je zvysok vacsi jak 8 tak sa vytvori samostatny volny blok
            *(int*)(p + new_block_size) = (new_block_size - old_block_size);
        }
        else
        {
            //inak sa to pripocita ku bloku co sa prave alokuje

            *(int*)p = (new_block_size + (new_block_size - old_block_size)) | 0x1;
            *(int*)(p + new_block_size - 4 + new_block_size - old_block_size) = (new_block_size + (new_block_size - old_block_size)) | 0x1;
        }
    }
    memset(p + 4, 4, len - 8);
}


//uvolnenie bloku
void free_block(char* p)
{

    //ak idem uvolnit blok ktory nieje prvy ani posledny
    if ((*(int*)(p - 4)) != (*(int*)pointer_on_memory) && (p + (*(int*)p)) <= pointer_on_memory + ((*(int*)pointer_on_memory) - 1))
    {
        //zarovnam o 1 B menej na hlavicke a paticke
        *(int*)p = *(int*)p & ~0x1;
        *(int*)(p + (*(int*)p - 4)) = *(int*)(p + (*(int*)p - 4)) & ~0x1;

        //ak je aj blok za tymto blokom volny---funguje
        if (((*(int*)(p - 4)) & 0x1) == 1 && (((*(int*)(p + *(int*)p)) & 0x1) == 0))
        {
            // printf("\n\n vyuzivam ak je aj blok za tymto blokom volny- \n\n}");
            *(int*)p = *(int*)p + *(int*)(p + *(int*)p);
            *(int*)(p + (*(int*)p - 4)) = *(int*)p;
        }



        //ak je aj blok pred tymto blokom volny
        else if (((*(int*)(p - 4)) & 0x1) == 0 && (((*(int*)(p + (*(int*)p))) & 0x1) == 1))
        {
            // printf("\n\n vyuzivam ak je aj blok pred tymto blokom volny- \n\n}");
            *(int*)(p - *(int*)(p - 4)) = *(int*)(p - 4) + *(int*)p;
            *(int*)(p + (*(int*)p - 4)) = *(int*)(p - 4) + *(int*)p;


        }


        //ak je aj blok pred a aj blok po tomto bloku volne
        else if (((*(int*)(p - 4)) & 0x1) == 0 && (((*(int*)(p + (*(int*)p))) & 0x1) == 0))
        {
            //  printf("\n\n vyuzivam  ak je aj blok pred a aj blok po tomto bloku volne \n\n}");
            *(int*)(p - *(int*)(p - 4)) = *(int*)(p - 4) + *(int*)p + *(int*)(p + *(int*)p);
            *(int*)(p + (*(int*)p + *(int*)(p + *(int*)p)) - 4) = *(int*)(p - 4) + *(int*)p + *(int*)(p + *(int*)p);

        }

    }
    //ak sa jedna o prvy alebo posledny blok
    else
    {
        *(int*)p = *(int*)p & ~0x1;
        *(int*)(p + (*(int*)p - 4)) = *(int*)(p + (*(int*)p - 4)) & ~0x1;
        //ak sa jedna o prvy blok
        if ((*(int*)(p - 4)) == (*(int*)pointer_on_memory))
        {
            //ak je aj blok za tymto blokom volny---funguje
            if ((((*(int*)(p + *(int*)p)) & 0x1) == 0))
            {
                //  printf("\n\n vyuzivam ak je aj blok za tymto blokom volny- \n\n}");
                *(int*)p = *(int*)p + *(int*)(p + *(int*)p);
                *(int*)(p + (*(int*)p - 4)) = *(int*)p;
            }
        }
//ak sa jedna o posledny blok
        else if ((p + (*(int*)p)) >= pointer_on_memory + ((*(int*)pointer_on_memory) - 1))
        {
            if (((*(int*)(p - 4)) & 0x1) == 0)
            {
                // printf("\n\n vyuzivam ak je aj blok pred tymto blokom volny- \n\n}");
                *(int*)(p - *(int*)(p - 4)) = *(int*)(p - 4) + *(int*)p;
                *(int*)(p + (*(int*)p - 4)) = *(int*)(p - 4) + *(int*)p;

            }
        }
    }


}
//funkcia ktora by mala fungovat analogicky ako malloc z kniznice stdlib
void* memory_alloc(unsigned int size)
{
    if (size % 2 == 1)
    {
        size = size + 1;    //ak parameter size je neparny tak sa zmeni jeho hodnota o 1 vacsia
    }
    int len = size + 8;  // velkost ktoru bude zaberat blok volnej pamate spolu s hlavickou a patickou


    char* support_pointer = (pointer_on_memory + 4);  //pomocny pointer, ktory ukazuje na zaciatok pamate + 4 byte kde je ulozena velkost pamate -4

    while ((support_pointer < ((pointer_on_memory + 4) + *(int*)(pointer_on_memory)-5)) && ((*(int*)support_pointer & 1) || (*(int*)support_pointer < len)))
    {
        //cyklus pokial niesom na konci a zaroven najdeny blok nieje uz prideleny a priliš malý

        support_pointer = support_pointer + *((int*)(support_pointer)) - 1;//posuniem sa na hlavicku dalsieho bloku
    }

    if (*(int*)support_pointer >= len && (support_pointer < ((pointer_on_memory + 4) + *(int*)(pointer_on_memory)-5)))
    {
        addblock(support_pointer, len);//pomocna funkcia na pridelenie bloku

        return (support_pointer + 4);//vraciam pointer na zaciatok volnej alokovanej pamate
    }
    else
    {
        //printf("\n \n nepodarilo sa alokovat pamat\n} ");
        return NULL;//ak sa nepodarilo alokovat vraciam NULL

    }

}
//funkcia na uvolnenie pamati
int memory_free(void * valid_ptr)
{

    valid_ptr = (char*)valid_ptr - 4;     //posuniem si pointer na volnu pamat o - 4 byte aby dany pointer ukazoval na hlavicku s hodnotou danej volnej pamate



    // ak ten pointer ukazuje zazracne na uplne zaciatok na prve 4 byte kde je ulozena velkost tak returne 1 lebo uvolnenie daneho bloku by pokaslalo cely program

    if ((*(int*)(valid_ptr)) == (*(int*)pointer_on_memory))
    {
        return 1;
    }

    char* help = (char*)valid_ptr;
    free_block(help);   //volanie pomocnej funkcie na uvolenie daneho bloku
    return 0; // ak sa uvolnilo tak funkcia vracia 0
}
//funkcia na pozretie ci je pointer ktory pride do funkcie platny alebo nie, ak je plany vraciame 1 ak nie tak 0
int memory_check(void* ptr)
{
    if (ptr == NULL)
        return 0;

    return 1;
}
//funkcia na prvotnu nastavenie pola ohladom toho aby som vedel s akou velkostou tohto daneho pola pracujem a nastavenie globalneho pointera na zaciatok od kade pracujem
void memory_init(void* ptr, unsigned int size)
{
    if (size <= 8)
    {
        exit(1);//ak je velkost mensia rovna 8 tak skonci sa mi program
    }
    //ak mi pride nepárna velkost pola, znizim o jedna na parnu inak by mi to robilo problemy
    if (size % 2 == 1)
    {
        size = size - 1;
    }

    pointer_on_memory = (char*)ptr;     //ukazem globalny pointer na zaciatok pola

    //na prve 4 byte zaciatku pola sa ulozi velkost s akou sa bude pracovat v tom poli;
    int* tmp = (int*)pointer_on_memory;
    *tmp = size;

    //na 4 byte od zaciatku pola si na 4 byte sa ulozi velkost s ktorou sa bude  pracovat minus prve 4 byte
    int* tmp1 = (int*)(pointer_on_memory + 4);
    *tmp1 = size - 4;


    //pointer_on_memory[size-1]= -75;

}



void tester(char* region, int min_block, int max_block, int minMemory, int maxMemory)
{

    char* pointers[14000]; // pole pointerov
    int allocated_nofragmentation = 0; //allokovane bez fragmentacie
    int allocated_fragmentation = 0;  //alokovane s fragmentaciou
    int requested_allocations = 0;   // pocet kolko bolo pozadavok na alokaciu
    int accepted_allocations = 0;   // pocet kolko z pozadovanych alokacii sa realne alokovalo
    int random_block = 0;         //premena do ktorej sa bude ukladat nahodna velkost bloku z intervalu <min_block,max_block>
    int i = 0;
    memset(region, 0, 100000);
    int  memory=(rand() % (maxMemory - minMemory + 1)) + minMemory;
    memory_init(region + 500, memory);



    while (allocated_nofragmentation <= (memory - min_block))  //pokial tam je miesto aspon na blok s minimalnou velkosou
    {
        random_block = (rand() % (max_block - min_block + 1)) + min_block; //nahodna velkost bloku z intervalu <min_block,max_block>
        if (allocated_nofragmentation + random_block > memory)
        {
            continue;
        }
        allocated_nofragmentation += random_block; //pripocet pamate
        requested_allocations++;  //pripocet k poctu pozadovanych alokacii
        pointers[i] = (char*)memory_alloc(random_block);
        if (pointers[i])
        {
            accepted_allocations++; //pripocet k poctu prijatych alokacii
            allocated_fragmentation += random_block;
            i++;
        }

    }


    for (int x = 0; x < i; x++)
    {
        if (memory_check(pointers[x]))  //ak je platny pointer tak ho uvolnim
        {
            memory_free(pointers[x]);
        }
        else
        {
            printf("\nWrong memory check\n");
        }
    }
    memset(region + 500, 0, memory);
    for (int j = 0; j < 100000; j++)
    {
        if (region[j] != 0)
        {
            for (int x = 0; x < memory + 1000; x++)
            {
                printf("suradnice je %d, a hodnota %d\n", x, region[x]);
            }
            region[j] = 0;
            printf("som mimo pamat %d\n", j - 500);
        }
    }
    double sum = ((double)accepted_allocations / (double)requested_allocations) * 100;
    double sum_bytes = ((double)allocated_fragmentation / (double)allocated_nofragmentation) * 100;
    printf("vyuzivam velkost blokov %d %d pre velkost pamate %d bytes: alokovane %.2f%% blocks (%.2f%% bytes).\n", min_block, max_block, memory, sum, sum_bytes);


}
void tester_bonus(char* region, int min_block, int max_block, int memory)
{
    int support_array[14000]= {0};
    char* pointers[14000]; // pole pointerov
    int allocated_nofragmentation = 0; //allokovane bez fragmentacie
    int allocated_fragmentation = 0;  //alokovane s fragmentaciou
    int requested_allocations = 0;   // pocet kolko bolo pozadavok na alokaciu
    int accepted_allocations = 0;   // pocet kolko z pozadovanych alokacii sa realne alokovalo
    int random_block = 0;         //premena do ktorej sa bude ukladat nahodna velkost bloku z intervalu <min_block,max_block>
    int i = 0;
    memset(region, 0, 100000);

    memory_init(region + 500, memory);

    int random=0;
// TESTOVANIE PRVA CAST - TESTOVANIE iBA ALLOC A NASLEDNE FREE A CI SOM NEPRETIEKOL
    while (allocated_nofragmentation <= (memory - min_block))  //pokial tam je miesto aspon na blok s minimalnou velkosou
    {
        random_block = (rand() % (max_block - min_block + 1)) + min_block; //nahodna velkost bloku z intervalu <min_block,max_block>
        if (allocated_nofragmentation + random_block > memory)
        {
            continue;
        }
        allocated_nofragmentation += random_block; //pripocet pamate
        pointers[i] = (char*)memory_alloc(random_block);
        if (pointers[i])
        {

            i++;
        }

    }
    /*
    for (int i = 450; i < memory +550; i++)
    {
        printf("pred free aaaaaaaaaaaadresa 0x%p s hodnotou %d v poradi %d \n ", pointer_on_memory + i, region[i], i);
    }
*/

    for (int x = 0; x < i; x++)
    {

        if (memory_check(pointers[x]))  //ak je platny pointer tak ho uvolnim
        {
         //   printf("\nafdsaf\n");
            memory_free(pointers[x]);
        }
        else
        {
            printf("\nZly memory_check\n");
        }

    }
    /*
    for (int i = 450; i < memory +550; i++)
    {
        printf("po free aaaaaaaaaaaadresa 0x%p s hodnotou %d v poradi %d \n ", pointer_on_memory + i, region[i], i);
    }*/

    memset(region + 500, 0, memory);
    for (int j = 0; j < 100000; j++)
    {
        if (region[j] != 0)
        {
            for (int x = 0; x < memory + 1000; x++)
            {
                printf("suradnice je %d, a hodnota %d\n", x, region[x]);
            }
            region[j] = 0;
            printf("som mimo pamat %d\n", j - 500);
        }
    }

    memory_init(region + 500, memory);
    char* pointers1[14000];
   // printf("\n\n\n\nabcxvasva\n\n\n\n");
//TESTOVANIE DRUHA CAST OPAKOVANE ALOKOVANIE A FREE

    for(int y =0; y<10; y++)
    {
        allocated_nofragmentation=0;
        i=0;

        memset(pointers1, 0, memory+1000);
        while (allocated_nofragmentation <= (memory - min_block))  //pokial tam je miesto aspon na blok s minimalnou velkosou
        {
            random_block = (rand() % (max_block - min_block + 1)) + min_block; //nahodna velkost bloku z intervalu <min_block,max_block>
            if (allocated_nofragmentation + random_block > memory)
            {
                continue;
            }
            allocated_nofragmentation += random_block; //pripocet pamate
            pointers1[i] = (char*)memory_alloc(random_block);
            if (pointers1[i])
            {

                i++;
            }
        }
        /*
        for (int i = 450; i < memory +550; i++)
        {
            printf("pred free aaaaaaaaaaaadresa 0x%p s hodnotou %d v poradi %d \n ", pointer_on_memory + i, region[i], i);
        }*/

        for (int x = 0; x < i; x++)
        {

            if (memory_check(pointers1[x]))  //ak je platny pointer tak ho uvolnim
            {
              //  printf("\nafdsaf\n");
                memory_free(pointers1[x]);
            }
            else
            {
                printf("\nZly memory_check\n");
            }
        }
      /*  for (int i = 450; i < memory +550; i++)
        {
            printf("po free aaaaaaaaaaaadresa 0x%p s hodnotou %d v poradi %d \n ", pointer_on_memory + i, region[i], i);
        }*/





    }
     memset(region + 500, 0, memory);
    for (int j = 0; j < 100000; j++)
    {
        if (region[j] != 0)
        {
            for (int x = 0; x < memory + 1000; x++)
            {
                printf("suradnice je %d, a hodnota %d\n", x, region[x]);
            }
            region[j] = 0;
            printf("som mimo pamat %d\n", j - 500);
        }
    }

    // TESTOVANIE TRETIA CAST ALOKOVANIE A NAHODNE FREE
    memory_init(region + 500, memory);
    char* pointers2[14000];
  // printf("\n\n\n\nabcxvasva\n\n\n\n");
//test ci to stale pojde

    for(int y =0; y<10; y++)
    {
        allocated_nofragmentation=0;
        i=0;

        memset(pointers2, 0, memory+1000);
        while (allocated_nofragmentation <= (memory - min_block))  //pokial tam je miesto aspon na blok s minimalnou velkosou
        {
            random_block = (rand() % (max_block - min_block + 1)) + min_block; //nahodna velkost bloku z intervalu <min_block,max_block>
            if (allocated_nofragmentation + random_block > memory)
            {
                continue;
            }
            allocated_nofragmentation += random_block; //pripocet pamate
            pointers2[i] = (char*)memory_alloc(random_block);
            if (pointers2[i])
            {
                support_array[i]=1;
                i++;
            }
        }
        /*
        for (int i = 450; i < memory +550; i++)
        {
            printf("pred free aaaaaaaaaaaadresa 0x%p s hodnotou %d v poradi %d \n ", pointer_on_memory + i, region[i], i);
        }
*/
        srand(time(0));
        for (int x = 0; x < i+70; x++)
        {
            random =(rand() %
                     (i -  0+ 1)) + 0;
            if(support_array[random]==1)
            {
                if (memory_check(pointers2[random])&&(support_array[random]==1))  //ak je platny pointer tak ho uvolnim
                {
                    support_array[ random]=0;
                  //  printf("\nafdsaf\n");
                    memory_free(pointers2[random]);
                }
                else
                {
                    printf("\nZly memory_check\n");
                }
            }
        }
        /*
        for (int i = 450; i < memory +550; i++)
        {
            printf("po free aaaaaaaaaaaadresa 0x%p s hodnotou %d v poradi %d \n ", pointer_on_memory + i, region[i], i);
        }
*/




    }
     memset(region + 500, 0, memory);
    for (int j = 0; j < 100000; j++)
    {
        if (region[j] != 0)
        {
            for (int x = 0; x < memory + 1000; x++)
            {
                printf("suradnice je %d, a hodnota %d\n", x, region[x]);
            }
            region[j] = 0;
            printf("som mimo pamat %d\n", j - 500);
        }
    }



}

//DSA Zadanie 1 - Martin Saraka
// main funkcia
int main()
{
    //char region[51];


    char region[150000];//pole simulujece pamat
    char* pointer[13000];

    //testovanie screnára cislo 1.
    for (int i = 0; i < 10; i++)
    {

        tester(region, 8, 8, 50,50);
        tester(region, 8, 8, 100,100);
        tester(region, 8, 8, 200,200);
        tester(region, 8, 8, 50,200);

        tester(region, 15, 15, 50,50);
        tester(region, 15, 15, 100,100);
        tester(region, 15, 15, 200,200);
        tester(region, 15, 15, 50,200);

        tester(region, 24, 24, 50,50);
        tester(region, 24, 24, 100,100);
        tester(region, 24, 24, 200,200);
        tester(region, 24, 24, 50,200);



    }
    //Scenar 2
    for (int i = 0; i < 50; i++)
    {

        tester(region, 8, 24, 50,50);




    }
    //Scenar 3
    for (int i = 0; i < 50; i++)
    {

        tester(region, 500, 5000, 1000,10000);
        tester(region, 500, 5000, 10000,10000);




    }
    //Scenar 4
    for (int i = 0; i < 50; i++)
    {

        tester(region, 8, 50000, 1000,100000);
        tester(region, 8, 50000, 100000,100000);




    }

    //memory_init(region,5000);
    //ODKOMENTOVAT TESTER BONUS AK CHCETE DOKLADNE TESTOVANIE
    tester_bonus(region,8,8,500);



    /* for (int i = 0; i < 5000; i++)
     {
         printf("aaaaaaaaaaaadresa 0x%p s hodnotou %d v poradi %d \n ", pointer_on_memory + i, region[i], i);
     }*/



    return 0;
}
