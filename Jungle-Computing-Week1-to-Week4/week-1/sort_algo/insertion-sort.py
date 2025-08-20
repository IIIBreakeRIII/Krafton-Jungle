# Insertion Sort

def insertion_sort(num_list):

    for i in range(1, len(num_list)):
        key = num_list[i]
        j = i - 1

        while j >= 0 and num_list[j] > key:
            num_list[j + 1] = num_list[j]
            j -= 1

        num_list[j + 1] = key

    return num_list

print(insertion_sort([1, 6, 7, 2, 5]))
