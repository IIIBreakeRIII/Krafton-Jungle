# 3 x 3 행렬 선택 게임
# https://www.notion.so/1intheworldhsryu/Week-1-6th-BOJ-Brute-Force-23027535a87280aebb17de6c5b3e2ad8?source=copy_link#23127535a87280f0b45eeef09a4b1f5c

import sys

li = [[1, 5, 3], [2, 5, 7], [5, 3, 5]]
check = [False] * 3
m = sys.maxsize

def backtracking(row, score):

    global m

    if row == 3:
        m = min(m, score)
        return

    for i in range(3):
        if not check[i]:
            check[i] = True
            backtracking(row + 1, score + li[row][i])
            check[i] = False

backtracking(0, 0)
print(m)        # 8
