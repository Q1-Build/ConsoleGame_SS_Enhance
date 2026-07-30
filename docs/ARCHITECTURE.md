# SS_Enhance 아키텍처

이 문서는 코드 구조, 계층별 책임, 의존성 방향과 장면 수명 주기를 정의한다.

## 현재 구조

제품 소스는 `SS-Enhance`, 자동 테스트 소스는 `SS-Enhance.Tests` 아래에 둔다.
헤더는 `.h`, 구현은 `.cpp`를 사용하며
한 파일에는 하나의 핵심 클래스만 둔다. 실제 필요가 생기기 전에 빈 디렉터리나 빈 클래스를 만들지 않는다.

```text
SS-Enhance/
├─ Main.cpp
├─ Assets/
│  └─ Audio/
│     ├─ Bgm/
│     └─ Sfx/
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
│  ├─ AudioCue.h
│  ├─ IAudio.h
│  ├─ InputKey.h
│  ├─ IInput.h
│  ├─ Windows/
│  │  ├─ WindowsAudio.h
│  │  └─ WindowsAudio.cpp
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
│  ├─ ScreenBuffer.cpp
│  ├─ ScreenViewport.h
│  └─ ScreenViewport.cpp
└─ Game/
   ├─ Domain/
   │  ├─ BossBattle.h
   │  ├─ BattleRules.h
   │  ├─ BattleRules.cpp
   │  ├─ BattleSettlement.h
   │  ├─ BattleSettlement.cpp
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
      ├─ InputOverlay.h
      ├─ InputOverlay.cpp
      ├─ ChatOverlay.h
      ├─ ChatOverlay.cpp
      ├─ LocalizedText.h
      ├─ LocalizedText.cpp
      ├─ SettingsScene.h
      ├─ SettingsScene.cpp
      ├─ BattleScene.h
      ├─ BattleScene.cpp
      ├─ BossRenderer.h
      ├─ BossRenderer.cpp
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

SS-Enhance.Tests/
├─ GameTests.cpp
└─ SS-Enhance.Tests.vcxproj
```

## 구성 요소와 책임

| 계층 | 구성 요소 | 책임 |
| --- | --- | --- |
| Application | `GameApplication` | 프레임 루프, 공유 서비스 수명, 장면 생성과 교체 |
| Core | `GameConstants` | 화면 크기와 프레임 시간 등 공통 컴파일 타임 상수 |
| Core | `Language` | 화면 표시 언어 값 |
| Core | `IRandomProvider`, `RandomProvider` | 난수 계약과 메르센 트위스터 구현 |
| Assets | `Audio/Bgm`, `Audio/Sfx` | 실행 파일과 함께 배포하는 배경음악·효과음 WAV 자산 |
| Platform | `MusicTrack`, `SoundEffect`, `IAudio` | 플랫폼과 파일 이름에 독립적인 BGM·효과음 식별 값, 재생과 마스터 볼륨 계약 |
| Rendering | `Color`, `Cell` | ANSI 색상과 전각 연속 칸을 포함한 화면 셀 값 |
| Rendering | `IScreen`, `ScreenBuffer`, `ScreenViewport` | 그리기 계약, 전각 문자 폭 처리, 전체 버퍼와 장면 영역 격리 |
| Rendering | `IFramePresenter` | 완성된 프레임 출력 계약 |
| Platform | `InputKey`, `IInput` | 운영체제와 독립적인 게임 키·완성 문자 입력 값과 계약 |
| Platform/Console | `ConsoleSession` | Windows 콘솔 모드·창 제목 초기화와 RAII 복원 |
| Platform/Console | `ConsoleInput` | Windows 키 상태와 IME 완성 문자를 게임 입력으로 변환 |
| Platform/Console | `ConsolePresenter` | ANSI 프레임을 Windows 콘솔에 출력 |
| Platform/Windows | `WindowsAudio` | WinMM 기반 반복 BGM, 비동기 효과음과 음원 파일 경로 관리 |
| Game/Domain | `Sword` | 검의 강화 단계와 등급 관리 |
| Game/Domain | `Difficulty`, `DifficultyTuning` | 난이도 값과 비용·냉각·제한 시간·실패 페널티 시작 단계 튜닝 |
| Game/Domain | `BossBattle`, `BattleRules` | 데이터 기반 공격 시퀀스와 공격·방어·반격 피해 및 선택 보상 규칙 |
| Game/Domain | `BattleSession` | 체력, 공격 시퀀스, 정직·교란·가짜 예고와 방어 상태 |
| Game/Domain | `BattleSettlement` | 승리 보상 또는 패배·후퇴 페널티의 일회성 반영 |
| Game/Domain | `ProgressionRules` | 강화 구간 상한, 보스 해금과 엔딩 조건 |
| Game/Domain | `PlayerProgress` | 검, 재화, 기억 조각, 강화 기록 관리 |
| Game/Domain | `ForgeRules` | 강화 비용, 확률, 성공과 실패 결과 판정 |
| Game/Domain | `ForgeSession` | 온도, 리듬, 제한 시간, 타격 점수 관리 |
| Game/Domain | `ForgeOutcome` | 한 번의 강화 판정 결과 전달 |
| Game/Effects | `Particle`, `ParticleSystem` | 초당 생성률 기반 파티클 방출, 갱신, 제거와 그리기 |
| Game/Scenes | `IScene`, `SceneTransition` | 장면 수명 주기와 전환 요청 계약 |
| Game/Scenes | `SceneContext` | 공유 진행·설정, 장면 간 임시 결과와 서비스의 비소유 참조 |
| Game/Scenes | `GameHudRenderer` | 공통 배경, HUD와 검 형상 렌더링 |
| Game/Scenes | `InputOverlay` | 공통 조작 안내와 입력 강조 가상 키보드 |
| Game/Scenes | `ChatOverlay` | 전역 채팅 메시지 편집, 표시 줄 보관과 우측 하단 채팅 렌더링 |
| Game/Scenes | `LocalizedText` | 현재 언어에 맞는 UI 문구와 검 이름 선택 |
| Game/Scenes | `TitleScene` | 한국어 기본 제목 연출과 설정 화면 진입 |
| Game/Scenes | `SettingsScene` | 표시 언어, 마스터 볼륨과 게임 난이도 선택 |
| Game/Scenes | `BattleScene` | 보스 전투 입력, 예고·흔들림 연출, 승패와 보상 반영 |
| Game/Scenes | `BossRenderer` | 보스별 아스키 형상, 대표 색상과 등장·격파 연출 |
| Game/Scenes | `EndingScene` | 최종 기록과 완성된 검 표시 |
| Game/Scenes | `ForgeScene` | 검 상태 표시, 비용 확인과 제련 시작 |
| Game/Scenes | `ForgingScene` | 실시간 입력 해석, 제련 진행과 판정 요청 |
| Game/Scenes | `ResultScene` | 강화 결과와 단계 변화 연출 |
| Composition Root | `main` | 플랫폼 구현 생성, 의존성 주입과 실행 |
| Tests | `GameTests` | 전투·강화·진행 규칙과 채팅 줄 보관·렌더링 경계 검증 |

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

