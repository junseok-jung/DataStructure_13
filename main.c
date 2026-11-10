#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 피벗 선택 방식 상수 정의 */
#define PIVOT_FIRST   1
#define PIVOT_RANDOM  2
#define PIVOT_MEDIAN3 3

/* 64비트 환경 기준 함수 호출 1회당 스택 프레임 크기(Byte) 계산 기준
 * - Merge Sort: 포인터 2개(16B) + int 매개변수/지역변수 4개(16B) + 반환주소/SFP(16B) = 48 Bytes
 * - Quick Sort: 포인터 1개(8B) + int 매개변수/지역변수 5개(20B) + 반환주소/SFP(16B) = 44 Bytes (정렬 패딩 포함 48 Bytes)
 */
#define MERGE_FRAME_SIZE (sizeof(int*) * 2 + sizeof(int) * 4 + 16)
#define QUICK_FRAME_SIZE (sizeof(int*) * 1 + sizeof(int) * 5 + 16)

 /* 성능 측정용 전역 변수 */
long long g_comparisons = 0;
int g_max_depth = 0;

/* 두 정수의 값을 교환하는 보조 함수 */
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/* 오름차순 정렬 여부를 검증하는 함수 */
int is_sorted(const int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0; /* 정렬 실패 */
        }
    }
    return 1; /* 정렬 성공 */
}

/* =========================================================
 * [1] 병합 정렬 (Merge Sort)
 * ========================================================= */
void merge(int* arr, int* temp, int left, int mid, int right) {
    int i = left;      /* 왼쪽 부분 배열 시작 인덱스 */
    int j = mid + 1;   /* 오른쪽 부분 배열 시작 인덱스 */
    int k = left;      /* 임시 배열 시작 인덱스 */

    /* 두 부분 배열의 원소를 비교하여 작은 값부터 temp에 저장 */
    while (i <= mid && j <= right) {
        g_comparisons++; /* 데이터 값 비교 횟수 증가 */
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        }
        else {
            temp[k++] = arr[j++];
        }
    }

    /* 왼쪽 부분 배열에 남은 원소 복사 */
    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    /* 오른쪽 부분 배열에 남은 원소 복사 */
    while (j <= right) {
        temp[k++] = arr[j++];
    }

    /* 정렬된 임시 배열의 내용을 원본 배열로 복사 */
    for (int idx = left; idx <= right; idx++) {
        arr[idx] = temp[idx];
    }
}

void merge_sort_rec(int* arr, int* temp, int left, int right, int depth) {
    /* 최대 재귀 깊이 갱신 */
    if (depth > g_max_depth) {
        g_max_depth = depth;
    }

    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort_rec(arr, temp, left, mid, depth + 1);
        merge_sort_rec(arr, temp, mid + 1, right, depth + 1);
        merge(arr, temp, left, mid, right);
    }
}

size_t run_merge_sort(int* arr, int n) {
    g_comparisons = 0;
    g_max_depth = 0;

    /* 병합을 위한 임시 배열 동적 할당 */
    int* temp = (int*)malloc(sizeof(int) * n);
    if (temp == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        exit(1);
    }

    merge_sort_rec(arr, temp, 0, n - 1, 1);

    /* 최대 추가 메모리 = 임시 배열 크기 + 최대 재귀 스택 크기 */
    size_t memory_used = (sizeof(int) * n) + ((size_t)g_max_depth * MERGE_FRAME_SIZE);

    free(temp);
    return memory_used;
}

/* =========================================================
 * [2] 퀵 정렬 (Quick Sort)
 * ========================================================= */
int partition(int* arr, int left, int right, int pivot_type) {
    /* 피벗 선택 방식에 따라 피벗을 결정한 뒤, 공통 분할을 위해 arr[left] 위치로 이동 */
    if (pivot_type == PIVOT_RANDOM) {
        int rand_idx = left + rand() % (right - left + 1);
        swap(&arr[left], &arr[rand_idx]);
    }
    else if (pivot_type == PIVOT_MEDIAN3) {
        int mid = left + (right - left) / 2;

        /* 세 원소(left, mid, right)의 값을 비교하여 중간값을 찾음 (값 비교 3회 포함) */
        g_comparisons++;
        if (arr[left] > arr[mid]) swap(&arr[left], &arr[mid]);

        g_comparisons++;
        if (arr[left] > arr[right]) swap(&arr[left], &arr[right]);

        g_comparisons++;
        if (arr[mid] > arr[right]) swap(&arr[mid], &arr[right]);

        /* 중간값인 arr[mid]를 피벗 위치인 arr[left]로 이동 */
        swap(&arr[left], &arr[mid]);
    }

    int pivot = arr[left];
    int low = left + 1;
    int high = right;

    while (low <= high) {
        /* 피벗보다 큰 값을 만날 때까지 low를 오른쪽으로 이동 */
        while (low <= right) {
            g_comparisons++; /* 데이터 값 비교 횟수 증가 */
            if (arr[low] <= pivot) {
                low++;
            }
            else {
                break;
            }
        }

        /* 피벗보다 작거나 같은 값을 만날 때까지 high를 왼쪽으로 이동 */
        while (high >= left + 1) {
            g_comparisons++; /* 데이터 값 비교 횟수 증가 */
            if (arr[high] > pivot) {
                high--;
            }
            else {
                break;
            }
        }

        /* 엇갈리지 않았다면 두 원소 교환 */
        if (low < high) {
            swap(&arr[low], &arr[high]);
        }
    }

    /* 피벗(arr[left])을 확정된 위치(arr[high])로 이동 */
    swap(&arr[left], &arr[high]);
    return high;
}

