Глотов Н.С ЛР№2 Cортировка Хоара с простым слиянием. OMP
// №13 Сортировка Хоара с простым слиянием 
// ЛР#2 Реализация OMP

#include "stdafx.h"
#include <omp.h>
#include <iostream>
#include <time.h>
#include <queue>

using namespace std;

struct part
{
	int* array;
	unsigned size;
};

int* merge(int* a, int* b, int n, int m)
{
	int* c;
	int size = n + m;
	c = new int[size];
	int i = 0, j = 0, k = 0;
	while (i < n && j < m)
	{
		if (a[i] <= b[j])
		{
			c[k] = a[i];
			i++;
		}
		else
		{
			c[k] = b[j];
			j++;
		}
		k++;
	}
	while (i < n)
	{
		c[k] = a[i];
		k++;
		i++;
	}
	while (j < m)
	{
		c[k] = b[j];
		k++;
		j++;
	}
	return c;
}

void swap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

int partition(int arr[], int low, int high)
{
	int pivot = arr[high];
	int i = (low - 1);
	for (int j = low; j <= high - 1; j++)
	{
		if (arr[j] < pivot)
		{
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}

void quickSort(int arr[], int low, int high)
{
	if (low < high)
	{
		int pi = partition(arr, low, high);
		quickSort(arr, low, pi - 1);
		quickSort(arr, pi + 1, high);
	}
}


int* parallel_merge(int numtasks, part* parts)
{
	if (numtasks == 2)
	{
		return merge(parts[0].array, parts[1].array, parts[0].size, parts[1].size);
	}
	else if (numtasks == 3)
	{
		return merge(parts[2].array, merge(parts[0].array, parts[1].array, parts[0].size, parts[1].size),
			parts[2].size, parts[0].size + parts[1].size);
	}
	else if (numtasks < 2) return nullptr;

	int current_parts = numtasks / 2 + numtasks % 2;
	part* sorted_parts = new part[current_parts];
	int _numtasks = numtasks;
	int _current_parts = current_parts;
	int part_size = parts[0].size;
	for (size_t i = 0; i < numtasks / 2; i++)
	{
		sorted_parts[i].array = new int[part_size * 2];
		sorted_parts[i].size = part_size * 2;
	}
	if (numtasks % 2)
	{
		sorted_parts[current_parts - 1].size = parts[numtasks - 1].size;
		sorted_parts[current_parts - 1].array = new int[sorted_parts[current_parts - 1].size];
		for (size_t i = 0; i < sorted_parts[current_parts - 1].size; i++)
		{
			sorted_parts[current_parts - 1].array[i] = parts[numtasks - 1].array[i];
		}
		_numtasks--;
		_current_parts--;
	}

	omp_set_num_threads(_current_parts);
#pragma omp parallel shared(_current_parts)
	{
		int taskid = omp_get_thread_num();
		int part_id = ((taskid + 1) * 2) - 1;
		sorted_parts[taskid].array = merge(parts[part_id].array, parts[part_id - 1].array, parts[part_id].size, parts[part_id - 1].size);
	}
	for (size_t i = 0; i < numtasks; i++)
	{
		delete[] parts[i].array;
	}
	return parallel_merge(current_parts, sorted_parts);
}

int main(int argc, char** argv)
{
	int* start_array = nullptr;
	int* sorted_start_array = nullptr;
	int* array = nullptr;
	int* remainder_array = nullptr;
	int* result_array = nullptr;
	clock_t start;
	clock_t end;
	clock_t start_single;
	clock_t end_single;
	int numtasks;
	int size;
	int remainder = 0;
	int** parts;
	if (argc == 3)
	{
		numtasks = atoi(argv[1]);
		size = atoi(argv[2]);
	}
	else
	{
		{
			cout << "Error! Only the number of elements in the array is needed" << endl; cout << "Error!" << endl << "Two arguments must be entered: [1] - size array, [2] - threads..." << endl;
			return 0;
		}
	}
	start_array = new int[size];
	sorted_start_array = new int[size];

	for (size_t i = 0; i < size; i++)
	{
		start_array[i] = rand() % 1000;
	}
	for (size_t i = 0; i < size; i++)
	{
		sorted_start_array[i] = start_array[i];
	}
	start_single = clock();
	quickSort(sorted_start_array, 0, size - 1);
	end_single = clock();
	remainder = size % numtasks;
	size = size - remainder;
	array = new int[size];
	if (remainder)
	{
		remainder_array = new int[remainder];
	}
	int i = 0;
	for (; i < size; i++)
	{
		array[i] = start_array[i];
	}
	for (int j = 0; j < remainder; j++, i++)
	{
		remainder_array[j] = start_array[i];
	}
	if (remainder > 1)
	{
		quickSort(remainder_array, 0, remainder - 1);
	}
	delete[] start_array;
	int part_size = size / numtasks;
	parts = new int*[numtasks];
	for (size_t i = 0; i < numtasks; i++)
	{
		parts[i] = new int[part_size];
		int shift = i * part_size;
		for (int j = 0; j < part_size; j++)
		{
			parts[i][j] = array[j + shift];
		}
	}
	omp_set_num_threads(numtasks);
#pragma omp parallel shared(numtasks)
	{
		int taskid = omp_get_thread_num();
#pragma omp single
		{
			start = clock();
		}
		quickSort(parts[taskid], 0, part_size - 1);
	}
	if (numtasks == 2)
	{
		if (remainder)
		{
			result_array = merge(remainder_array, merge(parts[0], parts[1], part_size, part_size), remainder, size);
			size += remainder;
		}
		else result_array = merge(parts[0], parts[1], part_size, part_size);
	}
	else if (numtasks == 3)
	{
		if (remainder)
		{
			result_array = merge(remainder_array, merge(parts[2], merge(parts[0], parts[1], part_size, part_size),
				part_size, part_size + part_size), remainder, size);
			size += remainder;
		}
		else result_array = merge(parts[2], merge(parts[0], parts[1], part_size, part_size), part_size, part_size + part_size);
	}
	else if (numtasks < 2)
	{
		result_array = parts[0];
	}
	else
	{
		int current_parts = numtasks / 2 + numtasks % 2;
		part* sorted_parts = new part[current_parts];
		int _numtasks = numtasks;
		int _current_parts = current_parts;
		for (size_t i = 0; i < numtasks / 2; i++)
		{
			sorted_parts[i].array = new int[part_size * 2];
			sorted_parts[i].size = part_size * 2;
		}
		if (numtasks % 2)
		{
			sorted_parts[current_parts - 1].size = part_size;
			sorted_parts[current_parts - 1].array = new int[part_size];
			for (size_t i = 0; i < part_size; i++)
			{
				sorted_parts[current_parts - 1].array[i] = parts[numtasks - 1][i];
			}

			_numtasks--;
			_current_parts--;
		}

		omp_set_num_threads(_current_parts);
#pragma omp parallel shared(_current_parts)
		{
			int taskid = omp_get_thread_num();
			int part_id = ((taskid + 1) * 2) - 1;

			sorted_parts[taskid].array = merge(parts[part_id], parts[part_id - 1], part_size, part_size);

		}
		for (size_t i = 0; i < numtasks; i++)
		{
			delete[] parts[i];
		}
		delete[] parts;
		if (remainder)
		{
			result_array = merge(remainder_array, parallel_merge(current_parts, sorted_parts), remainder, size);
			size += remainder;
		}
		else result_array = parallel_merge(current_parts, sorted_parts);
	}
	end = clock();
	bool same = true;
	for (size_t i = 0; i < size; i++)
	{
		if (sorted_start_array[i] != result_array[i])
		{
			same = false;
			break;
		}

	}
	cout << "Arrays check: ";
	if (same) cout << "PASS!" << endl;
	else cout << "ERROR!" << endl;
	double seconds_single = (double)(end_single - start_single) / CLOCKS_PER_SEC;
	cout << "The time of single thread quick sort " << seconds_single << " seconds." << endl;
	double seconds = (double)(end - start) / CLOCKS_PER_SEC;
	cout << "The time of " << numtasks << "x threads quick sort " << seconds << " seconds." << endl;
	cout << endl << "Result: " << seconds_single / seconds << "x acceleration" << endl;
	delete[] sorted_start_array;
	delete[] array;
	delete[] remainder_array;
	delete[] result_array;
	return 0;
}
