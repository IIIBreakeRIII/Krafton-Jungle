# 이진 트리 과제 문제 모음

본 문서는 이진 트리(BTNode)를 다루는 다양한 C 언어 실습 문제들을 다룹니다. 각 문제는 함수 설명, 프로토타입, 샘플 입출력, 예시 트리 구조 등을 포함합니다.

---

## 1. `identical()`  
두 개의 이진 트리가 구조적으로 동일한지 확인하는 재귀 함수

- **함수 원형**:  
  `int identical(BTNode *tree1, BTNode *tree2);`

- **설명**:  
  두 트리가 모두 비어있거나, 루트 값이 같고 각각의 왼쪽 및 오른쪽 서브트리가 동일하면 구조적으로 동일하다고 판단합니다.

- **예시 출력**:  
  "Both trees are structurally identical."

---

## 2. `maxHeight()`  
이진 트리의 최대 높이를 반환하는 함수

- **함수 원형**:  
  `int maxHeight(BTNode *root);`

- **설명**:  
  주어진 루트로부터 가장 먼 리프 노드까지의 링크 수를 반환합니다. 빈 트리의 높이는 `-1`입니다.

- **예시 출력**:  
  "The maximum height of the binary tree is: 2"

---

## 3. `countOneChildNodes()`  
정확히 하나의 자식만 가진 노드의 수를 반환하는 함수

- **함수 원형**:  
  `int countOneChildNodes(BTNode *root);`

- **설명**:  
  자식 노드가 하나뿐인 노드의 수를 재귀적으로 카운트합니다.

- **예시 출력**:  
  "The Number of nodes that have exactly one child node is: 2"

---

## 4. `sumOfOddNodes()`  
이진 트리 내 홀수 값들의 합을 반환하는 재귀 함수

- **함수 원형**:  
  `int sumOfOddNodes(BTNode *root);`

- **설명**:  
  홀수 노드의 값을 누적하여 합산합니다. 짝수 노드는 무시됩니다.

- **예시 출력**:  
  "The sum of all odd numbers in the binary tree is: 131"

---

## 5. `mirrorTree()`  
트리를 자기 자신의 미러 트리로 변환하는 재귀 함수

- **함수 원형**:  
  `void mirrorTree(BTNode *node);`

- **설명**:  
  좌우 자식 노드를 교환하여 미러 구조를 만듭니다. 임시 트리를 만들지 않고 직접 변경합니다.

- **예시 출력**:  
  "Mirror binary tree is: 1 2 3 4 6 5"

---

## 6. `printSmallerValues()`  
주어진 값보다 작은 트리 노드 값을 모두 출력하는 함수

- **함수 원형**:  
  `void printSmallerValues(BTNode *node, int m);`

- **설명**:  
  값 `m`보다 작은 값을 갖는 모든 노드를 출력합니다. (in-order 순회)

- **예시 출력**:  
  "The values smaller than 55 are: 50 30 25 10"

---

## 7. `smallestValue()`  
이진 트리에서 가장 작은 값을 찾는 함수

- **함수 원형**:  
  `int smallestValue(BTNode *node);`

- **설명**:  
  재귀적으로 탐색하여 트리 내 가장 작은 값을 반환합니다.

- **예시 출력**:  
  "Smallest value of the binary tree is: 10"

---

## 8. `hasGreatGrandchild()`  
증손자 노드가 하나라도 있는 노드의 값을 출력하는 함수

- **함수 원형**:  
  `int hasGreatGrandchild(BTNode *node);`

- **설명**:  
  깊이(depth)가 3 이상인 하위 노드가 존재하는 경우, 해당 노드의 값을 출력합니다.

- **예시 출력**:  
  "The values stored in all nodes of the tree that has at least one great-grandchild are: 50"

---

📝 **참고 사항**:
- 트리 입력 시 숫자가 아닌 문자는 NULL로 간주됩니다.
- 대부분의 트리 순회는 **전위(pre-order)** 혹은 **중위(in-order)** 방식으로 구현됩니다.
