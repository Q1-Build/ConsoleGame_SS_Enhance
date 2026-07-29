# SS_Enhance 아키텍처

이 문서는 코드 구조, 계층별 책임, 의존성 방향과 장면 수명 주기를 정의한다.

## 현재 구조

모든 소스는 `SS-Enhance` 아래에 둔다. 헤더는 `.h`, 구현은 `.cpp`를 사용하며
한 파일에는 하나의 핵심 클래스만 둔다. 실제 필요가 생기기 전에 빈 디렉터리나 빈 클래스를 만들지 않는다.

```text
SS-Enhance/
├─ Main.cpp
├─ Application/
│  ├─ GameApplication.h
│  └─ GameApplication.cpp
├─ Core/
│  ├─ GameConstants.h
│  ├─ Language.h
│  ├─ IRandomProvider.h
│  ├─ RandomProvider.h
│  └─ RandomProvider.cpp
├─ Platform/
│  ├─ InputKey.h
│  ├─ IInput.h
│  └─ Console/
│     ├─ ConsoleSession.h
│     ├─ ConsoleSession.cpp
│     ├─ ConsoleInput.h
│     ├─ ConsoleInput.cpp
│     ├─ ConsolePresenter.h
│     └─ ConsolePresenter.cpp
├─ Rendering/
│  ├─ Color.h
│  ├─ Cell.h
│  ├─ IScreen.h
│  ├─ IFramePresenter.h
│  ├─ ScreenBuffer.h
│  └─ ScreenBuffer.cpp
└─ Game/
   ├─ Domain/
   │  ├─ BossBattle.h
   │  ├─ BattleRules.h
   │  ├─ BattleRules.cpp
   │  ├─ BattleSession.h
   │  ├─ BattleSession.cpp
   │  ├─ Difficulty.h
   │  ├─ Sword.h
   │  ├─ Sword.cpp
   │  ├─ PlayerProgress.h
   │  ├─ PlayerProgress.cpp
   │  ├─ ProgressionRules.h
   │  ├─ ProgressionRules.cpp
   │  ├─ ForgeOutcome.h
   │  ├─ ForgeRules.h
   │  ├─ ForgeRules.cpp
   │  ├─ ForgeSession.h
   │  └─ ForgeSession.cpp
   ├─ Effects/
   │  ├─ Particle.h
   │  ├─ ParticleSystem.h
   │  └─ ParticleSystem.cpp
   └─ Scenes/
      ├─ IScene.h
      ├─ SceneTransition.h
      ├─ SceneContext.h
      ├─ GameHudRenderer.h
      ├─ GameHudRenderer.cpp
      ├─ LocalizedText.h
      ├─ LocalizedText.cpp
      ├─ SettingsScene.h
      ├─ SettingsScene.cpp
      ├─ BattleScene.h
      ├─ BattleScene.cpp
      ├─ EndingScene.h
      ├─ EndingScene.cpp
      ├─ TitleScene.h
      ├─ TitleScene.cpp
      ├─ ForgeScene.h
      ├─ ForgeScene.cpp
      ├─ ForgingScene.h
      ├─ ForgingScene.cpp
      ├─ ResultScene.h
      └─ ResultScene.cpp
```

## 구성 요소와 책임

| 계층 | 구성 요소 | 책임 |
| --- | --- | --- |
| Application | `GameApplication` | 프레임 루프, 공유 서비스 수명, 장면 생성과 교체 |
| Core | `GameConstants` | 화면 크기와 프레임 시간 등 공통 컴파일 타임 상수 |
| Core | `Language` | 화면 표시 언어 값 |
| Core | `IRandomProvider`, `RandomProvider` | 난수 계약과 메르센 트위스터 구현 |
| Rendering | `Color`, `Cell` | ANSI 색상과 전각 연속 칸을 포함한 화면 셀 값 |
| Rendering | `IScreen`, `ScreenBuffer` | 그리기 계약, 전각 문자 폭 처리, 메모리 화면 버퍼 |
| Rendering | `IFramePresenter` | 완성된 프레임 출력 계약 |
| Platform | `InputKey`, `IInput` | 운영체제와 독립적인 게임 입력 값과 계약 |
| Platform/Console | `ConsoleSession` | Windows 콘솔 초기화와 RAII 복원 |
| Platform/Console | `ConsoleInput` | Windows 키 상태를 게임 입력으로 변환 |
| Platform/Console | `ConsolePresenter` | ANSI 프레임을 Windows 콘솔에 출력 |
| Game/Domain | `Sword` | 검의 강화 단계와 등급 관리 |
| Game/Domain | `Difficulty`, `DifficultyTuning` | 난이도 값과 비용·냉각·제한 시간 튜닝 |
| Game/Domain | `BossBattle`, `BattleRules` | 보스 패턴 값과 공격·방어·반격 피해 및 보상 규칙 |
| Game/Domain | `BattleSession` | 체력, 공격 예고, 방어, 연속 공격과 타이밍 상태 |
| Game/Domain | `ProgressionRules` | 강화 구간 상한, 보스 해금과 엔딩 조건 |
| Game/Domain | `PlayerProgress` | 검, 재화, 기억 조각, 강화 기록 관리 |
| Game/Domain | `ForgeRules` | 강화 비용, 확률, 성공과 실패 결과 판정 |
| Game/Domain | `ForgeSession` | 온도, 리듬, 제한 시간, 타격 점수 관리 |
| Game/Domain | `ForgeOutcome` | 한 번의 강화 판정 결과 전달 |
| Game/Effects | `Particle`, `ParticleSystem` | 초당 생성률 기반 파티클 방출, 갱신, 제거와 그리기 |
| Game/Scenes | `IScene`, `SceneTransition` | 장면 수명 주기와 전환 요청 계약 |
| Game/Scenes | `SceneContext` | 공유 진행·설정, 장면 간 임시 결과와 서비스의 비소유 참조 |
| Game/Scenes | `GameHudRenderer` | 공통 배경, HUD와 검 형상 렌더링 |
| Game/Scenes | `LocalizedText` | 현재 언어에 맞는 UI 문구와 검 이름 선택 |
| Game/Scenes | `TitleScene` | 한국어 기본 제목 연출과 설정 화면 진입 |
| Game/Scenes | `SettingsScene` | 표시 언어와 게임 난이도 선택 |
| Game/Scenes | `BattleScene` | 보스 전투 입력, 예고·흔들림 연출, 승패와 보상 반영 |
| Game/Scenes | `EndingScene` | 최종 기록과 완성된 검 표시 |
| Game/Scenes | `ForgeScene` | 검 상태 표시, 비용 확인과 제련 시작 |
| Game/Scenes | `ForgingScene` | 실시간 입력 해석, 제련 진행과 판정 요청 |
| Game/Scenes | `ResultScene` | 강화 결과와 단계 변화 연출 |
| Composition Root | `main` | 플랫폼 구현 생성, 의존성 주입과 실행 |

