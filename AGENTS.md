# SS_Enhance 프로젝트 작업 지침

이 문서는 이 저장소에서 작업하는 모든 AI 에이전트와 개발자가 항상 따라야 하는 프로젝트 지침이다.
프로젝트 구조나 핵심 규칙이 변경되면 코드와 함께 이 문서도 반드시 갱신한다.

## 1. 프로젝트 목표

- 프로젝트명: `SS_Enhance`
- 플랫폼: Windows 콘솔
- 언어 및 표준: C++17
- 핵심 경험은 `강화 준비 → 실시간 제련 → 강화 판정 → 검의 진화 → 전투와 성장`이다.
- 화려한 연출만큼 읽기 쉬운 구조, 명확한 책임, 테스트 가능한 게임 규칙을 중요하게 다룬다.

## 2. 현재 구현 상태

첫 번째 플레이 가능 버전의 기능을 계층별 클래스로 분리한 상태다.
`Main.cpp`는 플랫폼 구현을 생성해 `GameApplication`에 주입하는 Composition Root만 담당한다.

### 현재 클래스와 역할

| 계층 | 구성 요소 | 현재 책임 |
| --- | --- | --- |
| Application | `GameApplication` | 프레임 루프, 공유 서비스 수명, 장면 생성과 교체 |
| Core | `GameConstants` | 화면 크기와 프레임 시간 등 공통 컴파일 타임 상수 |
| Core | `IRandomProvider`, `RandomProvider` | 난수 계약과 메르센 트위스터 구현 |
| Rendering | `Color`, `Cell` | ANSI 색상과 화면 셀 값 |
| Rendering | `IScreen`, `ScreenBuffer` | 장면 그리기 계약과 메모리 화면 버퍼 |
| Rendering | `IFramePresenter` | 완성된 프레임의 출력 계약 |
| Platform | `InputKey`, `IInput` | 운영체제와 독립적인 게임 입력 값과 계약 |
| Platform/Console | `ConsoleSession` | Windows 콘솔 초기화와 RAII 복원 |
| Platform/Console | `ConsoleInput` | Windows 키 상태를 게임 입력으로 변환 |
| Platform/Console | `ConsolePresenter` | ANSI 프레임을 Windows 콘솔에 출력 |
| Game/Domain | `Sword` | 검의 강화 단계, 등급, 이름 관리 |
| Game/Domain | `PlayerProgress` | 검, 재화, 기억 조각, 강화 기록 관리 |
| Game/Domain | `ForgeRules` | 강화 비용, 확률, 성공과 실패 결과 판정 |
| Game/Domain | `ForgeSession` | 온도, 리듬, 제한 시간, 타격 점수 관리 |
| Game/Domain | `ForgeOutcome` | 한 번의 강화 판정 결과 전달 |
| Game/Effects | `Particle`, `ParticleSystem` | 파티클 상태, 생성, 갱신, 제거, 그리기 |
| Game/Scenes | `IScene`, `SceneTransition` | 장면 수명 주기와 전환 요청 계약 |
| Game/Scenes | `SceneContext` | 장면이 공유하는 상태와 서비스의 비소유 참조 |
| Game/Scenes | `GameHudRenderer` | 공통 배경, HUD, 검 형상 렌더링 |
| Game/Scenes | `TitleScene` | 제목 연출과 게임 시작 |
| Game/Scenes | `ForgeScene` | 검 상태 표시, 비용 확인, 제련 시작 |
| Game/Scenes | `ForgingScene` | 실시간 입력 해석, 제련 진행, 강화 판정 요청 |
| Game/Scenes | `ResultScene` | 강화 결과와 단계 변화 연출 |
| Composition Root | `main` | 플랫폼 구현 생성, 의존성 주입, 애플리케이션 실행 |

### 현재 게임 흐름

```text
Title
  ↓ Enter
Forge
  ↓ 강화 비용 지불
Forging
  ↓ 3회 타격 또는 제한 시간 종료
Result
  ↓ Enter
Forge
```

### 현재 플레이 규칙

