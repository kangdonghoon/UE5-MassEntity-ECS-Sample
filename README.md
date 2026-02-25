# Mass Entity ECS 샘플 — 팀 기반 전투 시뮬레이션

Unreal Engine 5의 **Mass Entity Framework(ECS)** 를 활용한 팀 기반 전투 시뮬레이션 샘플 코드입니다.
수천 개의 엔티티가 이동, 전투, 사망 처리를 ECS 패턴으로 수행하며,
**Instanced Static Mesh(ISM)** 를 통해 효율적으로 렌더링됩니다.

환경: Unreal Engine 5 (Lyra Starter Game 기반)


## 파일 구조

```
MassECS/

[데이터 정의]
  MassEntityTestFragments.h          — Fragment, Tag, SharedFragment 정의
  MassEntityTestCombatTrait.h/.cpp   — Trait, 엔티티 템플릿 구성

[시스템 (Processor)]
  MassEntityTestMovementProcessor.h/.cpp   — 이동 + 회전
  MassEntityTestCombatProcessor.h/.cpp     — 타겟 탐색 + 데미지
  MassEntityTestHealthProcessor.h/.cpp     — 체력 재생
  MassEntityTestDeathObserver.h/.cpp       — 사망 감지 + 엔티티 파괴

[스포너 / 시각화]
  MassEntityTestSpawner.h/.cpp       — 엔티티 생성 + ISM 시각화
```


## ECS 아키텍처

### 핵심 개념

- **Fragment** — 데이터 컴포넌트 (구조체). HealthFragment, CombatFragment, TeamFragment, MoveFragment, TargetFragment
- **Tag** — 상태 마커 (메모리 0). FMassEntityTestDeadTag
- **Shared Fragment** — 아키타입 전체가 공유하는 데이터. FMassEntityTestVisualParams (메시, 머티리얼, 스케일)
- **Processor** — 로직 시스템 (매 프레임 실행). MovementProcessor, CombatProcessor, HealthRegenProcessor
- **Observer** — 이벤트 반응 시스템. DeathObserver (DeadTag 추가 시 트리거)
- **Trait** — 엔티티 템플릿 빌더. CombatTrait (Fragment 조합 정의)

### Fragment 구성

엔티티 하나의 데이터 구성:

```
[Per-Entity Fragments]
  FTransformFragment              — 위치, 회전, 스케일
  FMassEntityTestHealthFragment   — HP, 최대HP, 재생량
  FMassEntityTestCombatFragment   — 공격력, 사거리, 쿨타임
  FMassEntityTestTargetFragment   — 현재 공격 대상
  FMassEntityTestTeamFragment     — 팀 ID
  FMassEntityTestMoveFragment     — 이동 속도

[Shared Fragment] (아키타입 공유)
  FMassEntityTestVisualParams     — 메시, 머티리얼, 스케일

[Tag]
  FMassEntityTestDeadTag          — 사망 시 부여
```


## 실행 흐름

### 1. 초기화 단계

```
BeginPlay
  -> AMassEntityTestSpawner::SpawnUnits(Count)
      -> UMassSpawnerSubsystem::SpawnEntities()
           엔티티에 Trait에서 정의한 Fragment들이 자동 부착
      -> 각 엔티티에 위치(Transform)와 팀(TeamId) 설정
      -> SetupMeshFromEntity()
           첫 엔티티의 SharedFragment(VisualParams)에서
           메시, 머티리얼, 스케일을 읽어 ISM 컴포넌트에 설정
      -> AddInstanceForEntity() x N
           ISM 인스턴스 생성 + Entity<->Instance 매핑 구축
```

### 2. 매 프레임 처리 순서

