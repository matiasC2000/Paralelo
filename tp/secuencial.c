#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#define ORDENXFILAS 0
#define ORDENXCOLUMNAS

double *A,*B,*C;
int N,T,cantTotal;
double elemento=1;
int iguales=1;

pthread_barrier_t barreras[8];
pthread_barrier_t barreraGlobal;
pthread_mutex_t miMutex;

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}


void funcion()
{
    //tengo que ordenar mi sizeSection
    int size;
    int puntI=0, puntD=0;
    int puntB;
    int finI,finD;
    for(size = 2; size <= N/2; size*=2){ //limite-inicio tamanio total del vector
        for (int i = 0; i < N/size; i++)
        {
            puntI=i*size;    //punto inicio vector I
            puntB=puntI;
            finI=puntI+size/2;
            puntD=puntI+size/2;    //punto 0 vector D
            finD=i*size+size;
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
        for (int i = 0; i < N; i++)
        {
            A[i]=B[i];
        }
        /*printf("size:%d\n",size);
        for (int i = 0; i < N; i++)
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


    int check=1;
    //printf("print completo Axi: ");
    for (int i = 0; i < (N/2)-1; i++)
    {
        //printf("%.0f ",A[i]);
        if(A[i]>A[i+1]){
            check=0;
        }
    }
    if(check==1){
        printf("\nlinda");
    }
    printf("\n");
    int cantComparacion = 1;
    

    //tengo que recorrer la mitad del vector comparando los otros dos

    //miestras que el u otro no lo encuentre lo comparo
    int i=0,sizeVector = N/2;
    while(iguales && i<sizeVector){
        //miro esta pos + total/2
        if(A[i]!=A[i+sizeVector]){
            //si es igual pongo = 0 y se corta el de todos
            iguales=0;
        }
        i++;
    }

    //supongo que si es una variable compartida no la tengo que compartir ni reducir ni nada
}

int main(int argc, char *argv[])
{
    N = atoi(argv[1]);
    int check=1;


    //supongo que los dos vectores estan alocados de forma continua
    A = (double *)malloc(sizeof(double) * N);
    B = (double *)malloc(sizeof(double) * N);

    int i;
    double timetick;

    // Inicializar el generador de números aleatorios
    srand(time(NULL));
    for (i = 0; i < N/2; i++)
    {
        A[i]= (double) (rand() % 100);
        A[i+N/2] = A[i];
    }

    for (int i = 0; i < N; i++)
    {
        printf("%.0f ",A[i]);
    }
    printf("\n");
    
    timetick = dwalltime();
    //ejecuto el algoridmo
    funcion();

    printf("Tiempo en segundos usando los dos en filas %f\n", dwalltime() - timetick);

    for (int i = 0; i < N-1; i++)
    {
        printf("%.0f ",A[i]);
        if(A[i]>A[i+1]&& i+1 != N/2 ){
            check=0;
        }
    }

    if(iguales){
        printf("Son iguales\n");
    }else{
        printf("Son distinto\n");
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