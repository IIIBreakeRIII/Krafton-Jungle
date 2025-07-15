# BOJ 9663 N-Queen
# https://www.notion.so/1intheworldhsryu/Week-1-6th-BOJ-Brute-Force-23027535a87280aebb17de6c5b3e2ad8?source=copy_link#23127535a872803eac1ad663c306ef36

N = int(input())

col_check = [False]*N
diagonal_check1 = [False]*(2*N-1)
diagonal_check2 = [False]*(2*N-1)

count = 0

def func(r):
    global count
    if r == N:
        count += 1
        return

    for i in range(N):
        if col_check[i] or diagonal_check1[i + r] or diagonal_check2[N - 1 - i + r]: 
            continue
            
        col_check[i] = True 
        diagonal_check1[i + r] = True
        diagonal_check2[N - 1 - i + r] = True
        func(r + 1)

        col_check[i] = False
        diagonal_check1[i + r] = False
        diagonal_check2[N - 1 - i + r] = False

func(0)
print(count)
