import random
from itertools import combinations

# 기존 0~3주차 배정 결과를 initial_schedule에 입력하세요.
# 각 라운드별로 팀을 튜플 리스트로 정의합니다. 2인 팀은 2원, 3인 팀은 3원 튜플 사용.
initial_schedule = [
    # 0주차
    [
        ("김형진","김시연","김정은"),
        ("이요한","장윤호","김은비"),
        ("김현수","정성원","손형호"),
        ("장아연","전세영","서정"),
        ("박준성","백준렬","강경찬"),
        ("이시영","박중섭","이정호"),
        ("조예은","신정호"),
        ("김형욱","박시현","조계진"),
        ("전석모","안준","진주영"),
        ("류현소","표후동","서찬진"),
        ("박도현","김도훈","이승헌"),
        ("지경희","유성수","박종민"),
        ("이승준","정지훈","김형일"),
        ("정성훈","진충열","진솔"),
    ],
    # 1주차
    [
        ("박준성","전세영","이승헌"),
        ("김형일","진솔","김시연"),
        ("유성수","김형욱","진주영"),
        ("류현소","박중섭","김도훈"),
        ("안준","지경희","김형진"),
        ("조계진","정성원","박종민"),
        ("정성훈","서찬진"),
        ("박도현","김정은","장아연"),
        ("이시영","김현수","이요한"),
        ("진충열","전석모","강경찬"),
        ("백준렬","장윤호","이승준"),
        ("표후동","김은비","조예은"),
        ("이정호","박시현","서정"),
        ("손형호","신정호","정지훈"),
    ],
    # 2주차
    [
        ("조예은","강경찬","김형욱"),
        ("정지훈","진주영","진충열"),
        ("신정호","김현수","장아연"),
        ("정성원","안준","이정호"),
        ("표후동","정성훈","전세영"),
        ("이승준","이시영","서정"),
        ("김은비","박도현"),
        ("전석모","이승헌","박종민"),
        ("김형진","조계진","유성수"),
        ("지경희","백준렬","서찬진"),
        ("박중섭","장윤호","김시연"),
        ("손형호","진솔","이요한"),
        ("박준성","류현소","박시현"),
        ("김정은","김형일","김도훈"),
    ],
    # 3주차
    [
        ("장아연","이승준","박중섭"),
        ("김은비","강경찬","서찬진"),
        ("박도현","백준렬","김시연"),
        ("김현수","진주영","서정"),
        ("전세영","김도훈","김형진"),
        ("전석모","진솔","정성원"),
        ("유성수","이요한"),
        ("이승헌","조예은","정성훈"),
        ("정지훈","박시현","지경희"),
        ("박종민","표후동","박준성"),
        ("김형일","손형호","이정호"),
        ("장윤호","안준","김형욱"),
        ("신정호","진충열","조계진"),
        ("이시영","류현소","김정은"),
    ],
]

# 플레이어 명단 자동 생성 (중복 없이)
players = sorted({p for rnd in initial_schedule for grp in rnd for p in grp})

# 초기 previous_pairs 설정
previous_pairs = set()
for rnd in initial_schedule:
    for grp in rnd:
        for a, b in combinations(grp, 2):
            previous_pairs.add(frozenset((a, b)))


def valid_group(group):
    """
    그룹 내 모든 페어가 이전에 만나지 않았는지 확인
    """
    for a, b in combinations(group, 2):
        if frozenset((a, b)) in previous_pairs:
            return False
    return True


def backtrack_grouping(remaining):
    """
    남은 인원으로 그룹을 구성하는 백트래킹
    """
    if not remaining:
        return []

    if len(remaining) == 2:
        pair = tuple(remaining)
        if valid_group(pair):
            return [pair]
        return None

    first = remaining[0]
    for combo in combinations(remaining[1:], 2):
        group = (first, combo[0], combo[1])
        if not valid_group(group):
            continue
        next_remaining = [p for p in remaining if p not in group]
        result = backtrack_grouping(next_remaining)
        if result is not None:
            return [group] + result
    return None


def generate_schedule(total_rounds=13):
    """
    total_rounds까지 라운드를 생성하며, 기존 배정(initial_schedule) 이후 라운드도 이어서 탐색
    """
    schedule = [rnd.copy() for rnd in initial_schedule]

    for rnd_idx in range(len(initial_schedule) + 1, total_rounds + 1):
        pool = players.copy()
        random.shuffle(pool)

        groups = backtrack_grouping(pool)
        if groups is None:
            raise RuntimeError(f"라운드 {rnd_idx}에서 유효한 그룹을 찾지 못했습니다.")

        schedule.append(groups)
        for grp in groups:
            for a, b in combinations(grp, 2):
                previous_pairs.add(frozenset((a, b)))

    return schedule


if __name__ == "__main__":
    total_rounds = 13
    schedule = generate_schedule(total_rounds)
    for i, rnd in enumerate(schedule, start=1):
        print(f"=== {i}주차 팀 배정 ===")
        for tidx, grp in enumerate(rnd, start=1):
            print(f"팀 {tidx}: {', '.join(grp)}")
        print()

