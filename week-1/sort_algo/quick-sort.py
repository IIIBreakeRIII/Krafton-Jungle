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

        print(f"{left_list} + {[pivot]} + {right_list}")
    
    return quick_sort(left_list) + [pivot] + quick_sort(right_list)


my_list = [24, 26, 2, 16, 32, 31, 25]
print("Before Sorting : ")
print(my_list)
print()
print("During Sorting... : ")
print(quick_sort(my_list))
