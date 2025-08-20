#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * 함수 역할: 새로운 트리 인스턴스를 생성하고 공용 센티넬(nil) 노드를 설정
 * 불변식: nil은 항상 BLACK이며, 초기에는 root가 nil을 가리킴(빈 트리)
 */
rbtree *new_rbtree(void) {
  
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));          // 트리 구조체 메모리 확보(0 초기화)
  node_t *nilNode = (node_t *)calloc(1, sizeof(node_t));    // 공용 센티넬 노드 확보

  nilNode->color = RBTREE_BLACK;                             // [중요] RB 속성: nil(리프)은 BLACK
  p->nil = nilNode;                                          // 트리 전역에서 공유될 nil 지정
  p->root = nilNode;                                         // 초기 루트는 nil(빈 트리)

  return p;
}

// 오른쪽 순회(우회전)
/*
 * 함수 역할: x를 기준으로 우회전하여 (y=x->left를 상향 승격) 서브트리의 형태를 재배치
 * 핵심 판단:
 *  - (y->right != tree->nil): 이동되는 서브트리가 실노드인지(부모 포인터 갱신 필요 여부)
 *  - (x->parent == tree->nil): 회전 후 y가 새 루트가 되는지
 *  - (x == x->parent->left / else): x가 부모의 어느 쪽 자식이었는지에 따른 부모-자식 연결 갱신
 */
void right_rotation(rbtree *tree, node_t *x) {
  
  node_t *y;

  y = x->left;                        // 우회전의 피벗 노드
  x->left = y->right;                 // y의 오른쪽 서브트리를 x의 왼쪽으로 이동
  
  if (y->right != tree->nil) {        // 이동된 서브트리가 nil이 아니면 그 부모를 x로 갱신
    y->right->parent = x;
  }

  y->parent = x->parent;               // y를 x의 기존 부모 아래로 끌어올림

  if (x->parent == tree->nil) {        // x가 루트였다면 회전 후 y가 새 루트가 됨
    tree->root = y;
  }
  else if (x == x->parent->left) {     // x가 왼쪽 자식이었다면 그 자리에 y를 연결
    x->parent->left = y;
  }
  else {                               // x가 오른쪽 자식이었다면 그 자리에 y를 연결
    x->parent->right = y;
  }

  y->right = x;                        // y의 오른쪽 자식으로 x 배치
  x->parent = y;                       // x의 부모를 y로 갱신
}

// 왼쪽 순회(좌회전)
/*
 * 함수 역할: x를 기준으로 좌회전하여 (y=x->right를 상향 승격) 서브트리의 형태를 재배치
 * 핵심 판단:
 *  - (y->left != tree->nil): 이동되는 서브트리가 실노드인지(부모 포인터 갱신 필요 여부)
 *  - (x->parent == tree->nil): 회전 후 y가 새 루트가 되는지
 *  - (x == x->parent->left / else): x가 부모의 어느 쪽 자식이었는지에 따른 부모-자식 연결 갱신
 */
void left_rotation(rbtree *tree, node_t *x) {
  node_t *y;

  y = x->right;                       // 좌회전의 피벗 노드
  x->right = y->left;                 // y의 왼쪽 서브트리를 x의 오른쪽으로 이동
  
  if (y->left != tree->nil) {         // 이동된 서브트리가 nil이 아니면 그 부모를 x로 갱신
    y->left->parent = x;
  }

  y->parent = x->parent;              // y를 x의 기존 부모 아래로 끌어올림

  if (x->parent == tree->nil) {       // x가 루트였다면 회전 후 y가 새 루트가 됨
    tree->root = y;
  }
  else if (x == x->parent->left) {    // x가 왼쪽 자식이었다면 그 자리에 y를 연결
    x->parent->left = y;
  }
  else {                              // x가 오른쪽 자식이었다면 그 자리에 y를 연결
    x->parent->right = y;
  }

  y->left = x;                        // y의 왼쪽 자식으로 x 배치
  x->parent = y;                      // x의 부모를 y로 갱신
}

/*
 * 함수 역할: 노드 x를 루트로 하는 서브트리를 후위순회(post-order)로 해제
 * 핵심 판단:
 *  - (x->left != t->nil): 왼쪽 서브트리가 비어있지 않으면 먼저 재귀 해제
 *  - (x->right != t->nil): 오른쪽 서브트리가 비어있지 않으면 다음 재귀 해제
 *  - 자신의 메모리를 마지막에 해제(후위순회)
 * 참고: nil은 공용 센티넬이므로 여기서 해제하지 않음
 */
