import random

def quick_sort_gen_rnd_list(n):
    arr = [random.randint(0, 100) for _ in range(n)]
    quick_sort(arr, 0, n - 1)

def quick_sort(arr, left, right):
    while left < right:
        # 피벗: 중간값 선택
        mid = (left + right) // 2
        arr[left], arr[mid] = arr[mid], arr[left]
        pivot = arr[left]

        i = left + 1
        j = right

        # Hoare 파티셔닝
        while True:
            while i <= j and arr[i] <= pivot:
                i += 1
            while i <= j and arr[j] > pivot:
                j -= 1
            if i > j:
                break
            arr[i], arr[j] = arr[j], arr[i]

        arr[left], arr[j] = arr[j], arr[left]  # 피벗과 arr[j] 교환하여 원래 위치로 복귀

        # 재귀 대신 작은 쪽 먼저 호출 → 큰 쪽은 while 반복
        if j - left < right - j:
            quick_sort(arr, left, j - 1)
            left = j + 1
        else:
            quick_sort(arr, j + 1, right)
            right = j - 1
    return arr
