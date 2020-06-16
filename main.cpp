// №13 Сортировка Хоара с простым слиянием 
// ЛР#3 Реализация TBB

#include "stdafx.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include <time.h>
using namespace tbb;

#include <iostream>
#include <vector>

using namespace std;

struct part
{
	int* array;
	unsigned size;
};

void merge(int* c, int* a, int* b, int n, int m)
{
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


class Merger : public task
{
private:
	unsigned task_id;
	part* parts;
	part* sorted_parts;
public:
	Merger(unsigned _task_id, part* _parts, part* _sorted_parts)
		: task_id(_task_id), parts(_parts), sorted_parts(_sorted_parts) {}

	task* execute()
	{
		int part_id = ((task_id + 1) * 2) - 1;
		merge(sorted_parts[task_id].array, parts[part_id].array, parts[part_id - 1].array, parts[part_id].size, parts[part_id - 1].size);

		return NULL;
	}
};

class Sorter : public task
{
private:
	unsigned task_id;
	unsigned part_size;
	part* parts;
public:
	Sorter(unsigned _task_id, part* _parts, unsigned _part_size)
		: task_id(_task_id), parts(_parts), part_size(_part_size) {}

	task* execute()
	{
		quickSort(parts[task_id].array, 0, part_size - 1);
		return NULL;
	}
};

class ParallelMerger : public task
{
private:
	unsigned num_tasks;
	part* parts;
	part* sorted_parts;
	vector<Merger*> mergers;
public:
	ParallelMerger(unsigned _num_tasks, part* _parts, part* _sorted_parts)
		: num_tasks(_num_tasks), parts(_parts), sorted_parts(_sorted_parts)
	{
		mergers.resize(num_tasks);
	}

	task* execute()
	{
		unsigned task_id = 0;
		for (auto& x : mergers)
		{
			x = new (allocate_child()) Merger(task_id, parts, sorted_parts);
			task_id++;
		}
		set_ref_count(num_tasks + 1);
		for (auto& x : mergers)
		{
			spawn(*x);
		}
		wait_for_all();
		return NULL;
	}
};

class ParallelSorter : public task
{
private:
	unsigned num_tasks;
	unsigned part_size;
	part* parts;
	vector<Sorter*> sorters;
public:
	ParallelSorter(unsigned _num_tasks, part* _parts, unsigned _part_size)
		: num_tasks(_num_tasks), parts(_parts), part_size(_part_size)
	{
		sorters.resize(num_tasks);
	}

	task* execute()
	{
		unsigned task_id = 0;
		for (auto& x : sorters)
		{
			x = new (allocate_child()) Sorter(task_id, parts, part_size);
			task_id++;
		}
		set_ref_count(num_tasks + 1);
		for (auto& x : sorters)
		{
			spawn(*x);
		}

		wait_for_all();

		return NULL;
	}
};

int* parallel_merge(int numtasks, part* parts)
{
	if (numtasks == 2)
	{
		int* result_array = new int[parts[0].size + parts[1].size];
		merge(result_array, parts[0].array, parts[1].array, parts[0].size, parts[1].size);
		return result_array;
	}
	else if (numtasks == 3)
	{
		int* tmp_array = new int[parts[0].size + parts[1].size];
		merge(tmp_array, parts[0].array, parts[1].array, parts[0].size, parts[1].size);
		int* result_array = new int[parts[0].size + parts[1].size + parts[2].size];
		merge(result_array, parts[2].array, tmp_array, parts[2].size, parts[0].size + parts[1].size);
		delete[] tmp_array;
		return result_array;
	}
	else if (numtasks < 2) return nullptr;
	int current_parts = numtasks / 2 + numtasks % 2;
	part* sorted_parts = new part[current_parts];
	int _numtasks = numtasks;
	int _current_parts = current_parts;
	int part_size = parts[0].size;
	for (size_t i = 0; i < numtasks / 2; i++)
	{
		int part_id = ((i + 1) * 2) - 1;
		sorted_parts[i].size = parts[part_id].size + parts[part_id - 1].size;
		sorted_parts[i].array = new int[sorted_parts[i].size];
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
	ParallelMerger& parallelmerger = *new (task::allocate_root()) ParallelMerger(_current_parts, parts, sorted_parts);
	task::spawn_root_and_wait(parallelmerger);
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
	part* parts;
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
	parts = new part[numtasks];
	for (size_t i = 0; i < numtasks; i++)
	{
		parts[i].size = part_size;
		parts[i].array = new int[part_size];
		int shift = i * part_size;
		for (int j = 0; j < part_size; j++)
		{
			parts[i].array[j] = array[j + shift];
		}
	}
	task_scheduler_init init(numtasks);
	start = clock();
	result_array = new int[size + remainder];
	ParallelSorter& parallelsorter = *new (task::allocate_root()) ParallelSorter(numtasks, parts, part_size);
	task::spawn_root_and_wait(parallelsorter);
	if (numtasks == 2)
	{
		if (remainder)
		{
			int* tmp_array = new int[part_size * 2];
			merge(tmp_array, parts[0].array, parts[1].array, part_size, part_size);
			merge(result_array, remainder_array, tmp_array, remainder, size);
			delete[] tmp_array;
			size += remainder;
		}
		else
		{
			int* result_array = new int[part_size * 2];
			merge(result_array, parts[0].array, parts[1].array, part_size, part_size);
		}
	}
	else if (numtasks == 3)
	{
		if (remainder)
		{
			int* tmp_array = new int[part_size * 2];
			int* tmp_array2 = new int[part_size * 3];
			merge(tmp_array, parts[0].array, parts[1].array, part_size, part_size);
			merge(tmp_array2, parts[2].array, tmp_array, part_size, part_size + part_size);
			merge(result_array, remainder_array, tmp_array2, remainder, size);
			delete[] tmp_array;
			delete[] tmp_array2;
			size += remainder;
		}
		else
		{
			int* tmp_array = new int[part_size * 2];
			merge(tmp_array, parts[0].array, parts[1].array, part_size, part_size);
			merge(result_array, parts[2].array, tmp_array, part_size, part_size + part_size);
			delete[] tmp_array;
		}
	}
	else if (numtasks < 2)
	{
		result_array = parts[0].array;
	}
	else
	{
		int current_parts = numtasks / 2 + numtasks % 2;
		part* sorted_parts = new part[current_parts];
		int _numtasks = numtasks;
		int _current_parts = current_parts;
		for (size_t i = 0; i < numtasks / 2; i++)
		{
			int part_id = ((i + 1) * 2) - 1;
			sorted_parts[i].size = parts[part_id].size + parts[part_id - 1].size;
			sorted_parts[i].array = new int[sorted_parts[i].size];
		}
		if (numtasks % 2)
		{
			sorted_parts[current_parts - 1].size = part_size;
			sorted_parts[current_parts - 1].array = new int[part_size];
			for (size_t i = 0; i < part_size; i++)
			{
				sorted_parts[current_parts - 1].array[i] = parts[numtasks - 1].array[i];
			}
			_numtasks--;
			_current_parts--;
		}

		ParallelMerger& parallelmerger = *new (task::allocate_root()) ParallelMerger(_current_parts, parts, sorted_parts);
		task::spawn_root_and_wait(parallelmerger);
		for (size_t i = 0; i < numtasks; i++)
		{
			delete[] parts[i].array;
		}
		delete[] parts;
		if (remainder)
		{
			merge(result_array, remainder_array, parallel_merge(current_parts, sorted_parts), remainder, size);
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
	cout << std::endl << "Result: " << seconds_single / seconds << "x acceleration" << endl;
	delete[] sorted_start_array;
	delete[] array;
	delete[] remainder_array;
	delete[] result_array;
	return 0;
}