void free_node(rbtree *t, node_t *x) {

  if (x->left != t->nil) {
    free_node(t, x->left);
  }

  if (x->right != t->nil) {
    free_node(t, x->right);
  }

  free(x);                            // 자식 해제 후 자기 자신 해제

  x = NULL;                           // 지역 변수 널링(호출자에는 영향 없음)
}

/*
 * 함수 역할: 트리 전체를 해제
 * 핵심 판단:
 *  - (t->root != t->nil): 트리가 비어있지 않은 경우에만 실제 노드들을 재귀 해제
 *  - 이후 공용 nil 해제, 마지막으로 트리 구조체 해제
 */
void delete_rbtree(rbtree *t) {

  if (t->root != t->nil) {
    free_node(t, t->root);
  }

  free(t->nil);                       // 공용 센티넬 해제
  free(t);
}

/*
 * 함수 역할: 삽입 후 RB 속성 위반(연속 RED 등)을 복구
 * 분기 구조:
 *  - while (부모가 RED): 위반 상황에서만 루프 진입
 *  - Case 분기: 부모가 조부모의 왼쪽/오른쪽 자식인지에 따른 대칭 처리
 *  - 삼촌(y)의 색에 따른 두 갈래:
 *    1) 삼촌 RED: 부모/삼촌 BLACK, 조부모 RED로 바꾸고 z를 조부모로 끌어올려 반복
 *    2) 삼촌 BLACK: 회전 + 재색칠로 고정
 * 주의: y가 nil일 수 있으나, nil은 BLACK이라는 불변식으로 인해 (y->color == BLACK) 판단이 가능
 */
void rbtree_insert_fixup(rbtree *t, node_t *z) {
  node_t *y;

  while (z->parent->color == RBTREE_RED) {                      // 부모가 RED → 속성 위반

    if (z->parent == z->parent->parent->left) {                 // 부모가 조부모의 왼쪽 자식인 경우
      y = z->parent->parent->right;                             // 삼촌: 조부모의 오른쪽 자식

      if (y->color == RBTREE_RED) {                             // [Case 1] 삼촌 RED → 재색칠로 높이 전파
        z->parent->color = RBTREE_BLACK;                        // 부모 BLACK
        y->color = RBTREE_BLACK;                                // 삼촌 BLACK
        z->parent->parent->color = RBTREE_RED;                  // 조부모 RED로 내려보냄
        
        z = z->parent->parent;                                  // 문제를 상위로 이동
      }
      else {                                                    // [Case 2] 삼촌 BLACK → 회전 필요
        if (z == z->parent->right) {                            // [2-1] 좌-우 형태(내부 꺾임)면 먼저 좌회전으로 직선화
          z = z->parent;
          left_rotation(t, z);
        }

        z->parent->color = RBTREE_BLACK;                        // 부모 BLACK
        z->parent->parent->color = RBTREE_RED;                  // 조부모 RED
        right_rotation(t, z->parent->parent);                   // 우회전으로 재배치
      }
    }
    else {                                                      // 부모가 조부모의 오른쪽 자식인 경우(대칭)
      y = z->parent->parent->left;                              // 삼촌: 조부모의 왼쪽 자식

      if (y->color == RBTREE_RED) {                             // [Case 1 대칭] 삼촌 RED
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        z = z->parent->parent;
      }
      else {                                                    // [Case 2 대칭] 삼촌 BLACK
        if (z == z->parent->left) {                             // [2-1 대칭] 우-좌 형태면 먼저 우회전
          z = z->parent;
          right_rotation(t, z);
        }

        z->parent->color = RBTREE_BLACK;                        // 부모 BLACK
        z->parent->parent->color = RBTREE_RED;                  // 조부모 RED
        left_rotation(t, z->parent->parent);                    // 좌회전으로 재배치
      }
    }
  }
  t->root->color = RBTREE_BLACK;                                // [마무리] 루트는 항상 BLACK
}

/*
 * 함수 역할: 키를 가진 새 노드를 BST 규칙대로 삽입하고, 색을 RED로 초기화한 후 fixup 호출
 * 핵심 판단:
 *  - while (x != nil): 삽입 위치 탐색(BST 하강)
 *  - (z->key < x->key / else): 좌/우로 하강 결정
 *  - (y == nil): 트리가 비어 있으면 새 노드가 루트가 됨
 *  - 좌/우 자식 연결 후, 새 노드의 자식은 nil, 색은 RED로 지정(표준 삽입 규칙)
 */
