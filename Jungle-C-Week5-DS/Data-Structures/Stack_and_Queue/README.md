# 스택과 큐 문제 모음

## 1. createQueueFromLinkedList
`createQueueFromLinkedList()` 함수는 연결 리스트에 저장된 모든 정수를 **큐(연결 리스트 기반)** 에 차례대로 enqueue 하여 큐를 생성합니다.  
연결 리스트의 첫 번째 노드부터 순서대로 큐에 삽입하며, **큐가 비어 있지 않으면 먼저 비웁니다.**

**함수 원형**
```c
void createQueueFromLinkedList(LinkedList *ll, Queue *q);
```

**예시**
- 연결 리스트: `1 2 3 4 5`  
  결과 큐: `1 2 3 4 5`

---

## 2. createStackFromLinkedList
`createStackFromLinkedList()` 함수는 연결 리스트에 저장된 모든 정수를 **스택(연결 리스트 기반)** 에 push 하여 스택을 생성합니다.  
연결 리스트의 첫 번째 노드부터 순서대로 push 하며, **스택이 비어 있지 않으면 먼저 비웁니다.**

**함수 원형**
```c
void createStackFromLinkedList(LinkedList *ll, Stack *stack);
```

**예시**
- 연결 리스트: `1 3 5 6 7`  
  결과 스택: `7 6 5 3 1`

---

## 3. isStackPairwiseConsecutive
`isStackPairwiseConsecutive()` 함수는 스택의 원소들이 **순서쌍으로 연속적인 값인지** 검사합니다.  
push()와 pop()만을 사용하여 스택에서 값을 넣거나 뺄 수 있습니다.

**함수 원형**
```c
int isStackPairwiseConsecutive(Stack *s);
```

**예시**
- 스택: `(16, 15, 11, 10, 5, 4)` → 연속 쌍 → true  
- 스택: `(16, 15, 11, 10, 5, 1)` → 연속 쌍 아님 → false  
- 스택: `(16, 15, 11, 10, 5)` → 원소 수 홀수 → false

---

## 4. reverseQueue
`reverseQueue()` 함수는 **스택을 사용하여 큐의 순서를 뒤집는** 함수입니다.  
스택 조작은 push()/pop()만, 큐 조작은 enqueue()/dequeue()만 사용해야 하며, 스택은 사용 전 비워야 합니다.

**함수 원형**
```c
void reverseQueue(Queue *q);
```

**예시**
- 큐: `1 2 3 4 5`  
  결과: `5 4 3 2 1`

---

## 5. recursiveReverseQueue
`recursiveReverseQueue()` 함수는 **재귀적으로 큐의 순서를 뒤집는** 함수입니다.

**함수 원형**
```c
void recursiveReverseQueue(Queue *q);
```

**예시**
- 큐: `1 2 3 4 5`  
  결과: `5 4 3 2 1`

---

## 6. removeUntilStack
`removeUntilStack()` 함수는 **특정 값이 나올 때까지 스택의 값을 pop** 합니다.

**함수 원형**
```c
void removeUntilStack(Stack *s, int value);
```

**예시**
- 스택: `(1, 2, 3, 4, 5, 6, 7)`, value=4 → 결과: `(4, 5, 6, 7)`  
- 스택: `(10, 20, 15, 25, 5)`, value=15 → 결과: `(15, 25, 5)`

---

## 7. balanced
`balanced()` 함수는 문자열이 **괄호 ()[]{}로 구성되어 균형 잡혔는지** 검사합니다.  
올바른 괄호 짝과 순서가 맞으면 balanced, 아니면 not balanced.

**함수 원형**
```c
int balanced(char *expression);
```

**예시**
- Balanced:  
  - `()`  
  - `([])`  
  - `{[]()[]}`
- Not balanced:  
  - `{{)]`  
  - `[({{)])`
