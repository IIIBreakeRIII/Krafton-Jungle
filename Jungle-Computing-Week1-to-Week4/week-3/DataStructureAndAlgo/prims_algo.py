import heapq
from collections import defaultdict

def prim(nodes, adj):
    visited = [False] * nodes
    min_heap = [(0, 0, -1)]  # (weight, u, from) - 시작 노드는 from=-1로 표시
    mst = []
    total_cost = 0

    while min_heap:
        weight, u, prev = heapq.heappop(min_heap)
        if visited[u]:
            continue
        visited[u] = True
        total_cost += weight
        if prev != -1:  # 시작 노드는 제외하고 간선만 추가
            mst.append((prev, u, weight))

        for v, w in adj[u]:
            if not visited[v]:
                heapq.heappush(min_heap, (w, v, u))  # u에서 v로 가는 간선

    return mst, total_cost

# 입력 그래프 예제
adj = defaultdict(list)
edges = [(0, 1, 4), (0, 2, 3), (1, 2, 1), (1, 3, 2), (2, 3, 4)]
for u, v, w in edges:
    adj[u].append((v, w))
    adj[v].append((u, w))  # 무방향 그래프

mst_edges, cost = prim(4, adj)
print("MST edges:", mst_edges)
print("Total cost:", cost)
