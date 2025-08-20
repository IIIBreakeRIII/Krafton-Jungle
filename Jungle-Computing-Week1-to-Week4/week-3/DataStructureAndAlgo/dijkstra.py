import heapq

# 그래프 정의 (인접 리스트 방식)
graph = {
    'A': [('B', 3), ('C', 10), ('D', 9)],
    'B': [('A', 3), ('D', 4)],
    'C': [('A', 10), ('D', 1), ('E', 2), ('F', 5)],
    'D': [('A', 9), ('B', 4), ('C', 1), ('F', 6)],
    'E': [('C', 2), ('F', 2), ('G', 3)],
    'F': [('C', 5), ('D', 6), ('E', 2), ('G', 5)],
    'G': [('F', 5), ('E', 3)]
}

def dijkstra(graph, start):
    # 거리 테이블 초기화
    distances = {node: float('inf') for node in graph}
    distances[start] = 0
    
    # 우선순위 큐: (거리, 노드)
    queue = [(0, start)]
    
    while queue:
        current_dist, current_node = heapq.heappop(queue)
        
        if current_dist > distances[current_node]:
            continue
        
        for neighbor, weight in graph[current_node]:
            distance = current_dist + weight
            if distance < distances[neighbor]:
                distances[neighbor] = distance
                heapq.heappush(queue, (distance, neighbor))
    
    return distances

# 실행
shortest_paths = dijkstra(graph, 'A')
print("최단 거리 결과:")
for node, dist in shortest_paths.items():
    print(f"{node}: {dist}")
