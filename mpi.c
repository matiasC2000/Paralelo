#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

// Definición de las variables globales
double *A, *B;
int N;
int iguales = 1;

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
    }
}

int main(int argc, char *argv[])
{
    int rank, size, local_N;
    double start_time, end_time;

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

    // Reservar memoria para los arreglos
    A = (double *)malloc(sizeof(double) * N);
    B = (double *)malloc(sizeof(double) * N);

    // Reservar memoria para las porciones locales de los arreglos
    double *local_A = (double *)malloc(sizeof(double) * N);
    double *local_B = (double *)malloc(sizeof(double) * N);

    if (rank == 0)
    {
        // Inicializar el generador de números aleatorios y el arreglo A en el proceso principal
        srand(time(NULL));
        for (int i = 0; i < N / 2; i++)
        {
            A[i] = (double)(rand() % 100); // Llenar la primera mitad del arreglo A con valores aleatorios
            A[i + N / 2] = A[i];           // Copiar la primera mitad del arreglo A a la segunda mitad
        }

        // Imprimir el arreglo original
        for (int i = 0; i < N; i++)
        {
            printf("%.0f ", A[i]);
        }
        printf("\n");

        start_time = dwalltime(); // Iniciar el cronómetro
    }

    // Dividir el trabajo entre los procesos
    MPI_Scatter(A, local_N, MPI_DOUBLE, local_A, local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Ordenar la sección local del arreglo
    merge_sort(local_A, local_B, local_N);

    int remaining_processes = size;
    int comparison_factor = 1;

    // printf("rank: %d, size: %d\n", rank, size);

    while (remaining_processes > 1)
    {
        comparison_factor *= 2;

        if (rank % comparison_factor == 0)
        {
            int partner = rank + comparison_factor / 2;
            printf("IF rank: %d, partner: %d\n", rank, partner);

            if (partner < size)
            {
                int section_size = local_N * comparison_factor; // 2 * 2 = 4
                int start_index = rank * local_N;               // 0 * 2 = 0
                int mid_index = start_index + section_size / 2; // 0 + 2 = 2
                int end_index = start_index + section_size;     // 0 + 4 = 4

                // Recibir los datos del proceso compañero
                MPI_Recv(local_A + section_size / 2, section_size / 2, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                printf("IF rank: %d\n", rank);
                for (int i = start_index; i < end_index; i++)
                {
                    printf("%.0f ", A[i]);
                }
                printf("\n");

                // Fusionar las dos mitades ordenadas de las secciones combinadas
                merge(local_A, local_A + section_size / 2, start_index, mid_index, end_index);
            }
        }
        else
        {
            int partner = rank - comparison_factor / 2;
            printf("ELSE rank: %d, partner: %d\n", rank, partner);

            // Enviar mis datos al proceso compañero
            MPI_Send(local_A, local_N, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD);
            // [1,2,0,0,0,0,0,0]
            break;
        }

        remaining_processes /= 2;
    }
    // Reunir los arreglos ordenados en el proceso principal
    // MPI_Gather(local_A, local_N, MPI_DOUBLE, A, local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        end_time = dwalltime(); // Detener el cronómetro
        printf("Tiempo en segundos usando MPI: %f\n", end_time - start_time);

        // Verificar si las dos mitades del arreglo son iguales
        for (int i = 0; i < N / 2; i++)
        {
            if (A[i] != A[i + N / 2])
            {
                iguales = 0;
                break;
            }
        }

        // Imprimir el resultado de la verificación
        if (iguales)
        {
            printf("Son iguales\n");
        }
        else
        {
            printf("Son distintos\n");
        }

        // Imprimir el arreglo ordenado
        for (int i = 0; i < N; i++)
        {
            printf("%.0f ", A[i]);
        }
        printf("\n");
    }

    // Liberar la memoria
    free(A);
    free(B);
    free(local_A);
    free(local_B);

    // Finalizar MPI
    MPI_Finalize();
    return 0;
}
