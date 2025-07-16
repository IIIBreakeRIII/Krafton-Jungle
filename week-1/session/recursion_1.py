# n + (n - 1) + (n - 2) + ... + 1
def nsum(n):
    ans = 0
    for i in range(n):
        ans += i

    return ans + 10

# nsum using Recursion
def recusrion_nsum(n):
    if n == 0:
        return 0

    return recusrion_nsum(n - 1) + n

# num * n using recursion
def recursion_exp(total, a, n):
    
    if n == 0:
        return total

    return recursion_exp(total * a, a, n - 1)

# Fast Exponentiation (빠른 거듭제곱)
def recursion_fast_exp(x, n, total = 1):

    if n == 0:
        return total
    
    elif n % 2 == 0:
        return recursion_fast_exp(x * x, n // 2, total)
    else:
        return recursion_fast_exp(x, n - 1, total * x)