- `A` 또는 왼쪽 방향키: 검의 열기를 낮춘다.
- `D` 또는 오른쪽 방향키: 화력을 높인다.
- `Space` 또는 `Enter`: 망치로 검을 타격한다.
- 타이밍 정확도 68%, 온도 정확도 32%를 합산해 타격 점수를 계산한다.
- 세 번의 타격 평균이 최종 강화 확률에 영향을 준다.
- 완벽한 제련은 일정 확률로 두 단계 상승을 발생시킨다.
- 낮은 강화 단계의 실패는 단계를 유지한다.
- 높은 단계에서는 기억 조각이 먼저 소모되고, 조각이 없으면 한 단계 하락한다.

### 현재 구조의 핵심 경계

- `Main.cpp`에는 게임 규칙과 장면 로직이 없다.
- Windows API는 `Platform/Console` 구현에 격리되어 있다.
- `Game/Domain`은 입력, 렌더링, Windows API를 참조하지 않는다.
- 강화 판정은 외부에서 전달받은 난수 값으로 결정되어 재현과 테스트가 가능하다.
- 장면은 서로를 직접 생성하지 않고 `SceneTransition`만 반환한다.
- 공통 상태와 서비스는 `SceneContext`의 비소유 참조로 공유한다.
- 화면 버퍼 생성과 실제 콘솔 출력은 `ScreenBuffer`와 `ConsolePresenter`로 분리되어 있다.

## 3. 현재 코드 구조

모든 소스는 `ConsoleApplication1` 아래에서 다음 구조를 따른다.
헤더는 `.h`, 구현은 `.cpp`를 사용하며 한 파일에는 하나의 핵심 클래스만 둔다.

```text
ConsoleApplication1/
├─ Main.cpp
├─ Application/
│  ├─ GameApplication.h
│  └─ GameApplication.cpp
├─ Core/
│  ├─ GameConstants.h
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
   │  ├─ Sword.h
   │  ├─ Sword.cpp
   │  ├─ PlayerProgress.h
   │  ├─ PlayerProgress.cpp
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
      ├─ TitleScene.h
      ├─ TitleScene.cpp
      ├─ ForgeScene.h
      ├─ ForgeScene.cpp
      ├─ ForgingScene.h
      ├─ ForgingScene.cpp
      ├─ ResultScene.h
      └─ ResultScene.cpp
```

실제 필요가 생기기 전에는 빈 디렉터리나 빈 클래스를 미리 만들지 않는다.
새 시스템이 생기면 위 계층에 맞는 위치를 먼저 결정하고 구현한다.

## 4. 계층별 책임과 의존성

### `Main.cpp`

- 프로그램의 조립 지점(Composition Root)만 담당한다.
- 객체를 생성하고 의존성을 연결한 뒤 `GameApplication::Run()`을 호출한다.
- 게임 규칙, 렌더링, 입력 처리, 장면 로직을 작성하지 않는다.
- 특별한 이유가 없다면 30줄 이내로 유지한다.

### `Application`

- 프레임 루프와 애플리케이션 수명 주기를 관리한다.
- 현재 장면을 갱신하고 다음 장면으로 교체한다.
- 구체적인 강화 확률이나 화면 도형을 직접 계산하지 않는다.

### `Core`

- 특정 장면이나 Windows API에 의존하지 않는 공통 도구를 둔다.
- 상수는 의미 있는 이름을 사용하고 관련 범위 안에 둔다.
- 무관한 유틸리티를 한 클래스에 계속 추가하는 `Utils` 만능 클래스를 금지한다.

### `Platform/Console`

- `Windows.h`, 콘솔 모드, 키 입력 등 운영체제 의존 코드를 격리한다.
- 게임 규칙은 Windows 가상 키 코드나 콘솔 핸들을 알지 못해야 한다.
- 콘솔 상태를 변경했다면 RAII를 사용해 정상 종료와 예외 상황 모두에서 복구한다.

### `Rendering`

- 화면 버퍼와 그리기 기본 기능만 제공한다.
- 골드, 강화 단계, 성공 확률 같은 게임 의미를 직접 판단하지 않는다.
- 좌표 경계를 검사하고 화면 밖 쓰기를 안전하게 무시하거나 명확히 처리한다.

### `Game/Domain`

- 검, 플레이어 진행도, 비용, 확률, 강화 결과 등 순수 게임 규칙을 담당한다.
- 렌더링, 키보드, 프레임 출력, Windows API에 의존하지 않는다.
- 같은 입력과 난수 결과가 주어지면 같은 판정을 반환하도록 설계한다.
- 확률 판정에 필요한 난수는 외부에서 주입해 테스트 가능하게 만든다.

