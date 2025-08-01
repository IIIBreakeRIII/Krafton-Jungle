# Longest Common Subsequence using Memoization
# Example input = caema, ecrea

str1 = input()
str2 = input()

len_str1 = len(str1)
len_str2 = len(str2)

memoization = [[-1] * (len_str1 + 1) for _ in range(len_str2 + 1)] 

def LCS(s1, s2):
    m, n = len(s1), len(s2)

    # 둘 중 하나라도 0인 값이면 return 0
    if m == 0 or n == 0:
        return 0
    
    # 이미 계산한 m, n이라면 그 값을 그대로 반환
    # 중복 방지
    if memoization[m][n] != -1:
        return memoization[m][n]
    
    # 마지막 글자를 비교
    # 같다면 두 단어 모두 끝을 자름
    if s1[-1] == s2[-1]:
        memoization[m][n] = LCS(s1[:-1], s2[:-1]) + 1
        return memoization[m][n]
    # 다르다면 max값을 반환하도록
    else:
        case1 = LCS(s1[:-1], s2)
        case2 = LCS(s1, s2[:-1])
        memoization[m][n] = max(case1, case2)

        return memoization[m][n]

print(LCS(str1, str2))
