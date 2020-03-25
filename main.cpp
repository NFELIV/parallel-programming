#include "mpi.h"
#include <iostream>
#include "locale"

#define MaxMatrixSizeForPrintToScreen 10

int MaxSearch(const int* vector, const int n) // Поиск максимума в векторе, n 
{
	int max;
	if (n == 0)
	{
		return INT_MAX;
	}
	max = vector[0];
	for (int i = 0; i < n; i++)
	{
		if (max < vector[i])
		{
			max = vector[i];
		}
	}
	return max;
}

void OutputMatrix(const int* matrix, const int n, const int m) // Вывод матрицы на экран
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			printf("%7d ", matrix[i * m + j]);
		}
		printf("\n");
	}
	printf("\n");
}

void FillMatrix(int* matrix, const int n, const int m) // Заполнение матрицы
{
	srand((int)time(NULL));
	for (int i = 0; i < n * m; i++)
	{
		matrix[i] = (int)rand();
	}
}

int main(int argc, char* argv[])
{
	int msN, msM;				// размеры матрицы
	int *matrix = 0;			// указатель на массив, описывающий матриц
	int max;				// максимальное значение в матрице
	int proc_num, proc_rank;	        // переменные для хранения ранга этого процесса и количества процессов
	int *send_counts;			// указатель на массив, описывающий, сколько элементов нужно отправить в каждый процесс
	int *displs;				// указатель на массив, описывающий смещения, с которых начинается каждый сегмент
	int *recieve_buffer;		        // указатель на буфер, в котором должны храниться полученные данные
	int chunk_size;				// количество элементов в полном куске данных
	int rem;				// количество элементов, оставшиеся после разделения между процессами
	double start_time;			// время начала   
	double end_time;			// время окончания 
	int GlobMax = 0;			// переменная для сбора MAXимума
	const int root = 0;			// корневой процесс root
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_num);	// количество процессов 
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);	// ранг процесса
	if (argc == 3)
	{
		msN = atoi(argv[1]);
		msM = atoi(argv[2]);
		if (proc_rank == root) 
		{
			matrix = new int[msN * msM];
			FillMatrix(matrix, msN, msM);
			if ((msN < MaxMatrixSizeForPrintToScreen) && (msM < MaxMatrixSizeForPrintToScreen))
			{
				printf("\nThe Generated Matrix %dx%d:\n\n", msN, msM);
				OutputMatrix(matrix, msN, msM);
			}
		}
		start_time = MPI_Wtime();
		send_counts = new int[proc_num];
		displs = new int[proc_num];
		chunk_size = (int)(msN * msM) / proc_num;
		rem = (msN * msM) % proc_num;
		if (chunk_size == 0)
		{
			chunk_size = 1;
			rem = 0;
		}
		send_counts[0] = chunk_size + rem;
		displs[0] = 0;
		for (int i = 1; i < proc_num; i++)
		{
			send_counts[i] = chunk_size;
			displs[i] = i * chunk_size + rem;
		}
		recieve_buffer = new int[send_counts[proc_rank]];
		MPI_Scatterv(matrix, send_counts, displs, MPI_INT, recieve_buffer, send_counts[proc_rank], MPI_INT, root, MPI_COMM_WORLD);
		max = MaxSearch(recieve_buffer, send_counts[proc_rank]);
		MPI_Reduce(&max, &GlobMax, 1, MPI_INT, MPI_MAX, root, MPI_COMM_WORLD);
		if (proc_rank == root)
		{
			end_time = MPI_Wtime();
			printf("Max elem. = %7d\n\n", GlobMax);
			printf("Number of elem. in a full piece of data  = %3d\n", chunk_size);
			printf("Number of elem. remaining after separation between processes = %3d\n", rem);
			printf("Time spent: %lf\n", end_time - start_time);
			delete matrix;
		}
		delete[] send_counts;
		delete[] displs;
		delete[] recieve_buffer;
	}
	else
	{
	// Если аргументов нет или их больше 3 
		if (proc_rank == root) // для root процесса
		{
			printf("Enter: %s n m \n", argv[0]);
			printf("n - rows\n");
			printf("m - columns\n");
		}
	}
	MPI_Finalize();
	return 0;
}