### `Game/Effects`

- 파티클의 생성, 갱신, 제거를 담당한다.
- 강화 성공 여부를 직접 결정하지 않고 장면이 전달한 효과 요청만 표현한다.

### `Game/Scenes`

- 한 장면의 입력 해석, 상태 갱신, 화면 구성을 담당한다.
- 장면 전환은 명시적인 결과나 요청으로 애플리케이션에 전달한다.
- 장면끼리 서로의 구체 클래스를 직접 생성하거나 소유하지 않는다.
- 공통 데이터와 서비스는 `SceneContext`를 통해 필요한 최소 범위만 전달한다.

## 5. 의존성 방향

허용하는 기본 의존성 방향은 다음과 같다.

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

- `Domain`은 가장 안쪽 계층이며 외부 계층을 참조하지 않는다.
- 저수준 콘솔 구현을 장면에서 직접 생성하지 않는다.
- 헤더의 순환 참조를 금지한다.
- 소유하지 않는 객체는 가능한 한 참조로 전달한다.
- 소유권은 `std::unique_ptr`로 명확히 표현하고 공유 소유권은 꼭 필요한 경우에만 사용한다.
- 전역 가변 상태와 싱글턴을 사용하지 않는다.

## 6. SOLID 적용 규칙

### SRP: 단일 책임 원칙

- 클래스가 변경되는 이유는 하나여야 한다.
- 입력, 규칙 계산, 상태 변경, 렌더링을 한 함수에서 동시에 처리하지 않는다.
- 한 클래스가 빠르게 커지면 메서드 분할보다 먼저 책임 분리가 필요한지 검토한다.

### OCP: 개방·폐쇄 원칙

- 새 장면이나 새 검 등급을 추가할 때 기존 핵심 로직의 거대한 `switch`를 계속 수정하지 않는다.
- 변화 가능성이 실제로 확인된 지점에만 다형성이나 데이터 테이블을 도입한다.
- 단순한 값 객체마다 불필요한 인터페이스를 만들지 않는다.

### LSP: 리스코프 치환 원칙

- 인터페이스 구현은 호출자가 기대하는 입력, 출력, 수명 주기 계약을 지킨다.
- 파생 클래스에서 기반 동작을 무효화하기 위한 빈 구현이나 예외 던지기를 금지한다.

### ISP: 인터페이스 분리 원칙

- 장면이 전체 엔진 객체를 받지 않도록 입력, 렌더링, 난수 등 작은 역할로 분리한다.
- 사용하지 않는 함수까지 구현하도록 강요하는 대형 인터페이스를 만들지 않는다.

### DIP: 의존성 역전 원칙

- 게임 규칙과 장면은 구체적인 Windows 콘솔 구현이 아닌 필요한 계약에 의존한다.
- 난수, 입력, 시간처럼 테스트를 어렵게 만드는 외부 요소는 경계에서 주입한다.
- 다만 구현이 하나뿐이고 교체·테스트 필요성이 없는 내부 값 객체에는 형식적인 인터페이스를 만들지 않는다.

## 7. 명명과 코드 스타일

이 프로젝트는 **Modern C++ 관용 스타일**을 기준으로 통일한다.
C++ Core Guidelines의 안전성과 RAII 원칙을 따르되,
Windows 및 게임 코드에서 널리 읽히는 명명법을 사용한다.
개인의 취향에 따라 파일마다 스타일을 바꾸지 않는다.

### 이름 규칙

| 대상 | 규칙 | 예시 |
| --- | --- | --- |
| 클래스, 구조체, 열거형, 타입 별칭 | `PascalCase` | `ForgeSession`, `SceneType` |
| 함수, 메서드 | `PascalCase` | `CalculateChance()`, `Render()` |
| 지역 변수, 매개변수 | `camelCase` | `deltaSeconds`, `forgeCost` |
| private 멤버 변수 | `camelCase_` | `currentHeat_`, `playerProgress_` |
| 컴파일 타임 상수 | `kPascalCase` | `kScreenWidth`, `kMaxHeat` |
| `enum class` 항목 | `PascalCase` | `SceneType::Forge` |
| 인터페이스 | 역할 앞에 `I` 사용 | `IScene`, `IRandomProvider` |
| 파일과 폴더 | 핵심 타입과 동일한 `PascalCase` | `ForgeSession.h`, `Rendering/` |

