#include <stdio.h>
#include <stdlib.h>

// 노드 구조체 정의
typedef struct Node {
    int data;            // 데이터 저장
    struct Node* next;   // 다음 노드 주소 저장
} Node;

// 새로운 노드 생성 함수
Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node)); // 동적 메모리 할당
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// 리스트에 노드 추가 (맨 뒤에 추가)
void appendNode(Node** head, int data) {
    Node* newNode = createNode(data);

    if (*head == NULL) { // 리스트가 비어있을 경우
        *head = newNode;
    } else {
        Node* temp = *head;
        while (temp->next != NULL) { // 끝까지 이동
            temp = temp->next;
        }
        temp->next = newNode; // 마지막 노드에 연결
    }
}

// 리스트 출력
void printList(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        printf("%d -> ", temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}

// 메모리 해제
void freeList(Node* head) {
    Node* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

int main() {
    Node* head = NULL; // 빈 리스트 초기화

    // 노드 추가
    appendNode(&head, 10);
    appendNode(&head, 20);
    appendNode(&head, 30);

    // 리스트 출력
    printf("Linked List: ");
    printList(head);

    // 메모리 해제
    freeList(head);

    return 0;
}
