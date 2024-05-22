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
    int parte = N / T;
    int inicio = id * parte;
    int limite = inicio + parte;
    printf("Hilo id:%d\n", id);
    // Código que ejecutará cada hilo


    //tengo que ordenar mi parte
    int size, j, iteracion=0;
    int puntA=0, puntB=0;
    int suma;
    int puntAS=-1, puntBS=-1;    //en axi me guardo los que fueron mas grandes que B que estaban en A
    double temp;
    for(size = 2; size <= (limite-inicio); size*=2){ //limite-inicio tamanio total del vector
        for (int i = 0; i < N/size; i++)
        {
            puntAS=-1;
            puntBS=-1;
            puntA=inicio+i*size;    //punto inicio vector A
            puntB=puntA+size/2;    //punto inicio vector B
            while (puntA!=puntB)
            {
                if (puntAS==-1 && puntBS==-1){
                    //si axi = -1 es que no hay ningun elemento en mi vector axiliar
                    if (A[puntA]>A[puntB]){
                        //el que esta en A es mas grande que el de B
                        //por lo tanto tengo que mover al de A al vector axiliar(donde esta el valor de B)
                        temp = A[puntB];
                        A[puntB]=A[puntA];
                        //pongo el de B en el lugar donde estaba en A
                        A[puntA] = temp;
                        //actualizo los punteros
                        puntA++;
                        puntAS = puntB;
                        if(puntB+1<inicio+i*size+size){
                            puntB++;//ojo cuando sumo B porque puede ser que me pase del vector
                        }
                    }else{
                        //ahora tengo que el vector de A tiene el mas chiquito o igual
                        //no tengo que mover nada solo actualizar los vectores
                        puntA++;
                    }
                }else{
                    if (A[puntA]<=A[axi] && A[puntA]<=A[puntB]){
                        //A sea el chiquito
                        //actualizo los punteros
                        puntA++;
                        if (puntA==axi){
                            //si es igual a axi es que el vector axi se termino
                            axi=-1;
                        }
                    }else{
                        if (A[axi]<A[puntB] && A[axi]<A[puntB]){
                            //Axi sea el mas chiquito
                            temp = A[puntA];
                            A[puntA]=A[axi];
                            A[axi]=A[puntB];
                            A[puntB]=temp;
                            
                            //actualizo punteros
                            puntA++;
                            if (puntA==axi){
                                //si es igual a axi es que el vector axi se termino
                                axi=-1;
                            }
                            if(puntB+1<inicio+i*size+size){
                                puntB++;//ojo cuando sumo B porque puede ser que me pase del vector
                            }
                        }else{
                            //B sea el mas chiquito
                            //intercambio A con B
                            temp = A[puntA];
                            A[puntA] = A[puntB];
                            A[puntB] = temp;

                            //actualizo punteros
                            puntA++;
                            if (puntA==axi){
                                //si es igual a axi es que el vector axi se termino
                                axi=-1;
                            }
                            if(puntB+1<inicio+i*size+size){
                                puntB++;//ojo cuando sumo B porque puede ser que me pase del vector
                            }
                        }
                        
                    }
                    
                }
            }
        }
        printf("size:%d\n",size);
        for (int i = 0; i < N; i++)
        {
            if(!(i%size)){
                printf("\t");
            }
            printf("%.0f ",A[i]);
        }
        printf("\n");
    }
    //tengo que ver si quedo eleguido para seguir
        //hago el merge

    int hilosRestantes = T;

    sumas[id]=suma;

    while (id<hilosRestantes && hilosRestantes!=1)
    {
        printf("hilo: %d, espera: %d \n",id,hilosRestantes/2);
        pthread_barrier_wait(&barreras[id%(hilosRestantes/2)]);
        hilosRestantes = hilosRestantes/2;
        if(id<hilosRestantes){
            sumas[id] = sumas[id]+sumas[id+hilosRestantes];
        }
    }
    

    
    cantTotal += suma;
    printf("suma %d: %d \n",id,suma);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    //N = atoi(argv[1]);
    //T = atoi(argv[2]);
    N=16;
    T=1;
    pthread_t misThreads[T];
    int threads_ids[T];
    int check=1;

    A = (double *)malloc(sizeof(double) * N);

    int i,j;
    double timetick;

     for (i = 0; i < T/2; i++) {                //cant de hilos que se quedan frenados
        pthread_barrier_init(&barreras[i], NULL, 1);//2 // Inicializa cada barrera del arreglo
    }

    double numeros[] = {3, 15, 27, 8, 12, 23, 4, 30, 19, 6, 25, 9, 17, 2, 21, 14};

    //valor inicial a las matrices
    for (i = 0; i < N; i++)
    {
        //A[i]=N-i;
        A[i]=numeros[i];
    }
    
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

    for (int i = 0; i < N; i++)
    {
        printf("%.0f ",A[i]);
    }

    if (check)
    {
        printf("Multiplicacion de matrices resultado correcto\n");
    }
    else
    {
        printf("Multiplicacion de matrices resultado erroneo\n");
    }

    return 0;
}