```
Tick

  [Mass Entity Processor 실행 — 엔진 자동 호출]

  (1) MovementProcessor  (ExecuteBefore: CombatProcessor)
      - EnemyQuery: 살아있는 전체 유닛의 위치, 팀 수집
      - MoverQuery: 각 유닛마다
          - 가장 가까운 적 탐색
          - 적 방향으로 회전 (SetRotation)
          - 사거리 밖이면 이동 (SetLocation)

  (2) CombatProcessor  (ExecuteAfter: MovementProcessor)
      - CandidateQuery: 타겟 후보 수집
      - AttackerQuery: 각 공격자마다
          - 사거리 내 가장 가까운 적을 타겟으로 설정
          - 쿨타임 경과 시 타겟에 데미지 적용
          - 타겟 사망 시 -> Defer().AddTag<DeadTag>

  (3) HealthRegenProcessor  (ExecuteAfter: CombatProcessor)
      - 살아있는 엔티티의 HP를 초당 RegenPerSecond만큼 회복

  (4) DeathObserver  (DeadTag 추가 이벤트에 반응)
      - Transform 스케일을 Zero로 설정 (즉시 숨김)
      - Defer().DestroyEntity() 예약

  [Spawner Tick — 시각화 갱신]

  UpdateVisualization()
      - 유효하지 않은 엔티티의 ISM 인스턴스 제거
          RemoveInstanceForEntity() — swap & pop 방식
      - 살아있는 인스턴스의 Transform을 일괄 갱신
          BatchUpdateInstancesTransforms()
```

### 3. ISM 인스턴스 관리 구조

```
EntityToInstanceMap (TMap)           InstanceToEntityArray (TArray)
  Entity_A -> 0                       [0] Entity_A
  Entity_B -> 1                       [1] Entity_B
  Entity_C -> 2                       [2] Entity_C

삭제 시 (Entity_A 제거):
  1. RemoveInstance(0) -> ISM이 마지막 인스턴스를 0번으로 swap
  2. InstanceToEntityArray[0] = Entity_C  (마지막이던 C를 0번으로)
  3. EntityToInstanceMap[Entity_C] = 0     (C의 인덱스 갱신)
  4. Pop() + Remove(Entity_A)
```


## Processor 실행 순서 설정

```cpp
// MovementProcessor — CombatProcessor보다 먼저 실행
ExecutionOrder.ExecuteBefore.Add(TEXT("MassEntityTestCombatProcessor"));

// CombatProcessor — MovementProcessor 이후 실행
ExecutionOrder.ExecuteAfter.Add(TEXT("MassEntityTestMovementProcessor"));

// HealthRegenProcessor — CombatProcessor 이후 실행
ExecutionOrder.ExecuteAfter.Add(TEXT("MassEntityTestCombatProcessor"));

// DeathObserver — DeadTag 추가 이벤트에 자동 반응 (순서 지정 불필요)
```


## 에디터 설정 방법

### 1. Mass Entity Config Asset 생성

1. 콘텐츠 브라우저에서 우클릭 -> Miscellaneous -> Mass Entity Config
2. 생성된 Config Asset을 열고 Traits에 MassEntityTest Combat 추가
3. 각 Fragment의 기본값 설정 (HP, 공격력, 이동 속도, 메시, 머티리얼 등)

### 2. Spawner 배치

1. AMassEntityTestSpawner를 레벨에 배치 (팀별 1개씩, 최소 2개)
2. 각 Spawner의 프로퍼티 설정:
   - EntityConfig: 위에서 만든 Config Asset 할당
   - InitialCount: 스폰할 엔티티 수
   - TeamId: 팀 번호 (0, 1, 2, ...)
   - SpawnRadius: 스폰 반경

### 3. 필요 플러그인

Lyra.uproject에 다음 플러그인이 활성화되어 있어야 합니다:
- MassEntity
- MassGameplay
- MassSpawner


## 핵심 설계 포인트

- **Data-Oriented Design** — Fragment에 데이터만, Processor에 로직만 분리. 캐시 친화적 메모리 접근
- **ISM 캐싱** — 메시 인스턴스를 스폰 시 한 번 생성, 이후 Transform만 일괄 갱신 (BatchUpdate)
- **Swap & Pop 삭제** — 인스턴스 제거 시 마지막 요소와 교체 후 Pop. O(1) 삭제
- **Deferred 명령** — 태그 추가, 엔티티 파괴는 Context.Defer()로 안전하게 지연 처리
- **Observer 패턴** — DeadTag 부여 이벤트에 반응. 명시적 폴링 불필요
- **Shared Fragment** — 메시, 머티리얼 등 아키타입 공통 데이터는 한 번만 저장









![VideoProject-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/264ef8a1-d052-4a15-8768-e2f696a8e0ba)

