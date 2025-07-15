N = 8
# data = [5,2,3,4,1]
data = [4, 8, 2, 1, 5, 7, 6, 3]

# def swap(arr, i, j):
#     arr[i], arr[j] = arr[j], arr[i]
# 
# def pivot_helper(arr, start, end):
#     mid = (start + end) // 2
#     swap(arr, start, mid)  # 중간값을 피벗으로 사용
#     pivot = arr[start]
#     swap_idx = start
# 
#     for i in range(start + 1, end + 1):
#         if arr[i] < pivot:
#             swap_idx += 1
#             swap(arr, swap_idx, i)
#     swap(arr, start, swap_idx)
#     return swap_idx
# 
# def quick_sort_with_helper(arr,left,right) :
#     while left < right:
#         pivot_idx = pivot_helper(arr, left, right)
# 
#         # 작은 쪽 먼저 재귀 호출
#         if pivot_idx - left < right - pivot_idx:
#             quick_sort(arr, left, pivot_idx - 1)
#             left = pivot_idx + 1
#         else:
#             quick_sort(arr, pivot_idx + 1, right)
#             right = pivot_idx - 1
#     return arr

# 위 코드를 하나의 함수안에 표현
# lomuto 방식 Pivot을 기준으로 나누는 방식 - 1
def quick(arr, left, right):
    while left < right:
        
        # arr를 pivot 기준으로 좌, 우 분할
        mid = (left + right) // 2
        arr[left], arr[mid] = arr[mid], arr[left] # 중간값을 피벗으로 선택하기 위해 스왑
        pivot_idx = left
        pivot = arr[pivot_idx]

        for i in range(left + 1, right + 1):
            if arr[i] < pivot:
                pivot_idx += 1
                # pivot보다 작은 애를 찾으면 추후에 pivot에 적용할 pivot_idx를 1 증가시켜 놓고 pivot 앞으로 하나씩 쌓아둠 (현재 pivot은 일단 arr 첫 자리에 고정)
                arr[i], arr[pivot_idx] = arr[pivot_idx], arr[i]
        # arr내 pivot보다 작은 수 찾는 과정이 끝나면 arr 첫 자리에 고정해놨던 pivot을 실제로 그동안 증가시킨 pivot_idx 위치로 이동시킴 => 이로써 pivot 왼쪽엔 pivot보다 작은 수, 오른쪽엔 pivot보다 큰 수가 모이게 됨
        arr[left], arr[pivot_idx] = arr[pivot_idx], arr[left] 

        # 이제 재귀 호출해서 잘게 쪼개서 정렬하도록 함
        # 작은 쪽 먼저 재귀 호출 (스택 깊이 최적화)
        if pivot_idx - left < right - pivot_idx:
            quick(arr, left, pivot_idx - 1)
            left = pivot_idx + 1
        else:
            quick(arr, pivot_idx + 1, right)
            right = pivot_idx - 1

    return arr

# 출력
for n in quick(data, 0, N-1):
    print(n)


#hoare 방식
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
