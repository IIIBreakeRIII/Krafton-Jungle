# Longest Common Subsequence using Recursion

str1 = input()
str2 = input()

def LCS(s1, s2):

    # 하나라도 비어있을 경우 return 0
    if s1 == "" or s2 =="":
        return 0

    # 마지막 글자가 같을 경우
    if s1[-1] == s2[-1]:
        return LCS(s1[:-1], s2[:-1]) + 1
    # 마지막 글자가 다른 경우
    else:
        # 각 글자의 마지막 글자 -1
        case1 = LCS(s1[:-1], s2)
        case2 = LCS(s1, s2[:-1])

        return max(case1, case2)

print(LCS(str1, str2))