- 불리언은 `is`, `has`, `can`, `should`처럼 질문으로 읽히게 명명한다.
- 단위가 중요한 값은 이름에 단위를 포함한다 (`deltaSeconds`, `durationMs`).
- 컬렉션은 복수형으로 명명한다 (`strikeScores`, `particles`).
- 축약어와 의미 없는 이름(`data`, `temp`, `obj`, `manager`)을 피한다.
- 널리 알려진 짧은 반복 인덱스 `i`, 좌표 `x`, `y`는 제한적으로 허용한다.

### 파일과 선언 규칙

- 한 헤더에는 하나의 핵심 클래스 또는 하나의 밀접한 값 타입 묶음만 둔다.
- 선언은 `.h`, 구현은 `.cpp`에 둔다.
- 템플릿과 짧고 자명한 접근자만 필요한 경우 헤더에 구현할 수 있다.
- 헤더는 독립적으로 컴파일 가능해야 하며 자신이 사용하는 타입을 직접 포함한다.
- 헤더 포함 순서는 `자기 헤더 → 프로젝트 헤더 → 표준 라이브러리 → 플랫폼 헤더`로 구분한다.
- 헤더에는 `#pragma once`를 사용한다.
- 헤더의 순환 참조를 금지하고, 소유하지 않는 타입은 가능한 경우 전방 선언한다.
- `using namespace`는 헤더와 전역 범위에서 사용하지 않는다.
- 플랫폼 매크로의 오염을 막기 위해 필요한 경우 `NOMINMAX`처럼 범위를 명확히 제한한다.

### 포맷 규칙

- 들여쓰기는 공백 4칸을 사용하고 탭을 섞지 않는다.
- 중괄호는 선언문 다음 줄에 배치하는 Allman 스타일을 사용한다.
- 조건문과 반복문의 본문이 한 줄이어도 중괄호를 작성한다.
- 한 줄에는 하나의 명령문만 작성한다.
- 긴 식은 연산자의 의미가 보이도록 여러 줄로 정렬한다.
- 포인터와 참조 기호는 타입에 붙인다 (`Sword* sword`, `const Sword& sword`).
- 파일 끝에는 빈 줄 하나를 유지하고 불필요한 후행 공백을 남기지 않는다.

```cpp
if (currentHeat >= kMinResonanceHeat)
{
    StartResonance();
}
```

### 타입과 표현식 규칙

- C 스타일 캐스트를 금지하고 `static_cast`, `dynamic_cast` 등 의도가 드러나는 캐스트를 사용한다.
- `NULL`과 숫자 `0` 대신 `nullptr`을 사용한다.
- 범위가 있는 열거형인 `enum class`를 사용한다.
- 변하지 않는 값, 매개변수, 메서드에는 적극적으로 `const`를 적용한다.
- 타입이 명확하거나 반복자를 다룰 때 `auto`를 사용한다.
- 숫자 타입이나 반환 타입의 의미가 흐려지는 곳에서는 실제 타입을 작성한다.
- 매직 넘버를 금지하고 이름 있는 `constexpr` 상수나 규칙 데이터로 옮긴다.
- 컴파일 시간에 계산할 수 있는 단순 함수와 값에는 가능한 한 `constexpr`를 사용한다.
- `constexpr` 함수는 전역 상태나 플랫폼 API에 의존하지 않는 순수 계산을 우선한다.
- `consteval`과 `std::span`은 C++20 기능이므로 현재 C++17 코드에서 사용하지 않는다.
- 향후 프로젝트 표준을 C++20 이상으로 올릴 때 컴파일 타임 전용 함수에는 `consteval`,
  연속 메모리의 비소유 뷰에는 `std::span` 도입을 검토한다.
- 부동소수점 값에는 의도를 드러내는 접미사와 범위를 사용한다 (`1.0f`).

### 함수 규칙

- 함수는 하나의 책임과 한 단계의 추상화 수준을 유지한다.
- 함수 이름은 동작과 결과가 드러나는 동사로 시작한다.
- 상태를 바꾸지 않는 조회 함수는 `const`로 선언한다.
- 계산 결과, 성공 여부, 상태 전환 요청처럼 반환값을 무시하면 논리 오류가 되는 함수에는
  `[[nodiscard]]`를 사용한다.
