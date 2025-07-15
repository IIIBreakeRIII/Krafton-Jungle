import random

def radix_sort_gen_rnd_list(n):
    arr = [random.randint(0, 100) for _ in range(n)]
    radix_sort(arr)

def counting_sort(arr, digit):
    n = len(arr)
  
    output = [0] * (n)
    count = [0] * (10)
    
    for i in range(0, n):
        index = int(arr[i] / digit) 
        count[(index) % 10] += 1
 
    for i in range(1, 10):
        count[i] += count[i - 1]  
    
    i = n - 1

    while i >= 0:
        index = int(arr[i] / digit)
        output[count[(index) % 10 ] - 1] = arr[i]
        count[(index) % 10 ] -= 1
        i -= 1

    for i in range(0, len(arr)): 
        arr[i] = output[i]
 
def radix_sort(arr):

    maxValue = max(arr)

    digit = 1

    while int(maxValue / digit) > 0: 
        counting_sort(arr, digit)
        digit *= 10
