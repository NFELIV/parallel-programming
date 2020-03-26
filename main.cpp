#include "mpi.h"
#include <iostream>
#include <assert.h>
#include "locale"

float *create_rand_nums(int num_elements) // Создание массива случайных чисел. 
{
	float *rand_nums = new float[num_elements];
	for (int i = 0; i < num_elements; i++)
		rand_nums[i] = (rand() / (float)RAND_MAX);
	return rand_nums;
}

int MY_MPI_Bcast_binominal_tree(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) // Реализация bcast через алгоритм биномиального дерева
{
	int proc_num, proc_rank;	// переменные для хранения ранга этого процесса и количества процессов
	MPI_Status status;			// статус для MPI_Recv
	int relative_rank;			// относительный ранг
	int mask;					// маска
	int src, dst;
	MPI_Comm_size(MPI_COMM_WORLD, &proc_num);	
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);	

	if (proc_num == 1) return 0;
	if (root <= proc_rank)
		relative_rank = proc_rank - root;
	else
		relative_rank = proc_rank - root + proc_num;
	mask = 0x1; // присваиваем переменной бинарное значение 1 
	while (mask < proc_num)
	{
		if ((relative_rank & mask) != 0)
		{

			src = proc_rank - mask;
			if (src < 0) src = src + proc_num;
			MPI_Recv(buffer, count, datatype, src, 0, comm, &status);
			break;
		}
		mask <<= 1;
	}
	mask >>= 1;
	while (mask > 0)
	{
		if ((relative_rank + mask) < proc_num)
		{
			dst = proc_rank + mask;
			if (dst >= proc_num) dst = dst - proc_num;
			MPI_Send(buffer, count, datatype, dst, 0, comm);
		}
		mask >>= 1;
	}
	return 1;
}

int MY_MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) // Реализация MPI_Allreduce
{
	int root = 0;
	MPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm); // Сбор данных от всех к одному
	MY_MPI_Bcast_binominal_tree(recvbuf, count, datatype, root, comm); // Функция Bcast основанная на биноминальных деревьях
	return 1;
}


int main(int argc, char* argv[])
{
	int num_elements_per_proc;	// количество элементов на процесс
	int proc_num, proc_rank;	// переменные для хранения ранга этого процесса и количества процессов
	float *rand_nums;			// указатель на буфер, в котором должны храниться сгенерированные числа
	float local_sum;			// локальная сумма чисел	
	float global_sum;			// глобальная сумма чисел
	const int root = 0;			// корневой процесс root	
	double start_time;			// время начала   
	double end_time;			// время окончания 

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_num);	
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);	
	if (argc == 2)
	{
		start_time = MPI_Wtime();
		num_elements_per_proc = atoi(argv[1]);
		srand(time(NULL)*proc_rank);   
		rand_nums = new float[proc_num];
		rand_nums = create_rand_nums(num_elements_per_proc);
		local_sum = 0;
		for (int i = 0; i < num_elements_per_proc; i++)	local_sum += rand_nums[i];
		global_sum = 0;
		MY_MPI_Allreduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
		printf("Process %d, local_sum=%f, local_avg=%f, global_sum=%f, global_avg=%f\n", proc_rank, local_sum, local_sum / num_elements_per_proc, global_sum, global_sum / (proc_num * num_elements_per_proc));
		free(rand_nums);
	}
	else
	{
		// Если аргументов нет или их больше 2 
		if (proc_rank == root) 
		{
			printf("Enter: avg num_elements_per_proc\n");
		}
	}
	end_time = MPI_Wtime();
	if (proc_rank == root)
		printf("\nTime spent: %lf\n", end_time - start_time);
	MPI_Finalize();
	return 0;
}
