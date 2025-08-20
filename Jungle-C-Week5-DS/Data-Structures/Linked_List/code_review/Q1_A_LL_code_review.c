#include <stdio.h>
#include <stdlib.h>

typedef struct _listnode{
	int item;
	struct _listnode *next;
} ListNode;

typedef struct _linkedlist{
	int size;
	ListNode *head;
} LinkedList;

int insertSortedLL(LinkedList *ll, int item);

void printList(LinkedList *ll);
void removeAllItems(LinkedList *ll);
ListNode *findNode(LinkedList *ll, int index);
int insertNode(LinkedList *ll, int index, int value);
int removeNode(LinkedList *ll, int index);

int main() {
	LinkedList ll;
	int c, i, j;
	c = 1;

	ll.head = NULL;
	ll.size = 0;
  
	printf("1: Insert an integer to the sorted linked list:\n");
	printf("2: Print the index of the most recent input value:\n");
	printf("3: Print sorted linked list:\n");
	printf("0: Quit:");
  
  // Input = c = 0 -> if c is not quit
  while (c != 0) {
		printf("\nPlease input your choice(1/2/3/0): ");
		scanf("%d", &c);

		switch (c) {
		  case 1:
		  	printf("Input an integer that you want to add to the linked list: ");
		  	scanf("%d", &i);
		  	j = insertSortedLL(&ll, i);
		  	printf("The resulting linked list is: ");
		  	printList(&ll);
		  	break;
		  case 2:
		  	printf("The value %d was added at index %d\n", i, j);
		  	break;
		  case 3:
		  	printf("The resulting sorted linked list is: ");
		  	printList(&ll);
		  	removeAllItems(&ll);
		  	break;
		  case 0:
		  	removeAllItems(&ll);
		  	break;
		  default:
		  	printf("Choice unknown;\n");
		  	break;
		}
	}

	return 0;
}

// 정렬 상태를 유지하며 item을 삽입하고, 삽입된 인덱스를 반환
// 중복 값은 삽입하지 않고 -1 반환
int insertSortedLL(LinkedList *ll, int item) {

  // 리스트 포인터 자체가 NULL인 경우 (올바르지 않은 인자)
  if (ll == NULL) return -1;
  
  // 현재 노드를 헤드로 초기화
  ListNode *cur = ll->head;
  int idx = 0;
  
  // 빈 리스트인 경우 (head == NULL): 0번 위치에 삽입
  if (cur == NULL) {
    if (insertNode(ll, 0, item) == 0) return 0;
    return -1;
  }
  
  // 첫 노드 값보다 작으면 맨 앞에 삽입
  if (item < cur->item) {
    if (insertNode(ll, 0, item) == 0) return 0;
    return -1;
  }
  
  // 첫 노드부터 끝까지 순회
  while (cur != NULL) {
    // 현재 노드 값이 item과 같으면 (중복) 삽입하지 않음
    if (cur->item == item) {
      return -1;
    }
    // 다음 노드가 없거나(next == NULL) 다음 노드 값이 item보다 크거나 같으면
    // 현재 노드(cur) 다음 위치(idx+1)에 삽입 후보
    if (cur->next == NULL || cur->next->item >= item) {
      int insert_index = idx + 1;
      // 다음 노드 값이 동일하면(중복) 삽입하지 않음
      if (cur->next && cur->next->item == item) return -1;
      
      if (insertNode(ll, insert_index, item) == 0) return insert_index;
      return -1;
    }

    cur = cur->next;
    idx++;
  }

  return -1;
}

void printList(LinkedList *ll){

	ListNode *cur;
	if (ll == NULL)
		return;
	cur = ll->head;

	if (cur == NULL)
		printf("Empty");
	while (cur != NULL)
	{
		printf("%d ", cur->item);
		cur = cur->next;
	}
	printf("\n");
}

void removeAllItems(LinkedList *ll)
{
	ListNode *cur = ll->head;
	ListNode *tmp;

	while (cur != NULL){
		tmp = cur->next;
		free(cur);
		cur = tmp;
	}
	ll->head = NULL;
	ll->size = 0;
}

ListNode *findNode(LinkedList *ll, int index){

	ListNode *temp;

	if (ll == NULL || index < 0 || index >= ll->size)
		return NULL;

	temp = ll->head;

	if (temp == NULL || index < 0)
		return NULL;

	while (index > 0){
		temp = temp->next;
		if (temp == NULL)
			return NULL;
		index--;
	}

	return temp;
}

int insertNode(LinkedList *ll, int index, int value){

	ListNode *pre, *cur;

	if (ll == NULL || index < 0 || index > ll->size + 1)
		return -1;

	if (ll->head == NULL || index == 0){
		cur = ll->head;
		ll->head = malloc(sizeof(ListNode));
		ll->head->item = value;
		ll->head->next = cur;
		ll->size++;
		return 0;
	}

	if ((pre = findNode(ll, index - 1)) != NULL){
		cur = pre->next;
		pre->next = malloc(sizeof(ListNode));
		pre->next->item = value;
		pre->next->next = cur;
		ll->size++;
		return 0;
	}

	return -1;
}


int removeNode(LinkedList *ll, int index){

	ListNode *pre, *cur;

	if (ll == NULL || index < 0 || index >= ll->size)
		return -1;

	if (index == 0){
		cur = ll->head->next;
		free(ll->head);
		ll->head = cur;
		ll->size--;

		return 0;
	}

	if ((pre = findNode(ll, index - 1)) != NULL){

		if (pre->next == NULL)
			return -1;

		cur = pre->next;
		pre->next = cur->next;
		free(cur);
		ll->size--;
		return 0;
	}

	return -1;
}
