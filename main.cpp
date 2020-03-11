#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "locale"


int new_log2(const int x)
{
	int y = x;
	int i = 0;
	while (y % 2 == 0)
	{
		i++;
		y = y / 2;
	}
	if ((int)(pow(2, (float)(i))) == x)
	{
		return i;
	}
	else
	{
		return 0;
	}
}


void new_sum(const void *a, void *a1, int n, MPI_Datatype t)
{
	int i;
	if (t == MPI_INT)
	{
		for (i = 0; i<n; i++)
		{
			((int *)a1)[i] = ((int *)a)[i] + ((int *)a1)[i];
		}
	}
	if (t == MPI_FLOAT)
	{
		for (i = 0; i<n; i++)
		{
			((float *)a1)[i] = ((float *)a)[i] + ((float *)a1)[i];
		}
	}
	if (t == MPI_DOUBLE)
	{
		for (i = 0; i<n; i++)
		{
			((double *)a1)[i] = ((double *)a)[i] + ((double *)a1)[i];
		}
	}
}


void new_min(const void *a, void *a1, int n, MPI_Datatype t)
{
	int i;
	if (t == MPI_INT)
	{
		for (i = 0; i<n; i++)
		{
			if (((int *)a1)[i]>((int *)a)[i])
			{
				((int *)a1)[i] = ((int *)a)[i];
			}
		}
	}
	if (t == MPI_FLOAT)
	{
		for (i = 0; i<n; i++)
		{
			if (((float *)a1)[i]>((float *)a)[i])
			{
				((float *)a1)[i] = ((float *)a)[i];
			}
		}
	}
	if (t == MPI_DOUBLE)
	{
		for (i = 0; i<n; i++)
		{
			if (((double *)a1)[i]>((double *)a)[i])
			{
				((double *)a1)[i] = ((double *)a)[i];
			}
		}
	}
}

int Tree_Allreduce(void* sendbuf, void* recvbuf, int count, int ProcRank, int ProcNum, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
	int h;
	h = (int)(log((float)ProcNum) / log(2.0));
	if (new_log2(ProcNum) == 0) h++;
	for (int i = 0; i < h; i++)
	{
		int j = 0;
		while (j < ProcNum)
		{
			int k = j + pow(2, (float)(i));

			if (k < ProcNum)
			{
				if (ProcRank == j)
				{
					MPI_Status status;
					MPI_Recv(recvbuf, count, datatype, k, 0, MPI_COMM_WORLD, &status);
					if (op == MPI_MIN) new_min(sendbuf, recvbuf, count, datatype);
					if (op == MPI_SUM) new_sum(recvbuf, sendbuf, count, datatype);
				}
				if (ProcRank == k)
				{
					MPI_Send(sendbuf, count, datatype, j, 0, MPI_COMM_WORLD);
				}
			}
			j = j + pow(2, (float)(i + 1));
		}
	}
	for (int i = 1 ; i < ProcNum; i++) 
	{
		for (int j = 0; j < i; j++) 
		{
			int tmp = 1;
			for (int k = 1; k < i; k++) 
			{
				tmp = tmp * 2;
			}
			if (ProcRank == j) 
			{
				if ((tmp + j < ProcNum) && (tmp + j != 0)) 
				{
					MPI_Send(sendbuf, count, datatype, tmp + j, 0, comm);
				}
			}
			if ((ProcRank == tmp + j) && (tmp + j < ProcNum) && (tmp + j != 0)) 
			{
				MPI_Status status;
				MPI_Recv(sendbuf, count, datatype, j, 0, comm, &status);
			}
		}
		MPI_Barrier(comm);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Russian");
	int ProcRank, ProcNum;
	double TStart, TFinish;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	TStart = MPI_Wtime();
	int *a = new int[100];
	int *b = new int[100];
	for (int i = 0; i < 100; i++)
	{
		a[i] = i;
	}
	for (int i = 0; i < 100; i++)
	{
		b[i] = 0;
	}
	Tree_Allreduce(a, b, 100, ProcRank, ProcNum, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	for (int k = 0; k < 100; k++)
		if (ProcRank == 1)
		{
			fprintf(stdout, "%d ", (int*)a[k]);
			fflush(stdout);
		}
	TFinish = MPI_Wtime();
	MPI_Finalize();
}