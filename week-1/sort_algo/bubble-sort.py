def bubble_sort(num_list):
    n = len(num_list)

    for i in range(n):
        for j in range(0, n - i - 1):
            if num_list[j] > num_list[j + 1]:
                num_list[j], num_list[j + 1] = num_list[j + 1], num_list[j]
    
    return num_list

print(bubble_sort([1, 4, 2, 4, 4, 3]))
