import random

def gen_rnd_list(n):
    return bubble_sort([random.randint(1, 100) for _ in range(n)])

def bubble_sort(num_list):
    n = len(num_list)

    for i in range(n):
        for j in range(0, n - i - 1):
            if num_list[j] > num_list[j + 1]:
                num_list[j], num_list[j + 1] = num_list[j + 1], num_list[j]
    
    return num_list
