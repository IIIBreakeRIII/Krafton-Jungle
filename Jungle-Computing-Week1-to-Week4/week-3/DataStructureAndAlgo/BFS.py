import collections

def bfs(graph, start):

    visited, queue = set(), collections.deque([start])
    visited.add(start)

    while queue:

        node = queue.popleft()
        print(str(node) + " ", end="")

        for neighbor in graph[node]:
            if neighbor not in visited:
                visited.add(neighbor)
                queue.append(neighbor)

if __name__ == "__main__":
    graph = {0: [1, 2, 3], 1: [0, 2], 2: [0, 1, 4], 3:[0], 4: [2]}
    print("Following is BFS : ")
    bfs(graph, 0)
