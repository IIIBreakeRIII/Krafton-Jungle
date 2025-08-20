# 연결 리스트 문제 모음

## 1. insertSortedLL
`insertSortedLL()` 함수는 사용자가 입력한 정수를 **오름차순으로 정렬된 연결 리스트**에 삽입하는 C 함수입니다.  
이미 존재하는 값은 삽입하지 않으며, 삽입 성공 시 **해당 인덱스**를 반환하고, 실패 시 **-1**을 반환합니다.

**함수 원형**
```c
int insertSortedLL(LinkedList *ll, int item);
```

**예시**
- 현재 리스트: `2, 3, 5, 7, 9`  
  `insertSortedLL(8)` → `2, 3, 5, 7, 8, 9` (반환: `4`)

- 현재 리스트: `5, 7, 9, 11, 15`  
  `insertSortedLL(7)` → 변경 없음 (반환: `-1`)

---

## 2. alternateMergeLL
`alternateMergeLL()` 함수는 두 번째 리스트의 노드를 첫 번째 리스트의 **번갈아 가며 위치**에 삽입합니다.

**함수 원형**
```c
void alternateMergeLL(LinkedList *ll1, LinkedList *ll2);
```

**예시**
- L1: `1, 2, 3`  
  L2: `4, 5, 6, 7`  
  결과 → L1: `1, 4, 2, 5, 3, 6`, L2: `7`

---

## 3. moveOddItemsToBackLL
`moveOddItemsToBackLL()` 함수는 연결 리스트의 **홀수 값**을 모두 뒤로 이동시킵니다.

**함수 원형**
```c
void moveOddItemsToBackLL(LinkedList *ll);
```

**예시**
- 입력: `2, 3, 4, 7, 15, 18`  
  결과: `2, 4, 18, 3, 7, 15`

---

## 4. moveEvenItemsToBackLL
`moveEvenItemsToBackLL()` 함수는 연결 리스트의 **짝수 값**을 모두 뒤로 이동시킵니다.

**함수 원형**
```c
void moveEvenItemsToBackLL(LinkedList *ll);
```

**예시**
- 입력: `2, 3, 4, 7, 15, 18`  
  결과: `3, 7, 15, 2, 4, 18`

---

## 5. frontBackSplitLL
`frontBackSplitLL()` 함수는 연결 리스트를 **앞 절반**과 **뒤 절반**으로 분할합니다.  
노드 수가 홀수일 경우 **앞 리스트**가 하나 더 가집니다.

**함수 원형**
```c
void frontBackSplitLL(LinkedList *ll, LinkedList *resultFrontList, LinkedList *resultBackList);
```

**예시**
- 입력: `2, 3, 5, 6, 7`  
  결과 → frontList: `2, 3, 5`, backList: `6, 7`

---

## 6. moveMaxToFront
`moveMaxToFront()` 함수는 한 번의 순회로 **최대값 노드**를 찾아 리스트 맨 앞으로 이동시킵니다.

**함수 원형**
```c
int moveMaxToFront(ListNode **ptrHead);
```

**예시**
- 입력: `30, 20, 40, 70, 50`  
  결과: `70, 30, 20, 40, 50`

---

## 7. recursiveReverse
`recursiveReverse()` 함수는 **재귀적으로 연결 리스트를 뒤집는** 함수입니다.

**함수 원형**
```c
void recursiveReverse(ListNode **ptrHead);
```

**예시**
- 입력: `1, 2, 3, 4, 5`  
  결과: `5, 4, 3, 2, 1`