## 계층별 책임

### `Main.cpp`

- 프로그램의 Composition Root만 담당한다.
- 객체를 생성하고 의존성을 연결한 뒤 `GameApplication::Run()`을 호출한다.
- 게임 규칙, 렌더링, 입력 처리와 장면 로직을 작성하지 않는다.
- 특별한 이유가 없다면 30줄 이내로 유지한다.

### `Application`

- 프레임 루프와 애플리케이션 수명 주기를 관리한다.
- 현재 장면을 갱신하고 `GameApplication::CreateScene`에서 다음 장면을 만든다.
- 구체적인 강화 확률이나 화면 도형을 직접 계산하지 않는다.

### `Core`

- 특정 장면이나 Windows API에 의존하지 않는 공통 값과 작은 계약을 둔다.
- 상수는 의미 있는 이름과 관련 범위 안에 둔다.
- 무관한 기능을 계속 모으는 `Utils` 만능 클래스를 만들지 않는다.

### `Platform/Console`

- `Windows.h`, 콘솔 모드, 키 입력 등 운영체제 의존 코드를 격리한다.
- 게임 규칙에 Windows 가상 키 코드나 콘솔 핸들을 노출하지 않는다.
- 콘솔 상태를 변경하면 정상 종료와 예외 상황 모두에서 RAII로 복구한다.

### `Rendering`

- 화면 버퍼와 게임 의미를 모르는 그리기 기본 기능만 제공한다.
- 골드, 강화 단계, 성공 확률 같은 게임 의미를 판단하지 않는다.
- 좌표 경계를 검사하고 화면 밖 쓰기를 안전하게 무시하거나 명확히 처리한다.
- 한글과 CJK 전각 문자의 실제 콘솔 표시 폭을 셀 점유와 정렬 계산에 반영한다.

### `Game/Domain`

- 검, 플레이어 진행도, 비용, 확률과 강화 결과 등 순수 게임 규칙을 담당한다.
- 렌더링, 키보드, 프레임 출력과 Windows API에 의존하지 않는다.
- 같은 입력과 난수 결과에는 같은 판정을 반환한다.
- 확률 판정에 필요한 난수는 외부에서 전달받는다.

### `Game/Effects`

- 파티클의 생성, 갱신, 제거와 표현을 담당한다.
- 강화 성공 여부를 결정하지 않고 장면이 전달한 효과 요청만 표현한다.

### `Game/Scenes`

- 한 장면의 입력 해석, 상태 갱신과 화면 구성을 담당한다.
- 장면끼리 서로의 구체 클래스를 생성하거나 소유하지 않는다.
- 공통 데이터와 서비스는 `SceneContext`로 필요한 최소 범위만 전달한다.
- 현지화된 문자열과 검 이름은 화면 표현 책임으로 유지하고 Domain에 넣지 않는다.

## 의존성 방향

```text
Main
  → Application
      → Scenes
          → Domain
          → Effects
          → Rendering abstraction
          → Input abstraction
Platform/Console
  → Rendering/Input abstraction의 구현
```

- `Domain`은 가장 안쪽 계층이다.
- 장면에서 저수준 콘솔 구현을 직접 생성하지 않는다.
- 헤더 순환 참조를 금지하고 가능한 경우 전방 선언한다.
- 소유하지 않는 필수 객체는 참조로 전달한다.
- 단독 소유권은 `std::unique_ptr`로 표현하고 공유 소유권은 꼭 필요한 경우에만 사용한다.
- 전역 가변 상태와 싱글턴을 사용하지 않는다.

## 장면 수명 주기

모든 장면은 다음 계약을 유지한다.

```cpp
class IScene
{
public:
    virtual ~IScene() = default;
    virtual void OnEnter() = 0;
    [[nodiscard]] virtual SceneTransition Update(float deltaSeconds) = 0;
    virtual void Render(IScreen& screen) const = 0;
    virtual void OnExit() = 0;
};
```

- `OnEnter`: 장면 진입 상태 초기화
- `Update`: 입력 해석과 상태 갱신 후 전환 요청 반환
- `Render`: 현재 논리 상태를 그리며 게임 상태를 변경하지 않음
- `OnExit`: 장면 종료 정리

화면 버퍼 생성과 실제 콘솔 출력은 각각 `ScreenBuffer`와 `ConsolePresenter`가 담당한다.
