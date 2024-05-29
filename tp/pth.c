#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

double *A, *B;
int N, T;
int iguales = 1;

pthread_barrier_t barreras[64];
pthread_barrier_t barreraGlobal;

// Para calcular tiempo
double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

void *merge_sort(void *arg)
{
    // Identificador del hilo y cálculo del tamaño de la sección que este hilo procesará
    int thread_id = *(int *)arg;
    int section_size = N / T;
    int start_index = thread_id * section_size;
    int end_index = start_index + section_size;
    int merge_size, left_start, right_start, left_end, right_end, merge_index;

    // Ordenamiento de la sección asignada usando Merge Sort
    for (merge_size = 2; merge_size <= section_size; merge_size *= 2)
    {
        for (int i = 0; i < section_size / merge_size; i++)
        {
            left_start = start_index + i * merge_size;             // Inicio de la mitad izquierda
            right_start = left_start + merge_size / 2;             // Inicio de la mitad derecha
            left_end = right_start;                                // Fin de la mitad izquierda
            right_end = start_index + i * merge_size + merge_size; // Fin de la mitad derecha
            merge_index = left_start;                              // Índice para fusionar en el arreglo B

            // Fusionar las dos mitades ordenadas
            while (left_start < left_end && right_start < right_end)
            {
                if (A[left_start] < A[right_start])
                {
                    B[merge_index] = A[left_start];
                    left_start++;
                }
                else
                {
                    B[merge_index] = A[right_start];
                    right_start++;
                }
                merge_index++;
            }

            // Copiar cualquier elemento restante de la mitad izquierda
            while (left_start < left_end)
            {
                B[merge_index] = A[left_start];
                left_start++;
                merge_index++;
            }

            // Copiar cualquier elemento restante de la mitad derecha
            while (right_start < right_end)
            {
                B[merge_index] = A[right_start];
                right_start++;
                merge_index++;
            }
        }

        // Copiar los elementos ordenados de B a A para la siguiente iteración
        for (int i = start_index; i < end_index; i++)
        {
            A[i] = B[i];
        }
    }

    int remaining_threads = T;
    int comparison_factor = 1;
    int elDivisor = 2;
    int meVoy = 1;

    // Sincronización y combinación de secciones ordenadas entre hilos
    while (remaining_threads > 1 && meVoy)
    {
        comparison_factor *= 2;

        if (thread_id % comparison_factor == 0)//(thread_id % elDivisor == 0)
        {
            //quien se queda
            int esperoEn = thread_id/elDivisor;
            //printf("id: %d, espera en: %d\n",thread_id,esperoEn);
            //pthread_barrier_wait(&barreras[esperoEn]);
            
            int partner = thread_id + comparison_factor / 2;
            remaining_threads /= 2;
            if (partner < T)
            {
                //espero como en mpi en mi id "que el mensaje llegue a mi"
                printf("id: %d, espera en: %d\n",thread_id,partner);
                pthread_barrier_wait(&barreras[partner]);
                if(remaining_threads>1){
                    //aca lo ordeno
                    section_size = (int) (N / remaining_threads);
                    start_index = (thread_id / elDivisor) * section_size;
                    end_index = start_index + section_size;

                    printf("id: %d s:%d e:%d r: %d\n",thread_id, start_index, end_index,remaining_threads);

                    left_start = start_index;
                    right_start = left_start + section_size / 2;
                    left_end = right_start;
                    right_end = end_index;
                    merge_index = left_start;

                    // Fusionar las dos mitades ordenadas de las secciones combinadas
                    while (left_start < left_end && right_start < right_end)
                    {
                        if (A[left_start] < A[right_start])
                        {
                            B[merge_index] = A[left_start];
                            left_start++;
                        }
                        else
                        {
                            B[merge_index] = A[right_start];
                            right_start++;
                        }
                        merge_index++;
                    }

                    // Copiar cualquier elemento restante de la mitad izquierda
                    while (left_start < left_end)
                    {
                        B[merge_index] = A[left_start];
                        left_start++;
                        merge_index++;
                    }

                    // Copiar cualquier elemento restante de la mitad derecha
                    while (right_start < right_end)
                    {
                        B[merge_index] = A[right_start];
                        right_start++;
                        merge_index++;
                    }

                    // Copiar los elementos fusionados de B a A
                    for (int i = start_index; i < end_index; i++)
                    {
                        A[i] = B[i];
                    }
                }
            }
            elDivisor = elDivisor*2;
        }else{
            //el que se va
            int esperoEn = thread_id/elDivisor;
            //int partner = thread_id - comparison_factor / 2;
            int partner = thread_id;
            printf("id: %d, espera ab en: %d r:%d\n",thread_id,partner,remaining_threads);
            pthread_barrier_wait(&barreras[partner]);
            meVoy = 0;
            break;
        }
    }
    printf("f: %d\n",thread_id);

    // Barrera global para sincronizar todos los hilos antes de la verificación
    pthread_barrier_wait(&barreraGlobal);

    // Verificación de igualdad en paralelo
    int half_size = N / 2;
    section_size = half_size / T;
    start_index = thread_id * section_size;
    end_index = start_index + section_size;

    for (int i = start_index; i < end_index; i++)
    {
        // Comparar cada elemento de la primera mitad con su correspondiente en la segunda mitad
        if (A[i] != A[i + half_size])
        {
            iguales = 0;
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    pthread_t misThreads[T];
    int threads_ids[T];
    int check = 1;

    // supongo que los dos vectores estan alocados de forma continua
    A = (double *)malloc(sizeof(double) * N);
    B = (double *)malloc(sizeof(double) * N);

    int i, j;
    double timetick;

    for (i = 0; i < T ; i++)
    {                                                // cant de hilos que se quedan frenados
        pthread_barrier_init(&barreras[i], NULL, 2); // Inicializa cada barrera del arreglo
    }
    pthread_barrier_init(&barreraGlobal, NULL, T);

    // Inicializar el generador de números aleatorios
    srand(time(NULL));
    for (i = 0; i < N/2; i++)
    {
        A[i] = (double)(rand() % 100);
        A[i+N/2] = A[i];
    }

    
    for (int i = 0; i < N; i++)
    {
        printf("%.0f ",A[i]);
    }
    printf("\n");

    timetick = dwalltime();

    // llamo a los hilos
    for (int id = 0; id < T; id++)
    {
        threads_ids[id] = id;
        pthread_create(&misThreads[id], NULL, &merge_sort, (void *)&threads_ids[id]);
    }
    for (int id = 0; id < T; id++)
    {
        pthread_join(misThreads[id], NULL);
    }

    printf("Tiempo en segundos usando los dos en filas %f\n", dwalltime() - timetick);

    for (i = 0; i < T / 2; i++)
    {
        pthread_barrier_destroy(&barreras[i]); // Destruye cada barrera del arreglo
    }

    for (int i = 0; i < (N/2)-1; i++)
    {
        printf("%.0f ", A[i]);
        if (A[i] > A[i + 1] && i + 1 != N / 2) // N/2 = 4"
        {
            check = 0;
        }
    }

    for (int i = N/2; i < N-1; i++)
    {
        printf("%.0f ", A[i]);
        if (A[i] > A[i + 1] && i + 1 != N / 2) // N/2 = 4"
        {
            check = 0;
        }
    }

    printf("\n");

    if (iguales)
    {
        printf("Son iguales\n");
    }
    else
    {
        printf("Son distinto\n");
    }

    if (check)
    {
        printf("estan ordenados\n");
    }
    else
    {
        printf("estan desordenados\n");
    }

    free(A);
    free(B);

    return 0;
}