- 단순 접근자나 의도적으로 반환값을 버릴 수 있는 함수까지 `[[nodiscard]]`를 남용하지 않는다.
- 출력 매개변수보다 반환값을 우선한다.
- 여러 결과가 필요하면 의미 있는 결과 구조체를 반환한다.
- 불리언 매개변수가 호출 의미를 감추면 열거형 또는 별도 함수를 사용한다.
- 함수가 길어질 때 단순히 줄 수만 나누지 말고 책임 분리가 필요한지 먼저 검토한다.
- 정상적인 게임 흐름을 제어하기 위해 예외를 사용하지 않는다.
- 예외를 발생시키지 않는다는 계약을 실제로 보장할 수 있는 함수에만 `noexcept`를 작성한다.
- 소멸자와 이동 연산은 가능한 한 `noexcept`를 유지한다.

```cpp
// 강화 시도에 필요한 비용을 계산하며 반환값을 반드시 사용해야 한다.
[[nodiscard]] constexpr int CalculateForgeCost(int swordLevel) noexcept;
```

### 매개변수 전달 규칙

| 목적 | 전달 방식 | 예시 |
| --- | --- | --- |
| 기본 타입, 열거형, 작고 복사 비용이 낮은 값 타입 | 값 `T` | `float deltaSeconds` |
| 크거나 복사 비용이 높은 읽기 전용 객체 | 상수 참조 `const T&` | `const PlayerProgress& progress` |
| 호출자의 객체를 수정하되 소유하지 않음 | 참조 `T&` | `ScreenBuffer& screen` |
| null을 허용하는 비소유 객체 | 포인터 `T*` | `SoundPlayer* optionalSound` |
| 단독 소유권을 전달 | `std::unique_ptr<T>` 값 | `SetScene(std::unique_ptr<IScene> scene)` |
| 복사 가능한 값을 받아 내부에 저장 | 값으로 받은 뒤 `std::move` | `SetTitle(std::wstring title)` |
| rvalue만 허용하는 의미가 명확한 경우 | rvalue 참조 `T&&` | 이동 전용 타입의 전용 오버로드 |

- 읽기 전용 문자열을 소유하지 않는 짧은 호출에는 C++17의 `std::string_view` 또는
  `std::wstring_view`를 사용할 수 있다.
- `string_view`는 원본 문자열보다 오래 보관하지 않으며 임시 문자열에서 얻은 뷰를 멤버로 저장하지 않는다.
- C++17에서 연속 범위를 전달할 때는 컨테이너의 `const&` 또는 반복자 쌍을 사용한다.
  포인터와 길이를 따로 전달해야 한다면 둘의 관계와 유효 범위를 한글 주석으로 명시한다.
- 전달 방식은 크기만 보고 정하지 않고 소유권과 수정 가능성을 먼저 드러내야 한다.
- 템플릿의 전달 참조(`T&&`)는 완벽 전달이 실제로 필요한 범용 코드에서만 사용한다.

### 메모리와 소유권 규칙

- 직접적인 `new`, `delete`, `malloc`, `free` 사용을 금지한다.
- 단독 소유는 값 또는 `std::unique_ptr`로 표현한다.
- 동적 객체를 생성할 때는 직접 생성자 호출로 스마트 포인터를 감싸지 않고
  `std::make_unique` 또는 `std::make_shared`를 사용한다.
- `std::shared_ptr`는 실제 공유 소유권이 있을 때만 사용한다.
- `std::shared_ptr` 사이에 상호 참조가 필요하면 소유하지 않는 관찰 방향은
  `std::weak_ptr`로 표현해 순환 참조를 방지한다.
- 수명이 상위 객체에 의해 명확히 보장되는 단순 비소유 관계에는
  `weak_ptr`보다 참조 또는 포인터를 우선한다.
- 소유하지 않는 필수 객체는 참조로 전달한다.
- 소유하지 않는 선택 객체만 포인터로 표현하며 null 가능성을 주석과 계약에 명시한다.
- 컨테이너와 문자열은 표준 라이브러리 타입을 우선한다.
- 자원 획득과 해제는 RAII 객체의 생성자와 소멸자에 묶는다.

### 클래스 설계 규칙

