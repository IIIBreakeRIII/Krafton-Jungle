# No.11403 경로 찾기

n = int(input())
graph = [list(map(int, input().split())) for _ in range(n)]

# 플로이드-워셜
for k in range(n):
    for i in range(n):
        for j in range(n):
            if graph[i][k] and graph[k][j]:
                graph[i][j] = 1

# 출력
for row in graph:
    print(*row)
