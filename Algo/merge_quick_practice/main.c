#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void swap(char* x, char* y){
    char tmp = *x;
    *x = *y;
    *y = tmp;
    return;
}

void merge(char* arr, int left, int mid, int right){
    char* sorted = (char*)malloc((right - left + 1)*sizeof(char));

    int i = left, j = mid + 1, k = 0;

    while (i <= mid && j <= right) // i~mid mid+1 ~ right 병합
	{
		if (arr[i] <= arr[j]) sorted[k++] = arr[i++];
		else sorted[k++] = arr[j++];
	}

    // 잔챙이들 넣어주기
	if (i > mid) 
		while (j <= right) sorted[k++] = arr[j++];
	else
		while (i <= mid) sorted[k++] = arr[i++];

	for (i = left, k = 0; i <= right; i++, k++)
        arr[i] = sorted[k];

    free(sorted);
    return;
}

void mergesort(char* array, int left, int right){
    if(left < right){
        int mid = (left + right) / 2;
        mergesort(array, left, mid);
        mergesort(array, mid + 1, right);
        merge(array, left, mid, right); // sort 하면서 병합!!!
    }
}

int partition(char* array, int left, int right){
    char x = array[right];
    int i = left - 1;
    for(int j = left; j < right; j++){
        if(array[j] <= x){
            i = i + 1;
            swap(&array[i], &array[j]);
        }
    }
    swap(&array[i+1], &array[right]);
    return i + 1;
}

void quicksort(char* array, int left, int right){
    if(left < right){
        int pivot = partition(array, left, right);
        quicksort(array, left, pivot - 1);
        quicksort(array, pivot + 1, right);   
    }
    return;
}


int main(){
    char array[10] = "rqcftkdeyh";

    int lo = 0, hi = 10;
    
    printf("before sort : %s\n", array);

    mergesort(array, 0, 9);
    // quicksort(array, 0, 9);

    printf("after sort : %s\n", array);
    // cdefhkqrty

    return 0;
}