- 멤버 변수는 기본적으로 `private`으로 둔다.
- 단순 데이터 전달만 담당하는 값 타입에 한해 `struct`의 public 멤버를 허용한다.
- 접근자를 기계적으로 만들지 않고 클래스가 보장해야 하는 행위 중심의 메서드를 제공한다.
- 생성자가 끝난 객체는 즉시 유효한 상태여야 한다.
- 단일 매개변수 생성자와 변환 연산자는 의도하지 않은 묵시적 변환을 막기 위해
  기본적으로 `explicit`을 사용한다.
- 자원을 표준 컨테이너와 RAII 객체에 맡겨 소멸자, 복사·이동 생성자,
  복사·이동 대입 연산자를 직접 작성하지 않는 **Rule of Zero**를 기본으로 한다.
- 특별 멤버 함수 중 하나를 직접 정의해야 한다면 Rule of Five 전체의 복사·이동 의미를 검토하고,
  각 함수를 `= default` 또는 `= delete`로 명확하게 선언한다.
- 복사가 의미 없는 자원 소유 클래스는 복사 생성자와 복사 대입 연산자를 `= delete`로 금지한다.
- 상속보다 합성을 우선하고, 다형성이 실제로 필요한 경계에서만 인터페이스를 사용한다.
- 함수 오버라이드에는 반드시 `override`를 작성한다.
- 더 이상 파생될 의도가 없는 클래스나 추가 재정의를 허용하지 않는 가상 함수에는 `final`을 사용한다.
- `final`은 성능 최적화를 보장하기 위한 장치가 아니라 상속 의도를 보호하는 설계 계약으로 사용한다.

```cpp
// Windows 콘솔 세션을 단독 소유하며 복사를 허용하지 않는다.
class ConsoleSession final
{
public:
    explicit ConsoleSession(ConsoleSettings settings);
    ~ConsoleSession() noexcept;

    ConsoleSession(const ConsoleSession&) = delete;
    ConsoleSession& operator=(const ConsoleSession&) = delete;
    ConsoleSession(ConsoleSession&&) noexcept = default;
    ConsoleSession& operator=(ConsoleSession&&) noexcept = default;
};
```

### 계약과 오류 처리 규칙

- 내부 프로그래밍 오류와 절대로 깨지면 안 되는 불변 조건은 Debug 빌드의 `assert`로 즉시 탐지한다.
- 함수 진입 시 null 불허, 인덱스 범위, 도메인 값 범위처럼 호출자가 지켜야 하는 전제 조건을 검증한다.
- `assert` 안에는 상태 변경, 함수 호출 결과 저장 등 Release 빌드에서 사라지면 안 되는 동작을 넣지 않는다.
- `assert`는 Release 빌드에서 제거될 수 있으므로 사용자 입력, 파일 오류, 복구 가능한 런타임 실패 처리에 사용하지 않는다.
- 사용자 입력과 외부 시스템 오류는 조건문으로 검사하고 명시적인 상태, 결과 구조체 또는
  `std::optional`로 호출자에게 전달한다.
- 실패를 조용히 무시하지 않는다. 복구하거나 상위 계층에 전달하고, 플레이어에게 필요한 경우 메시지로 표시한다.
- 프로젝트 전용 Assertion 매크로를 도입한다면 플랫폼별 동작과 Release 정책을 한곳에 문서화한다.

### 가독성 우선 원칙

- 짧은 코드보다 의도가 바로 읽히는 코드를 우선한다.
- 중복 제거 때문에 서로 다른 책임을 억지로 하나의 함수나 클래스에 합치지 않는다.
- 영리하지만 설명이 필요한 한 줄 표현보다 이름 있는 중간 변수와 작은 함수를 사용한다.
- 주석은 `무엇을` 반복하지 않고 `왜` 그렇게 구현했는지를 설명한다.
- 기존 파일을 수정할 때는 주변 코드도 이 절의 스타일과 일관되게 정리한다.

## 8. 한글 주석 규칙

새로 작성하거나 구조를 변경하는 코드에는 중요한 의도와 계약을 설명하는 한글 주석을 반드시 작성한다.
코드 주석의 기본 언어는 한국어로 통일한다.
클래스명, API명, 표준 기술 용어는 의미가 더 명확한 경우 영어 원문을 함께 사용할 수 있다.

### 주석 밀도 기준

