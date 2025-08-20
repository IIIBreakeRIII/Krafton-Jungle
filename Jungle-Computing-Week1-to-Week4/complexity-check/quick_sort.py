import random

def quick_sort_gen_rnd_list(n):
    arr = [random.randint(0, 100) for _ in range(n)]
    quick_sort(arr)

def quick_sort(arr):

    if len(arr) < 2:
        return arr
    
    # pivot을 0번째 인덱스로 설정할 경우
    pivot = arr[0]
    left_list = []
    right_list = []

    for value in arr[1:]:
        if value < pivot:
            left_list.append(value)
        else:
            right_list.append(value)

    return quick_sort(left_list) + [pivot] + quick_sort(right_list)
