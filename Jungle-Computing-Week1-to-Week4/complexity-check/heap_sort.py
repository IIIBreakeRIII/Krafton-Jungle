import random

def heap_sort_gen_rnd_list(n):
    arr = [random.randint(0, 100) for _ in range(n)]
    heapSort(arr)

def heapify(arr, n, i):
    largest = i  # 루트를 최대값으로 가정
    l = 2 * i + 1  # 왼쪽 자식
    r = 2 * i + 2  # 오른쪽 자식

    # 왼쪽 자식이 루트보다 크다면
    if l < n and arr[l] > arr[largest]:
        largest = l

    # 오른쪽 자식이 현재 최대값보다 크다면
    if r < n and arr[r] > arr[largest]:
        largest = r

    # 최대값이 루트가 아니라면
    if largest != i:
        arr[i], arr[largest] = arr[largest], arr[i]  # 교환

        # 교환된 루트에 대해 다시 힙 구성
        heapify(arr, n, largest)

def heapSort(arr):
    n = len(arr)

    # 초기 최대 힙 구성
    for i in range(n // 2 - 1, -1, -1):
        heapify(arr, n, i)

    # 하나씩 원소를 꺼내어 다시 최대 힙 구성
    for i in range(n-1, 0, -1):
        arr[i], arr[0] = arr[0], arr[i]  # 루트와 마지막 요소 교환
        heapify(arr, i, 0)
