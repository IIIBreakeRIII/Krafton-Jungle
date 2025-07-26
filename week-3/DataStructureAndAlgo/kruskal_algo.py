# 크루스칼 알고리즘을 위한 Union-Find(Disjoint Set) 구조 정의
class UnionFind:
    def __init__(self, n):
        self.parent = list(range(n))  # 각 노드는 자기 자신이 부모
        self.rank = [0] * n           # 트리의 높이(최적화를 위한 랭크)

    def find(self, u):
        if self.parent[u] != u:
            self.parent[u] = self.find(self.parent[u])  # 경로 압축
        return self.parent[u]

    def union(self, u, v):
        root_u = self.find(u)
        root_v = self.find(v)
        if root_u == root_v:
            return False  # 이미 같은 집합이면 합치지 않음 (사이클 방지)
        if self.rank[root_u] < self.rank[root_v]:
            self.parent[root_u] = root_v
        else:
            self.parent[root_v] = root_u
            if self.rank[root_u] == self.rank[root_v]:
                self.rank[root_u] += 1
        return True

def kruskal(n, edges):
    # 간선을 가중치 기준으로 정렬
    edges.sort(key=lambda x: x[2])  # (u, v, weight)
    uf = UnionFind(n)
    mst = []
    total_cost = 0

    for u, v, weight in edges:
        if uf.union(u, v):  # 사이클이 생기지 않는 경우에만 간선 추가
            mst.append((u, v, weight))
            total_cost += weight

    return mst, total_cost

# 예시 사용
edges = [(0, 1, 4), (0, 2, 3), (1, 2, 1), (1, 3, 2), (2, 3, 4)]
mst_edges, cost = kruskal(4, edges)
print("MST edges:", mst_edges)
print("Total cost:", cost)