### `Platform/Windows`

- 콘솔 출력과 무관한 Windows 전용 서비스 구현을 격리한다.
- `WindowsAudio`는 BGM과 효과음에 별도 MCI 별칭을 사용해 두 종류의 수명과 재생 경로를 분리한다.
- 두 채널에는 같은 마스터 볼륨을 즉시 적용하며 새 효과음 요청은 현재 효과음만 교체한다.
- 실행 파일 아래 `Assets/Audio/Bgm`과 `Assets/Audio/Sfx`에서 음원을 찾는다.
- 음원 누락이나 장치 재생 실패는 게임 규칙과 장면 전환을 막지 않고 무음으로 복구한다.

### `Rendering`

- 화면 버퍼와 게임 의미를 모르는 그리기 기본 기능만 제공한다.
- 골드, 강화 단계, 성공 확률 같은 게임 의미를 판단하지 않는다.
- 좌표 경계를 검사하고 화면 밖 쓰기를 안전하게 무시하거나 명확히 처리한다.
- 한글과 CJK 전각 문자의 실제 콘솔 표시 폭을 셀 점유와 정렬 계산에 반영한다.
- `ScreenViewport`는 기존 장면을 104×34 영역에 격리해 보조 UI 확장이 장면 좌표에 영향을 주지 않게 한다.

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
- 장면은 `IAudio`, `MusicTrack`, `SoundEffect`만 사용하며 Windows API, 음원 경로와 파일 형식을 알지 못한다.
- 현지화된 문자열과 검 이름은 화면 표현 책임으로 유지하고 Domain에 넣지 않는다.
- `InputOverlay`는 모든 장면 뒤에 그려지며 입력 규칙을 변경하지 않고 공통 조작 피드백만 제공한다.
- `ChatOverlay`는 모든 장면 위에 유지되며 편집 중 게임 입력과 시간 갱신을 차단한다.

## 의존성 방향

```text
Main
  → Application
      → Scenes
          → Domain
          → Effects
          → Rendering abstraction
          → Input abstraction
          → Audio abstraction
Platform/Console
  → Rendering/Input abstraction의 구현
Platform/Windows
  → Audio abstraction의 구현
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

전체 화면 버퍼, 장면 뷰포트와 실제 콘솔 출력은 각각
`ScreenBuffer`, `ScreenViewport`, `ConsolePresenter`가 담당한다.
