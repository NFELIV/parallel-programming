// №13 Сортировка Хоара с простым слиянием 
// ЛР#1 Последовательная реализация

#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <ctime>

using namespace std;

void CreateArray(int arr[], int lenght) //Генерация 
{
	srand((unsigned int)time(NULL));
	for (int i = 0; i < lenght; i++)
	{
		arr[i] = rand() % 10;
	}
}

void PrintArray(int* arr, int size) //Печать 
{
	if (size < 20)
	{
		for (int i = 0; i < size; i++)
		{
			cout << arr[i]<< " ";
		}
	}
	return;
}

void QuickSort(int* arr, int l, int r) //Сортировка
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

int main(int argc, char * argv[]) 
{
	int size = 10000;
	double time = 0;
	if (argc == 2)
	{
		size = atoi(argv[1]);
	}
	else if (argc != 2)
	{
		cout << "Error! Only the number of elements in the array is needed" << endl;
	}
	int* array = new int[size];
	CreateArray(array, size);
	cout << "The Generated array: ";
	PrintArray(array, size);
	cout << endl;
	time = clock();
	QuickSort(array, 0, size - 1);
	time = (clock() - time) / static_cast<double>(CLOCKS_PER_SEC);
	cout << "Array after QuickSort: ";
	PrintArray(array, size);
	cout << endl;
	cout << "Sequence version time: " << time << endl;
	delete[] array;
	system("pause");
	return 0;
}
