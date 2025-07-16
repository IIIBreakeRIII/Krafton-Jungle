# FunctionVisualizer 클래스 임포트
from function_visualizer import FunctionVisualizer

# 시각화 객체 생트
visualizer = FunctionVisualizer()


# 피보나치 수열 함수에 visualizer 정의
@visualizer.visualize(param_names=["n"])
def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)


@visualizer.visualize(param_names=["a", "b", "c"])
def mod_pow(a, b, c):
    if b == 0:
        return 1
    half = mod_pow(a, b // 2, c)
    result = (half * half) % c
    if b % 2 == 1:
        result = (result * a) % c
    return result


if __name__ == "__main__":
    # 피보나치 함수 실행
    # result = fibonacci(5)
    # print(f"결과: {result}")

    # 그래프 저장
    # visualizer.render("fibonacci_calls")

    print(mod_pow(10, 11, 12))
    visualizer.render("mod_pow")