node_t *rbtree_insert(rbtree *t, const key_t key) {
  
  node_t *y = t->nil;                                  // y: x의 부모를 추적
  node_t *x = t->root;                                 // x: 탐색 포인터(현재 노드)
  node_t *z = (node_t *)calloc(1, sizeof(node_t));     // 새 노드

  z->key = key;

  while (x != t->nil) {                                // 삽입 지점 탐색
    y = x;
    if (z->key < x->key) {
      x = x->left;                                     // 더 작으면 왼쪽 하강
    }
    else {
      x = x->right;                                    // 크거나 같으면 오른쪽 하강
    }
  }

  z->parent = y;                                       // 부모 연결

  if (y == t->nil) {                                   // 트리가 비어있다면 새 노드가 루트
    t->root = z;
  }
  else if (z->key < y->key) {                          // 부모의 어느 쪽 자식으로 붙을지 결정
    y->left = z;
  }
  else {
    y->right = z;
  }

  z->left = t->nil;                                    // 새 노드의 자식은 nil로 초기화
  z->right = t->nil;
  z->color = RBTREE_RED;                               // 삽입 시 RED

  rbtree_insert_fixup(t, z);                           // 속성 위반 복구

  return z;
}

/*
 * 함수 역할: key를 가진 노드를 탐색한다(BST 탐색)
 * 핵심 판단:
 *  - (current == nil): 탐색 실패
 *  - (current->key == key): 탐색 성공
 *  - (current->key < key / else): 오른쪽/왼쪽으로 진행
 */
node_t *rbtree_find(const rbtree *t, const key_t key) {
  
  node_t *current = t->root;

  while (current != t->nil) {

    if(current->key == key) {
      return current;                                   // 일치하는 노드 발견
    }

    if (current->key < key) {
      current = current->right;                         // 더 크면 오른쪽으로
    }
    else {
      current = current->left;                          // 더 작으면 왼쪽으로
    }
  }

  return NULL;                                          // 없으면 NULL
}

/*
 * 함수 역할: 트리에서 최소 키 노드를 찾는다(가장 왼쪽 노드)
 * 핵심 판단:
 *  - (root == nil): 빈 트리면 NULL
 *  - (curr->left != nil): 더 왼쪽이 있으면 계속 진행
 */
node_t *rbtree_min(const rbtree *t) {

  if (t->root == t->nil) {
    return NULL;
  }
  
  node_t *curr = t->root;

  while (curr->left != t->nil) {
    curr = curr->left;
  }

  return curr;
}

/*
 * 함수 역할: 트리에서 최대 키 노드를 찾는다(가장 오른쪽 노드)
 * 핵심 판단:
 *  - (root == nil): 빈 트리면 NULL
 *  - (curr->right != nil): 더 오른쪽이 있으면 계속 진행
 */
node_t *rbtree_max(const rbtree *t) {
  
  if (t->root == t->nil) {
    return NULL;
  }

  node_t * curr = t->root;

  while (curr->right != t->nil) {
    curr = curr->right;
  }

  return curr;
}

/*
 * 함수 역할: 서브트리의 루트 u를 v로 치환하여 부모-자식 관계를 이식한다(BST 삭제 보조)
 * 핵심 판단:
 *  - (u->parent == nil): u가 루트였는지
 *  - (u == u->parent->left / else): u가 부모의 어느 쪽 자식이었는지
 *  - v가 nil일 수도 있으나, 부모 포인터를 u->parent로 일관 갱신(센티넬 포함)
 */
void rbtree_transplant(rbtree *t, node_t *u, node_t *v) {

  if (u->parent == t->nil) {            // u가 루트였다면 트리 루트를 v로 교체
    t->root = v;
  }
  else if (u == u->parent->left) {      // u가 왼쪽 자식이면 그 자리를 v로
    u->parent->left = v;
  }
  else {                                // u가 오른쪽 자식이면 그 자리를 v로
    u->parent->right = v;
  }

  v->parent = u->parent;                // v(또는 nil)의 부모를 갱신
}

