# KMP Algorithms

def search(pat, txt):
    res = []
    n = len(txt)
    m = len(pat)

    for i in range(n - m + 1):
        j = 0

        # compare pattern with substring 
        # starting at index i
        while j < m and txt[i + j] == pat[j]:
            j += 1

        # if full pattern matched
        if j == m:
            res.append(i)

    return res


if __name__ == "__main__":
    txt = "aabaacaadaabaaba"
    pat = "aaba"

    res = search(pat, txt)
    for idx in res:
        print(idx, end=" ")
