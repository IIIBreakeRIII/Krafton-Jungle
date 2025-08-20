import random

def gen_rnd_list(n):
    return selection_sort([random.randint(1, 100) for _ in range(n)])

def selection_sort(num_list):
    for i in range(len(num_list)):
        min_index = i

        for j in range(i + 1, len(num_list)):
            if num_list[j] < num_list[min_index]:
                min_index = j

        num_list[i], num_list[min_index] = num_list[min_index], num_list[i]
    
    return num_list