/*
 * 함수 역할: 삭제 후 발생하는 이중흑색(double black) 등 RB 속성 위반을 복구
 * 루프 조건: (x != root && x가 BLACK) → x에 추가적인 흑색이 있는 경우만 진입
 * 분기 구조(좌측 경우, 우측은 대칭):
 *  - w: x의 형제
 *  - [Case 1] w가 RED → w를 BLACK, 부모를 RED로 만들고 부모를 기준으로 좌/우 회전하여
 *    형제를 BLACK으로 바꿔 이후 케이스(2~4)로 변환
 *  - [Case 2] w가 BLACK이고, w의 양 자식이 BLACK → w를 RED로 승급시키고, x를 부모로 올려 반복
 *  - [Case 3] w가 BLACK이고, w의 바깥쪽 자식이 BLACK, 안쪽 자식이 RED → w를 RED, 안쪽 자식을 BLACK으로
 *    바꾼 후 w 기준의 회전으로 [Case 4] 형태로 변환
 *  - [Case 4] w가 BLACK이고, w의 바깥쪽 자식이 RED → w의 색을 부모 색으로, 부모/바깥쪽 자식을 BLACK으로,
 *    부모 기준 회전 후 루프 종료(x를 root로 설정)
 * 주의: nil의 색은 BLACK이므로, w 혹은 w의 자식이 nil이어도 color 비교가 유효
 */
void rbtree_delete_fixup(rbtree *t, node_t *x) {
  
  while (x != t->root && x->color == RBTREE_BLACK) {
    
    if (x == x->parent->left){                    // x가 왼쪽 자식인 경우(오른쪽 대칭은 else 블록)
      node_t *w = x->parent->right;               // 형제 w는 오른쪽

      if (w->color == RBTREE_RED){                // [Case 1] 형제 RED → 회전으로 BLACK 형제 케이스로 변환
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        left_rotation(t, x->parent);
        w = x->parent->right;                     // 회전 후 새 형제 재설정
      }

      if (w->left->color == RBTREE_BLACK && 
          w->right->color == RBTREE_BLACK) {      // [Case 2] 형제 BLACK + 양쪽 자식 BLACK
        w->color = RBTREE_RED;                    // 형제를 RED로 올려 보내고
        x = x->parent;                            // 추가 흑색을 부모로 이동시켜 반복
      }
      else {                                      // [Case 3/4]
        if (w->right->color == RBTREE_BLACK) {    // [Case 3] 바깥쪽(오른쪽) BLACK, 안쪽(왼쪽) RED
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          right_rotation(t, w);                   // 회전으로 Case 4 형태로 변환
          w = x->parent->right;                   // 새 형제 재설정
        }

        w->color = x->parent->color;              // [Case 4] 바깥쪽(오른쪽) RED
        x->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_BLACK;
        left_rotation(t, x->parent);              // 부모 기준 좌회전으로 최종 복구
        x = t->root;                              // 루프 종료를 위한 조건 설정
      }
    }
    else {                                        // x가 오른쪽 자식인 경우(대칭 처리)
      node_t *w = x->parent->left;

      if (w->color == RBTREE_RED){                // [Case 1 대칭]
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        right_rotation(t, x->parent);
        w = x->parent->left;
      }

      if (w->right->color == RBTREE_BLACK && 
          w->left->color == RBTREE_BLACK) {       // [Case 2 대칭]
        w->color = RBTREE_RED;
        x = x->parent;
      }
      else 
      {
        if (w->left->color == RBTREE_BLACK) {     // [Case 3 대칭] 바깥쪽(왼쪽) BLACK, 안쪽(오른쪽) RED
          w->right->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          left_rotation(t, w);
          w = x->parent->left;
        }

        w->color = x->parent->color;              // [Case 4 대칭]
        x->parent->color = RBTREE_BLACK;
        w->left->color = RBTREE_BLACK;
        right_rotation(t, x->parent);
        x = t->root;
      }
    }
  }
  x->color = RBTREE_BLACK;                        // 루프 탈출 후 x의 추가 흑색 제거(일반적으로 root 또는 단순 케이스)
}

