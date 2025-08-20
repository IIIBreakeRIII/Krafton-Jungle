class UnionFind:
    def __init__(self, size):
        # 부모 노드를 자기 자신으로 초기화
        self.parent = [i for i in range(size)]
        # 각 노드의 트리 높이 (랭크)를 기록
        self.rank = [0] * size

    def find(self, x):
        # 경로 압축(Path Compression)을 통한 루트 찾기
        if self.parent[x] != x:
            # 부모를 재귀적으로 찾아서 루트로 설정
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]

    def union(self, x, y):
        # 두 원소가 속한 집합을 합침
        rootX = self.find(x)
        rootY = self.find(y)

        # 루트가 같으면 이미 같은 집합
        if rootX == rootY:
            return

        # 높이가 낮은 트리를 높은 트리 아래에 붙임 (Union by Rank)
        if self.rank[rootX] < self.rank[rootY]:
            self.parent[rootX] = rootY
        elif self.rank[rootX] > self.rank[rootY]:
            self.parent[rootY] = rootX
        else:
            self.parent[rootY] = rootX
            self.rank[rootX] += 1

        print(self.parent)

    def connected(self, x, y):
        # 두 원소가 같은 집합에 속하는지 확인
        return self.find(x) == self.find(y)
        
# 노드 0부터 4까지 총 5개의 노드 초기화
uf = UnionFind(5)

# 0과 1을 같은 집합으로 합침
uf.union(0, 1)

# 1과 2를 같은 집합으로 합침
uf.union(1, 2)

# 3과 4를 같은 집합으로 합침
uf.union(3, 4)

# 0과 2는 같은 집합 (True)
print(uf.connected(0, 2))

# 0과 3은 다른 집합 (False)
print(uf.connected(0, 3))
