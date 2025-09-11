### Sorting Ready List

> #### ready_list 에 스레드를 삽입할 때 priority가 높은 스레드가 앞부분에 위치하도록 정렬
> thread.c : thread_yield, thread_unblock

- 기존에는 list_push_back을 사용해서 FIFO 방식으로 삽입
- list_insert_ordered를 활용해서 진행
- 정렬에 활용할 cmp_thread_priority() 함수는 아래에서 새로 선언

> cmp_thread_priority

- ready_list에 우선순위가 높은 스레드가 앞부분에 위치하도록 정렬할 때 사용할 정렬 함수를 새로 선언

### Preempt

> #### ready_list에 스레드가 삽입되고 나면, 현재 실행 중인 스레드와 ready_list에 있는 스레드의 priority를 비교
> #### priority가 더 높은 스레드가 ready_list에 있다면 즉시 CPU를 양보 (preempt_priority()호출)
> thread.c : thread_create, thread_wakeup

- 양보 여부를 확인하고 양보하는 preempt_priority() 함수는 아래에서 새로 선언

> thread_set_priority

- 실행 중인 thread의 priority를 변경되므로 ready_list에 있는 스레드보다 priority와 비교하여 현재 변경된 priority가 더 낮다면, 즉시 CPU를 양보(preempt_priority() 호출)

> preempt_priority(void)

- ready_list에 있는 스레드의 priority가 현재 실행중인 스레드의 priority보다 높으면 양보하는 함수를 새로 선언
    - ready_list는 priority를 기준으로 내림차순 정렬되므로, 가장 앞에 있는 스레드만 검사

### Lock Waiters
> #### lock, semaphore, condition variable을 기다리며 대기하고 있는 스레드가 여러개인 경우, lock을 획득할 수 있어졌을 때 priority가 가장 높은 스레드가 먼저 깨야함

- lock의 대기자 목록인 waiters도 priority 기준으로 내림차순 정렬로 변경

> #### semaphore

- 세마포어는 양의 정수값과 두 개의 연산자(P & V)로 구성된 동기화 기법
- "Down" or "P"
    - 세마포어를 획득하기 위해(공유 자원에 접근하려 할 때) 호출
    - 획득하면 세마포어를 획득했다는 의미로 value를 1 감소
    - 세마포어를 획득할 때까지(Value가 양수가 될 때까지) 기다린다. (Block 상태)
    - 세마포어를 바로 획득할 수 없을 때는 스레드가 기다리게 되는데
        - 기다리는 동안 다른 스레드가 세마포어를 먼저 해제(P)하고 나서, 이를 기다리는 스레드가 실행
- "Up" or "V"
    - 세마포어를 반환할 때(공유 자원 사용을 완료했을 때) 호출
    - 대기 중인 스레드 중 하나를 깨워주고, 세마포어의 value를 반환했다는 의미로 1 증가

> #### sema_down, cond_wait

- waiters와 스레드를 삽입할 때 priority가 높은 스레드가 앞부분에 위치하도록 정렬
    - list_insert_ordered를 활용
    - sema -> waiters를 정렬 할 때 사용하는 cmp_thread_priority 함수는 ready_list를 정렬할 때 사용한 함수를 그대로 사용
    - cond -> waiters를 정렬할 때 사용하는 cmp_sema_priority 함수는 아래에서 새로 선언

> #### cmp_sema_priority

- cond -> waiters를 정렬할 때 사용할 함수를 새로 선언
    - 인자로 전달되는 elem으로 바로 스레드에 접근할 수 없기 때문에, 이전의 cmp_thread_priority를 쓸 수 없어서 새로 선언해야함
    - 두 세마포어 안의 'waiters'중 제일 높은 priority를 비교해서 높으면 true를 반환하는 함수

> #### sema_up, cond_signal

- waiters에서 스레드를 깨우기 전에 waiters 목록을 다시 한 번 정렬
    - waiters에 들어있는 스레드가 donate를 받아 우선순위가 달라졌을 수 있기 때문
- sema_up에서는 unblock() 함수가 호출되면서 ready_list에 스레드가 삽입 -> 우선순위가 더 높은 스레드가 ready_list에 있다면 즉시 CPU를 양보
    - preempt_priority() 호출