/*
 * 함수 역할: 노드 p를 삭제(BST 삭제 + 필요 시 RB delete fixup)
 * 핵심 판단(세 가지 구조):
 *  1) p의 왼쪽이 nil → 오른쪽 자식으로 p를 대체
 *  2) p의 오른쪽이 nil → 왼쪽 자식으로 p를 대체
 *  3) 양쪽 자식 존재 → p의 후계자(y=오른쪽 서브트리의 최소)를 찾아 p 위치에 이식
 *     - (yOriginalColor): 삭제되는 실제 노드 색을 기록하여, BLACK 삭제 시 fixup 필요 판단
 *     - (y->parent == p): 후계자의 자식 x의 부모 조정이 간단한 특수 케이스
 *     - (else): y를 먼저 자기 자리에서 빼낸 후 p 자리로 이식하며 좌/우 링크, 부모, 색을 승계
 * 삭제 후 판단:
 *  - (yOriginalColor == BLACK): 흑색 균형이 깨졌으므로 rbtree_delete_fixup 호출
 */
int rbtree_erase(rbtree *t, node_t *p) {
  
  node_t *y;
  node_t *x;
  color_t yOriginalColor;

  y = p;                                            // 기본적으로 삭제 후보 y는 p
  yOriginalColor = y->color;                        // 삭제되는 노드의 원래 색 저장

  if (p->left == t->nil) {                          // [케이스 1] 왼쪽이 비었으면 오른쪽으로 대체
    x = p->right;
    rbtree_transplant(t, p, p->right);
  }
  else if (p->right == t->nil) {                    // [케이스 2] 오른쪽이 비었으면 왼쪽으로 대체
    x = p->left;
    rbtree_transplant(t, p, p->left);
  }
  else {                                            // [케이스 3] 양쪽 자식 모두 존재(후계자 사용)
    y = p->right;                                   // 오른쪽 서브트리로 가서
    while(y->left != t->nil){                       // 그 중 최소(y)를 찾음
      y = y->left;
    }
    
    yOriginalColor = y->color;                      // 실제로 제거되는 y의 색
    x = y->right;                                   // y의 대체 자식(없으면 nil)

    if (y->parent == p) {                           // 후계자의 부모가 p인 특수 케이스
      x->parent = y;                                // x의 부모를 y로(설령 x가 nil이어도 부모 설정 일관)
    } 
    else {                                          // 일반 케이스: y를 먼저 자기 자리에서 빼냄
      rbtree_transplant(t, y, y->right);            // y 대신 y->right를 올려두고
      y->right = p->right;                          // p의 오른쪽 서브트리를 y의 오른쪽으로
      y->right->parent = y;                         // 부모 갱신
    }

    rbtree_transplant(t, p, y);                     // p 자리로 y를 이식
    y->left = p->left;                              // p의 왼쪽 서브트리를 y의 왼쪽으로
    y->left->parent = y;                            // 부모 갱신
    y->color = p->color;                            // 색도 p의 색을 승계(트리 높이 유지 목적)
  }

  if (yOriginalColor == RBTREE_BLACK) {             // 흑색을 실제로 제거했을 때만 보수 필요
    rbtree_delete_fixup(t, x);
  }

  free(p);                                          // 원래 p 노드 메모리 해제(이식 후 더 이상 사용되지 않음)

  return 0;
}

/*
 * 함수 역할: (중위) 왼-루트-오 순서로 서브트리를 순회하며 최대 n개까지 키를 배열에 채움
 * 핵심 판단:
 *  - (curr == nil): 공백 서브트리면 즉시 반환
 *  - (*count < n): 배열 용량이 남아 있을 때만 기록
 *  - 좌/우 재귀 순서는 이진 탐색 트리의 정렬된 순서를 보장
 */
void subtree_to_array(const rbtree *t, node_t *curr, key_t *arr, size_t n, size_t *count) {
  
  if (curr == t->nil) {
    return;
  }
  
  subtree_to_array(t, curr->left, arr, n, count);      // 왼쪽 서브트리

  if (*count < n) {                                    // 용량 내에서만 저장
    arr[(*count)++] = curr->key;
  }
  else return;                                         // 용량 소진 시 조기 종료
  
  subtree_to_array(t, curr->right, arr, n, count);     // 오른쪽 서브트리
}

/*
 * 함수 역할: 트리 전체를 중위순회하여 배열로 내보냄(최대 n개)
 * 핵심 판단:
 *  - (root == nil): 빈 트리면 0 반환
 *  - 내부적으로 subtree_to_array가 정렬 순서로 채우며, 반환값은 성공(0)
 */
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  
  if (t->root == t->nil) {
    return 0;                                          // 빈 트리
  }

  size_t cnt = 0;
  
  subtree_to_array(t, t->root, arr, n, &cnt);          // 중위순회로 채움
  
  return 0;                                            // 성공
}
