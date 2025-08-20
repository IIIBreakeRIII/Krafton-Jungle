graph = {
    'A': ['B', 'C'],
    'B': ['A', 'D', 'E'],
    'C': ['A', 'F'],
    'D': ['B'],
    'E': ['B', 'F'],
    'F': ['C', 'E']
}

visited = {key: False for key in graph.keys()}

def dfs_recursive(graph, visited, node):
    if not visited[node]:
        print(node, end=' ')
        visited[node] = True
        for neighbour in graph[node]:
            dfs_recursive(graph, visited, neighbour)

def dfs_stack(graph, start):
    visited = set()
    stack = [start]

    while stack:
        node = stack.pop()
        if node not in visited:
            print(node, end=' ')
            visited.add(node)
            stack.extend([x for x in graph[node] if x not in visited])

print("DFS 방문 순서(재귀 사용)")
dfs_recursive(graph, visited, 'A')
print()

print("DFS 방문 순서(스택 사용)")
dfs_stack(graph, 'A')
