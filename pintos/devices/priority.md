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