현재 리팩터링된 코드의 주석 밀도를 앞으로 작성하는 코드의 최소 기준으로 삼는다.
주석 개수 자체를 목표로 삼지 말고, 처음 읽는 개발자가 구현 이유와 처리 순서를 추론하지 않아도 되는 수준을 유지한다.

- 모든 `.cpp` 파일에는 구현 의도를 설명하는 한글 주석이 최소 1개 이상 있어야 한다.
- 80줄 이상의 `.cpp`에서는 주요 처리 구역마다 일반적으로 3개 이상의 의미 있는 주석을 둔다.
- 긴 함수에서는 입력 해석, 상태 갱신, 판정, 연출처럼 책임 단계가 바뀌는 지점에 주석을 둔다.
- 계산식에는 각 숫자를 그대로 설명하지 않고 가중치, 보정 범위, 밸런스 의도를 설명한다.
- 장면 전환에는 상태를 확정하는 시점과 해당 순서를 선택한 이유를 설명한다.
- 렌더링 코드에는 문자 배치 자체보다 깜빡임 방지, 시각적 피드백, 영역 구분 같은 연출 의도를 설명한다.
- 플랫폼 코드에는 인코딩, 운영체제 API 격리, 자원 복원, 실패 가능성을 설명한다.
- 컨테이너 제거, 수명 관리, 성능 최적화처럼 구현 선택에 대안이 있는 부분은 선택 이유를 설명한다.
- 짧고 자명한 접근자, 단순 대입, 반복문의 각 줄에는 주석을 추가하지 않는다.
- 리팩터링으로 구현 책임이 이동하면 관련 주석도 새 책임 위치로 함께 이동하거나 다시 작성한다.

### 반드시 한글 주석을 작성할 위치

- 새 클래스와 인터페이스 선언 바로 위
- 외부 계층에서 호출하는 public API의 계약 바로 위
- 새 `enum`, 주요 구조체, 값 객체 선언 바로 위
- 클래스의 프로퍼티 또는 멤버 변수 묶음 위
- 수명 주기, 소유권, 단위, 유효 범위가 중요한 멤버 변수 옆
- 확률 계산, 상태 전환, 보정 공식 등 핵심 게임 규칙 위
- Windows API, 콘솔 복구, 인코딩처럼 이유를 알기 어려운 플랫폼 코드 위
- 성능 최적화나 우회 코드처럼 구현 이유가 코드만으로 드러나지 않는 부분

### 문서화 형식

- 클래스, 인터페이스, 주요 구조체와 public API에는 Doxygen 호환 `///` 주석을 사용한다.
- private 구현 세부사항, 멤버 변수 묶음, 핵심 알고리즘 내부에는 일반 `//` 주석을 사용한다.
- public API 문서에는 필요한 경우 책임, 매개변수 범위, 반환 의미, 소유권, 실패 조건을 적는다.
- 자명한 접근자까지 형식적인 Doxygen 주석을 반복하지 않는다.
- 문서 자동 생성 도입 전에도 동일한 형식을 유지해 API 계약을 코드 가까이에 둔다.

헤더와 소스의 역할은 다음처럼 구분한다.

- 헤더 주석: 클래스 책임, public API 계약, 소유권, 값의 범위와 수명
- 소스 주석: 알고리즘 선택 이유, 상태 변경 순서, 예외적인 분기, 연출과 성능 의도
- 헤더에 이미 적힌 책임을 소스에서 그대로 반복하지 않는다.
- 소스는 `무엇을 하는 함수인지`보다 `왜 이 방식과 순서로 구현했는지`를 설명한다.

### 주석 작성 예시

```cpp
/// 한 번의 제련 과정과 세 번의 타격 결과를 관리한다.
/// 화면 출력과 최종 강화 판정은 담당하지 않는다.
class ForgeSession
{
public:
    /// 경과 시간만큼 제련 상태를 갱신한다.
    /// @param deltaSeconds 이전 프레임 이후 경과한 초 단위 시간
    void Update(float deltaSeconds);

private:
    // 0~100 범위의 현재 검 온도다.
    float heat_ = 50.0f;

    // 타격별 정확도를 0~1 범위로 순서대로 저장한다.
    std::vector<float> strikeScores_;
};
```

### 피해야 할 주석

- 코드를 그대로 한국어로 반복하는 주석
- 구현과 맞지 않는 오래된 주석
- `i를 1 증가시킨다`처럼 자명한 동작 설명
- 클래스 책임과 무관한 긴 작업 일지

