// №13 Сортировка Хоара с простым слиянием 
// ЛР#3 Реализация TBB

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <tbb/tbb.h>
#include <math.h>
#include <ctime>

using namespace std;

void CreateArray(double arr[], int lenght) //Генерация 
{
	srand((unsigned int)time(NULL));
	for (int i = 0; i < lenght; i++)
	{
		arr[i] = rand() % 10;
	}
}

void PrintArray(double* arr, int size) //Печать 
{
	if (size < 20)
	{
		for (int i = 0; i < size; i++)
		{
			cout << arr[i] << " ";
		}
	}
	return;
}

void QuickSort(double* arr, int l, int r) //Быстрая сортировка для последовательной реализации
{
	int i = l, j = r;
	double m = arr[(r + l) / 2];
	double temp = 0;
	while (i <= j)
	{
		while (arr[i] < m)
		{
			i++;
		}
		while (arr[j] > m)
		{
			j--;
		}
		if (i <= j)
		{
			if (i < j)
			{
				temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
			}
			i++;
			j--;
		}
	}
	if (j > l)
	{
		QuickSort(arr, l, j);
	}
	if (r > i)
	{
		QuickSort(arr, i, r);
	}
}


void Merge(double* array, int sizeOfPart, int it1, int it2) //Слияние для параллельной реализации
{
	int i = 0, j = 0, k = 0;
	int size = sizeOfPart + sizeOfPart;
	double* temp;
	temp = new double[size];
	while (i < sizeOfPart && j < sizeOfPart)
	{
		if (array[i + it1] <= array[j + it2])
		{
			temp[k] = array[i + it1];
			i++;
		}
		else
		{
			temp[k] = array[j + it2];
			j++;
		}
		k++;
	}
	if (i < sizeOfPart)
	{
		for (int p = i; p < sizeOfPart; p++)
		{
			temp[k] = array[p + it1];
			k++;
		}
	}
	else
	{
		for (int p = j; p < sizeOfPart; p++)
		{
			temp[k] = array[p + it2];
			k++;
		}
	}
	for (i = it1; i < it2 + sizeOfPart; i++)
		array[i] = temp[i - it1];
}


void ParallelQuickSort(double* arr, int length, int n_threads)
{
	tbb::tick_count tInit1_1 = tbb::tick_count::now();
	tbb::task_scheduler_init init(n_threads);
	tbb::tick_count tInit2_1 = tbb::tick_count::now();
	int grain_size = 0;
	grain_size = length / (n_threads);
	cout << "Threads: " << n_threads << endl;
	cout << "Length of array: " << length << endl;
	cout << "Size of one part: " << grain_size << endl;
	tbb::parallel_for(tbb::blocked_range<int>(0, n_threads), [=, &arr](const tbb::blocked_range<int> &r)
	{
		for (int f = r.begin(); f != r.end(); f++)
		{
			QuickSort(arr, f*grain_size, grain_size);
		}

	});
	init.terminate();
	int k = 0;
	int i = (n_threads / 2);
	int size = 0;
	for (i = (n_threads / 2); i > 0; i = i / 2)
	{
		size = grain_size*static_cast<int>(pow(2.0, k));
		init.initialize(i);
		tbb::parallel_for(tbb::blocked_range<int>(0, i), [=, &arr, &size](const tbb::blocked_range<int> &r)
		{
			for (int f = r.begin(); f != r.end(); f++)
			{
				int start1 = f*grain_size
					*static_cast<int>(pow(2.0, k + 1));
				int start2 = start1 +
					grain_size*static_cast<int>(pow(2.0, k));
				Merge(arr, size, start1, start2);
				Merge(arr, size, start1, start2);
			}
		});
		k++;
		init.terminate();
	}
}

int main(int argc, char** argv)
{
	int size = 10000;
	int threads = 4;
	double* sequence_sorted_array = new double[size]; // Массив для последовательной версии
	double* parallel_sorted_array = new double[size]; // Массив для параллельной версии
	int notCorrect = 0; //Счетчик корректности
	if (argc == 3)
	{
		size = atoi(argv[1]);
		threads = atoi(argv[2]);
	}
	else
	{
		cout << "Error!" << endl << "Two arguments must be entered: [1] - size array, [2] - threads..." << endl;
		return 0;
	}
	double* array = new double[size]; // Исходный массив
	CreateArray(array, size);
	cout << "The Generated array: ";
	PrintArray(array, size);
	cout << endl;
	for (int i = 0; i < size; i++)
	{
		sequence_sorted_array[i] = array[i];
		parallel_sorted_array[i] = array[i];
	}
	//Параллельная реализация
	tbb::tick_count start_parallel_time = tbb::tick_count::now();
	ParallelQuickSort(parallel_sorted_array, size, threads);
	tbb::tick_count finish_parallel_time = tbb::tick_count::now();
	//
	//Последовательная реализация
	tbb::tick_count start_sequence_time = tbb::tick_count::now();
	QuickSort(sequence_sorted_array, 0, size - 1);
	tbb::tick_count finish_sequence_time = tbb::tick_count::now();
	//
	double* equal = new double[size];
	for (int i = 0; i < size; i++) //проверка на правильность
	{
		equal[i] = sequence_sorted_array[i] - parallel_sorted_array[i];
		if (equal[i] != 0.0)
		{
			notCorrect++;
		}
	}
	if (notCorrect > 0)
	{
		cout << "Sorted arrays are not equal!" << endl;
	}
	else
	{
		cout << "Parallel sorted array: ";
		PrintArray(parallel_sorted_array, size);
		cout << endl;
		cout << "Sequence sorted array: ";
		PrintArray(sequence_sorted_array, size);
		cout << endl;
		cout << "Sorted arrays are equal!" << endl;
		cout << "Parallel version time: " << (finish_parallel_time - start_parallel_time).seconds() << endl;
		cout << "Sequence version time: " << (finish_sequence_time - start_sequence_time).seconds() << endl;
	}
	delete[] array;
	delete[] sequence_sorted_array;
	delete[] parallel_sorted_array;
	system("pause");
	return 0;
}
