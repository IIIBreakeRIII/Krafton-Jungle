# 선택 정렬 알고리즘

def selection_sort(num_list):
    for i in range(len(num_list)):
        min_index = i

        for j in range(i + 1, len(num_list)):
            if num_list[j] < num_list[min_index]:
                min_index = j

        num_list[i], num_list[min_index] = num_list[min_index], num_list[i]
    
    return num_list

num_list = [9, 6, 5, 3, 2]

print(selection_sort(num_list))
