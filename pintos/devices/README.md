## Alarm Clock

### Reimplement `timer_sleep()`, defined in `devices/timer.c`

Although a working implementation is provided, it **busy waits**, that is, it spins in a loop checking the current time and calling `thread_yield()` until enough time has gone by. Reimplement it to avoid busy waiting.

<hr>

```c
void timer_sleep(int64_t ticks);
```

> Suspends execution of the calling thread until time has advanced by at least x timer ticks. Unless the system is otherwise idle, the thread need not wake up after exactly x ticks. Just put it on the ready queue after they have waited for the right amount of time.

`timer_sleep()` is useful for threads that operate in real-time, e.g. for blinking the cursor once per second. The argument to `timer_sleep()` is expressed in timer ticks, not in milliseconds or any another unit. There are `TIMER_FREQ` timer ticks per second, where `TIMER_FREQ` is a macro defined in `devices/timer.h`. The default value is 100. We don't recommend changing this value, because any change is likely to cause many of the tests to fail.

Separate functions `timer_msleep()`, `timer_usleep()`, and `timer_nsleep()` do exist for sleeping a specific number of milliseconds, microseconds, or nanoseconds, respectively, but these will call `timer_sleep()` automatically when necessary. You do not need to modify them. The alarm clock implementation is not needed for later projects, although it could be useful for project 4.

---

# 알람 시계 (Alarm Clock)

`timer_sleep()` 재구현하기

### 개요
`devices/timer.c`에 정의된 `timer_sleep()` 함수는 현재 **busy waiting** 방식으로 구현되어 있습니다.  
즉, **현재 시간을 계속 확인**하며 `thread_yield()`를 반복 호출하면서, 원하는 시간이 지날 때까지 CPU를 계속 점유합니다.  

이 과제의 목표는 **busy waiting 없이** 효율적으로 작동하도록 `timer_sleep()`을 **재구현**하는 것입니다.

---

## 함수 원형
```c
void timer_sleep(int64_t ticks);
```

> 호출한 스레드의 실행을, 최소한 x 타이머 틱(timer ticks) 만큼 시간이 흐를 때까지 일시 중단합니다.
> 시스템이 완전히 유휴 상태가 아닌 한, 정확히 x 틱 후에 스레드가 깨어날 필요는 없습니다.
> 단지 충분한 시간이 지난 후, 해당 스레드를 다시 ready queue에 넣어주면 됩니다.

`timer_sleep()` 함수는 실시간(real-time)으로 동작하는 스레드에서 유용합니다. 예를 들어, 1초마다 커서를 깜박이게 하는 경우에 사용됩니다.

이 함수의 인자는 밀리초(milliseconds)나 다른 단위가 아닌 타이머 틱(timer ticks) 단위입니다.
타이머는 1초에 `TIMER_FREQ`번 틱이 발생하며, `TIMER_FREQ`는 `devices/timer.h`에 매크로로 정의되어 있습니다.
기본 값은 100이며, 이 값을 변경하지 않는 것을 권장합니다. 값을 변경하면 여러 테스트가 실패할 가능성이 높습니다.

별도의 함수들인 `timer_msleep()`, `timer_usleep()`, `timer_nsleep()`는 각각 밀리초, 마이크로초, 나노초 단위로 대기(sleep)할 수 있도록 제공되지만, 내부적으로 필요시 자동으로 `timer_sleep()`을 호출합니다.
따라서 이 함수들을 수정할 필요는 없습니다.

알람 시계(timer_sleep) 구현은 이후 프로젝트에서 반드시 필요하지는 않지만, 프로젝트 4에서는 유용하게 사용될 수 있습니다.