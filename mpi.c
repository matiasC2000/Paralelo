#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

// Función para calcular el tiempo
double dwalltime()
{
    double sec;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

// Función para realizar el merge sort en paralelo
void merge_sort(double *local_A, double *local_B, int local_N)
{
    int size, left_start, right_start, left_end, right_end, merge_index;

    // Ordenamiento usando Merge Sort
    for (size = 2; size <= local_N; size *= 2)
    {
        for (int i = 0; i < local_N / size; i++)
        {
            left_start = i * size;
            right_start = left_start + size / 2;
            left_end = right_start;
            right_end = i * size + size;
            merge_index = left_start;

            // Fusionar las mitades izquierda y derecha
            while (left_start < left_end && right_start < right_end)
            {
                if (local_A[left_start] < local_A[right_start])
                {
                    local_B[merge_index] = local_A[left_start];
                    left_start++;
                }
                else
                {
                    local_B[merge_index] = local_A[right_start];
                    right_start++;
                }
                merge_index++;
            }

            // Copiar los elementos restantes de la mitad izquierda
            while (left_start < left_end)
            {
                local_B[merge_index] = local_A[left_start];
                left_start++;
                merge_index++;
            }

            // Copiar los elementos restantes de la mitad derecha
            while (right_start < right_end)
            {
                local_B[merge_index] = local_A[right_start];
                right_start++;
                merge_index++;
            }
        }

        // Copiar los elementos ordenados de B a A
        for (int i = 0; i < local_N; i++)
        {
            local_A[i] = local_B[i];
        }
    }
}

// Función para fusionar dos mitades ordenadas
void merge(double *A, double *B, int start_index, int mid_index, int end_index)
{
    int left_start = start_index;
    int right_start = mid_index;
    int merge_index = start_index;
    //printf("Primer elemento de A izquierdo: %0.f\n", A[left_start]);
    //printf("Primer elemento de A derecho: %0.f\n", A[right_start]);

    while (left_start < mid_index && right_start < end_index)
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
    while (left_start < mid_index)
    {
        B[merge_index] = A[left_start];
        left_start++;
        merge_index++;
    }

    // Copiar cualquier elemento restante de la mitad derecha
    while (right_start < end_index)
    {
        B[merge_index] = A[right_start];
        right_start++;
        merge_index++;
    }

    // Copiar los elementos fusionados de B a A
    for (int i = start_index; i < end_index; i++)
    {
        A[i] = B[i];
        //printf("A[i]: %0.f\n", A[i]);
    }
}


void proceso0(){

}

void procesoOtros(){

}



int main(int argc, char *argv[])
{
    // Definición de las variables globales
    double *A, *B;
    int N;
    int rank, size, local_N;
    double start_time, end_time;
    int local_iguales = 1;
    int global_iguales;

    // Inicializar MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtener el rango del proceso actual
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtener el número total de procesos

    // Verificar si se ha proporcionado el tamaño del arreglo
    if (argc < 2)
    {
        if (rank == 0)
        {
            printf("Uso: %s <N>\n", argv[0]);
        }
        MPI_Finalize(); // Finalizar MPI
        return 1;
    }

    N = atoi(argv[1]);  // Convertir el argumento N a entero
    local_N = N / size; // Calcular el tamaño de la porción local

    

    // Reservar memoria para las porciones locales de los arreglos
    double *local_A = (double *)malloc(sizeof(double) * N);
    double *local_B = (double *)malloc(sizeof(double) * N);

    if (rank == 0)
    {
        // Reservar memoria para los arreglos
        A = (double *)malloc(sizeof(double) * N);
        // Inicializar el generador de números aleatorios y el arreglo A en el proceso principal
        srand(time(NULL));
        for (int i = 0; i < N / 2; i++)
        {
            A[i] = (double)(rand() % 100); // Llenar la primera mitad del arreglo A con valores aleatorios
            A[i + N / 2] = A[i];           // Copiar la primera mitad del arreglo A a la segunda mitad
        }

        // Imprimir el arreglo original
        /*for (int i = 0; i < N; i++)
        {
            printf("%.0f ", A[i]);
        }
        printf("\n");
        */

        start_time = dwalltime(); // Iniciar el cronómetro
    }

    // Dividir el trabajo entre los procesos
    MPI_Scatter(A, local_N, MPI_DOUBLE, local_A, local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Ordenar la sección local del arreglo
    merge_sort(local_A, local_B, local_N);

    int remaining_processes = size;
    int comparison_factor = 1;

    while (remaining_processes > 1)
    {
        comparison_factor *= 2;

        if (rank % comparison_factor == 0)
        {
            int partner = rank + comparison_factor / 2;
            //printf("IF rank: %d, partner: %d\n", rank, partner);

            if (partner < size)
            {
                int section_size = local_N * comparison_factor; // 2 * 2 = 4 8
                int start_index = 0;                            // 0 * 2 = 0 0
                int mid_index = start_index + section_size / 2; // 0 + 2 = 2 4
                int end_index = start_index + section_size;     // 0 + 4 = 4 8

                // Recibir los datos del proceso compañero
                MPI_Recv(local_A + mid_index, section_size / 2, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                /*
                printf("rank: %d recibió: ", rank);
                for (int i = start_index; i < mid_index; i++)
                {
                    printf("%.0f ", local_A[i]);
                }
                printf("\n");
                for (int i = mid_index; i < end_index; i++)
                {
                    printf("%.0f ", local_A[i]);
                }
                printf("\n");
                */

                if (remaining_processes > 2)
                {
                    merge(local_A, local_B, start_index, mid_index, end_index);
                }
            }
        }
        else
        {
            int partner = rank - comparison_factor / 2;
            //printf("ELSE rank: %d, partner: %d\n", rank, partner);
            //printf(" local_N * comparison_factor: %d\n", local_N * comparison_factor);
            // Enviar mis datos al proceso compañero
            MPI_Send(local_A, (local_N * comparison_factor) / 2, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD);
            break;
        }

        remaining_processes /= 2;
    }

    MPI_Scatter(local_A, local_N / 2, MPI_DOUBLE, local_A, local_N / 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(local_A + N / 2, local_N / 2, MPI_DOUBLE, local_A + N / 2, local_N / 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Verificar si las dos mitades del arreglo son iguales
    for (int i = 0; i < local_N / 2; i++)
    {
        if (local_A[i] != local_A[i + N / 2])
        {
            local_iguales = 0;
            break;
        }
    }

    MPI_Reduce(&local_iguales, &global_iguales, 1, MPI_INT, MPI_LAND, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        end_time = dwalltime(); // Detener el cronómetro
        printf("Tiempo en segundos usando MPI: %f\n", end_time - start_time);
    }

    // Imprimir el resultado de la verificación
    if (rank == 0)
    {
        if (global_iguales)
        {
            printf("Son iguales\n");
        }
        else
        {
            printf("Son distintos\n");
        }

        /*
        // Imprimir el arreglo ordenado
        for (int i = 0; i < N; i++)
        {
            printf("%.0f ", local_A[i]);
        }
        printf("\n");*/
    }

    if(rank == 0){
        // Liberar la memoria
        free(A);
    }
    free(local_A);
    free(local_B);

    // Finalizar MPI
    MPI_Finalize();
    return 0;
}