void quick_sort_rec(int* arr, int left, int right, int pivot_type, int depth) {
    /* 최대 재귀 깊이 갱신 */
    if (depth > g_max_depth) {
        g_max_depth = depth;
    }

    if (left < right) {
        int pivot_idx = partition(arr, left, right, pivot_type);
        quick_sort_rec(arr, left, pivot_idx - 1, pivot_type, depth + 1);
        quick_sort_rec(arr, pivot_idx + 1, right, pivot_type, depth + 1);
    }
}

size_t run_quick_sort(int* arr, int n, int pivot_type) {
    g_comparisons = 0;
    g_max_depth = 0;

    quick_sort_rec(arr, 0, n - 1, pivot_type, 1);

    /* 최대 추가 메모리 = 최대 재귀 스택 크기 */
    size_t memory_used = (size_t)g_max_depth * QUICK_FRAME_SIZE;
    return memory_used;
}

/* =========================================================
 * 메인 실험 루프
 * ========================================================= */
int main(void) {
    int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    /* 난수 시드 초기화 */
    srand((unsigned int)time(NULL));

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];

        /* 원본 배열 및 복사본 배열 할당 */
        int* orig_arr = (int*)malloc(sizeof(int) * n);
        int* work_arr = (int*)malloc(sizeof(int) * n);
        if (!orig_arr || !work_arr) {
            fprintf(stderr, "메모리 할당 실패\n");
            return 1;
        }

        /* 무작위 정수 생성 (중복 허용) */
        for (int i = 0; i < n; i++) {
            orig_arr[i] = rand() % (n * 2);
        }

        printf("Data Size : %d\n\n", n);
        printf("%-15s %-14s %-11s %-13s\n", "Algorithm", "Comparisons", "Time(ms)", "Memory(Bytes)");
        printf("--------------------------------------------------------\n");

        /* 1. Merge Sort 실험 */
        memcpy(work_arr, orig_arr, sizeof(int) * n);
        clock_t start = clock();
        size_t mem_merge = run_merge_sort(work_arr, n);
        clock_t end = clock();
        double time_merge = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        if (!is_sorted(work_arr, n)) fprintf(stderr, "Merge Sort 정렬 오류!\n");
        printf("%-15s %-14lld %-11.3f %-13zu\n", "Merge Sort", g_comparisons, time_merge, mem_merge);

        /* 2. Quick Sort (First Element) 실험 */
        memcpy(work_arr, orig_arr, sizeof(int) * n);
        start = clock();
        size_t mem_q_first = run_quick_sort(work_arr, n, PIVOT_FIRST);
        end = clock();
        double time_q_first = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        if (!is_sorted(work_arr, n)) fprintf(stderr, "Quick(First) 정렬 오류!\n");
        printf("%-15s %-14lld %-11.3f %-13zu\n", "Quick (First)", g_comparisons, time_q_first, mem_q_first);

        /* 3. Quick Sort (Random Pivot) 실험 */
        memcpy(work_arr, orig_arr, sizeof(int) * n);
        start = clock();
        size_t mem_q_rand = run_quick_sort(work_arr, n, PIVOT_RANDOM);
        end = clock();
        double time_q_rand = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        if (!is_sorted(work_arr, n)) fprintf(stderr, "Quick(Random) 정렬 오류!\n");
        printf("%-15s %-14lld %-11.3f %-13zu\n", "Quick (Random)", g_comparisons, time_q_rand, mem_q_rand);

        /* 4. Quick Sort (Median-of-Three) 실험 */
        memcpy(work_arr, orig_arr, sizeof(int) * n);
        start = clock();
        size_t mem_q_med3 = run_quick_sort(work_arr, n, PIVOT_MEDIAN3);
        end = clock();
        double time_q_med3 = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        if (!is_sorted(work_arr, n)) fprintf(stderr, "Quick(Median3) 정렬 오류!\n");
        printf("%-15s %-14lld %-11.3f %-13zu\n\n", "Quick (Median3)", g_comparisons, time_q_med3, mem_q_med3);

        free(orig_arr);
        free(work_arr);
    }

    return 0;
}