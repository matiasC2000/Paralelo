#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#define ORDENXFILAS 0
#define ORDENXCOLUMNAS

double *A,*B,*C;
int N,T,cantTotal;
double elemento=1;
int sumas[8];

pthread_barrier_t barreras[8];
pthread_mutex_t miMutex;

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}


void *funcion(void *arg)
{
    int id = *(int *)arg;
    int sizeSection = N / T;
    int inicio = id * sizeSection;
    int limite = inicio + sizeSection;
    printf("Hilo id:%d\n", id);
    // Código que ejecutará cada hilo


    //tengo que ordenar mi sizeSection
    int size, j, iteracion=0;
    int puntI=0, puntD=0;
    int puntB;
    int finI,finD;
    int suma;
    double *temp;
    printf("Hilo id:%d  %.2f\n", id, A[0]);
    for(size = 2; size <= sizeSection; size*=2){ //limite-inicio tamanio total del vector
        for (int i = 0; i < sizeSection/size; i++)
        {
            puntI=inicio+i*size;    //punto inicio vector I
            puntB=puntI;
            finI=puntI+size/2;
            puntD=puntI+size/2;    //punto inicio vector D
            finD=inicio+i*size+size;
            while (puntI<finI && puntD<finD)
            {
                if (A[puntI]<A[puntD]){
                    B[puntB]=A[puntI];
                    puntB++;
                    puntI++;
                }else{
                    B[puntB]=A[puntD];
                    puntB++;
                    puntD++;
                }
            }
            while (puntI<finI){
                B[puntB]=A[puntI];
                puntB++;
                puntI++;
            }
            while (puntD<finD){
                B[puntB]=A[puntD];
                puntB++;
                puntD++;
            }
        }
        for (int i = inicio; i < limite; i++)
        {
            A[i]=B[i];
        }
        /*printf("id: %d size:%d\n",id,size);
        for (int i = inicio; i < limite; i++)
        {
            if(!(i%size)){
                printf("\t");
            }
            printf("%.0f ",A[i]);
        }
        printf("\n");*/
        
    }
    //tengo que ver si quedo eleguido para seguir
    //hago el merge

    int hilosRestantes = T;
    
    //sumas[id]=suma;
    if(id%T==0){
        printf("print completo Axi: %d ",id);
        for (int i = 0; i < N; i++)
        {
            printf("%.0f ",A[i]);
        }
        printf("\n");
    }

    int cantComparacion = 1;

    while ((id%cantComparacion==0) && hilosRestantes!=1)
    {
        //printf("hilo: %d, espera: %d \n",id,hilosRestantes/2);
        //pthread_barrier_wait(&barreras[id%(hilosRestantes/2)]);
        cantComparacion = cantComparacion*2;
        pthread_barrier_wait(&barreras[id/cantComparacion]);
        hilosRestantes = hilosRestantes/2;
        if(id%cantComparacion == 0 && id<hilosRestantes){
            printf("quien entro: %d \n",id);
            //codigo a ejecutar
            sizeSection = N / hilosRestantes;
            inicio = id * sizeSection;
            limite = inicio + sizeSection;

            puntI=inicio;    //punto inicio vector I
            puntB=puntI;
            finI=inicio+sizeSection/2;
            puntD=finI;    //punto inicio vector D
            finD=limite;

            printf("print restantes: %d \n",id);
            for (int i = inicio; i < limite; i++)
            {
                printf("%.0f ",A[i]);
            }
            printf("\n");
            
            
            while (puntI<finI && puntD<finD)
            {
                if (A[puntI]<A[puntD]){ 
                    B[puntB]=A[puntI];
                    puntB++;
                    puntI++;
                }else{
                    B[puntB]=A[puntD];
                    puntB++;
                    puntD++;
                }
            }
            while (puntI<finI){
                B[puntB]=A[puntI];
                puntB++;
                puntI++;
            }
            while (puntD<finD){
                B[puntB]=A[puntD];
                puntB++;
                puntD++;
            }
            for (int i = inicio; i < limite; i++)
            {
                A[i]=B[i];
            }
        }
    }
    
    //cantTotal += suma;
    //printf("suma %d: %d \n",id,suma);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    //N = atoi(argv[1]);
    //T = atoi(argv[2]);
    N = 16;
    T = 2;
    pthread_t misThreads[T];
    int threads_ids[T];
    int check=1;

    A = (double *)malloc(sizeof(double) * N);
    B = (double *)malloc(sizeof(double) * N);

    int i,j;
    double timetick;

    for (i = 0; i < T/2; i++) {                //cant de hilos que se quedan frenados
        pthread_barrier_init(&barreras[i], NULL, 2); // Inicializa cada barrera del arreglo
    }

    // Inicializar el generador de números aleatorios
    srand(time(NULL));
    for (i = 0; i < N; i++)
    {
        A[i]= (double) (rand() % 100);
    }

    for (int i = 0; i < N; i++)
    {
        printf("%.0f ",A[i]);
    }
    printf("\n");
    
    pthread_mutex_init(&miMutex, NULL);

    timetick = dwalltime();
    //llamo a los hilos
    for (int id = 0; id < T; id++)
    {
        threads_ids[id] = id;
        pthread_create(&misThreads[id], NULL, &funcion, (void *)&threads_ids[id]);
    }
    for (int id = 0; id < T; id++)
    {
        pthread_join(misThreads[id], NULL);
    }
    printf("Tiempo en segundos usando los dos en filas %f\n", dwalltime() - timetick);

    pthread_mutex_destroy(&miMutex);

    for (i = 0; i < T/2; i++) {
        pthread_barrier_destroy(&barreras[i]); // Destruye cada barrera del arreglo
    }

    //printf("\ncantTotal = %d",cantTotal);
    //printf("\ncantTotal = %d",sumas[0]);

    for (int i = 0; i < N-1; i++)
    {
        printf("%.0f ",A[i]);
        if(A[i]>A[i+1]){
            check=0;
        }
    }

    if (check)
    {
        printf("lindaaaaa\n");
    }
    else
    {
        printf("Que boludoooo\n");
    }

    return 0;
}