리팩터링 시 동작이 바뀌면 관련 주석도 같은 변경에서 반드시 갱신한다.

## 9. 장면 구현 규약

각 장면은 같은 수명 주기 형태를 유지한다.

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

- `OnEnter`: 장면 진입 시 필요한 상태 초기화
- `Update`: 입력 해석과 장면 상태 갱신, 전환 요청 반환
- `Render`: 현재 상태를 그리며 게임 상태를 변경하지 않음
- `OnExit`: 장면 종료 시 필요한 정리

입력과 렌더링을 분리해 `Render`는 논리 상태를 변경하지 않게 한다.

## 10. 게임 규칙 구현 규약

- 강화 비용, 기본 확률, 실패 패널티는 `ForgeRules` 한 곳에서 관리한다.
- UI는 확률이나 비용을 다시 계산하지 않고 도메인이 제공한 값을 표시한다.
- 타격 점수와 최종 성공 확률은 항상 `0.0f~1.0f` 범위를 보장한다.
- 검의 강화 단계는 최소 0을 보장한다.
- 골드와 기억 조각은 음수가 되지 않게 한다.
- 강화 판정과 연출 시작을 분리한다. 연출 때문에 결과가 바뀌면 안 된다.
- 프레임 속도와 무관하게 같은 속도로 동작하도록 모든 실시간 갱신에 `deltaSeconds`를 사용한다.

## 11. 작업 절차

1. 작업 전 이 문서와 관련 소스 파일을 읽는다.
2. 변경할 책임이 어느 계층에 속하는지 먼저 결정한다.
3. 기존 클래스가 다른 책임까지 떠안게 된다면 새 책임을 분리한다.
4. 새 클래스, 프로퍼티 묶음, 핵심 로직에 한글 주석을 작성하고 현재 주석 밀도 기준을 확인한다.
5. 프로젝트 파일에 새 `.cpp`와 `.h`가 포함됐는지 확인한다.
6. Debug x64 빌드를 실행한다.
7. `git diff --check`로 공백 오류를 확인한다.
8. 구조나 게임 규칙이 달라졌다면 이 문서를 함께 갱신한다.

## 12. 완료 조건

코드 작업은 다음 조건을 모두 만족해야 완료로 본다.

- Debug x64 빌드 성공
- 새 경고를 추가하지 않음
- `Main.cpp`에 게임 로직을 추가하지 않음
- 각 클래스의 책임이 한 문장으로 설명 가능
- Domain 계층이 Windows API와 렌더링에 의존하지 않음
- 새 핵심 클래스와 멤버에 필요한 한글 주석 존재
- 모든 새 `.cpp`에 구현 의도 주석이 있고 긴 구현의 주요 처리 구역이 설명됨
- 중요한 반환값에 `[[nodiscard]]`가 적용되고 단일 인자 생성자에 `explicit`이 적용됨
- 소유권과 복사·이동 정책이 코드에서 명확하게 표현됨
- 내부 불변 조건과 복구 가능한 런타임 오류가 구분되어 처리됨
- 입력, 상태 갱신, 판정, 렌더링의 경계가 명확함
- 변경된 구조와 이 문서의 설명이 일치함

## 13. 다음 확장 원칙

현재 수직 슬라이스의 구조 분리는 완료됐다. 이후 기능은 다음 순서로 확장한다.

1. 새 기능의 순수 규칙과 결과 타입을 `Game/Domain`에 먼저 정의한다.
2. 규칙에 필요한 외부 요소는 작은 인터페이스로 주입해 테스트 가능하게 만든다.
3. 기능별 장면을 추가하고 공통 시각 요소만 `GameHudRenderer` 또는 별도 렌더러로 추출한다.
4. 구체 장면 생성은 `GameApplication::CreateScene` 한곳에서만 수행한다.
5. Windows 전용 기능은 `Platform/Console`에 구현하고 게임 계층에는 계약만 노출한다.
6. 보스전, 저장, 사운드 같은 대형 기능을 추가하기 전에 해당 책임과 데이터 수명을 문서화한다.
7. 순수 게임 규칙부터 자동 테스트를 추가하고 플랫폼 코드는 통합 테스트로 검증한다.

새 기능을 `Main.cpp`에 직접 추가하거나 Domain에서 콘솔 API를 호출하지 않는다.
