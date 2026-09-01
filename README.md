# Kingdom

영역을 그려가며 땅을 개발하고, 성장하는 영웅으로 그 땅을 지키는 싱글 판타지 전략 게임.

기획서는 [Docs/](Docs/) 참고. 개발 계획과 기술 원칙은 [Docs/6.개발과_기술.md](Docs/6.개발과_기술.md).

## 구조

- `Source/KingdomCore/` — 순수 C++ 시뮬레이션 코어. **UE 헤더 금지** (`CoreMinimal.h` 포함 즉시 위반). CMake로 독립 빌드/테스트 가능
- `Source/KingdomGame/` — UE5 뷰 레이어. 코어 상태를 직접 수정하지 않고, 커맨드 제출 후 결과를 읽기만 한다
- `Docs/` — 기획서 (Notion 미러)

## 컨벤션

- 폴더: PascalCase
- 파일: 번호 접두는 `1.` 형식(하이픈 금지), 이름 내 공백은 `_`
- 코드: **언리얼 코딩 컨벤션** — KingdomCore의 순수 C++에도 동일 적용 (탭 들여쓰기, Allman 중괄호, 타입 접두 `F`/`E`/`I`/`T`, UObject 계열은 `U`/`A`, bool은 `b` 접두, 함수·변수 PascalCase, 네임스페이스 PascalCase). 기본 타입은 `KingdomCore/CoreTypes.h`의 별칭(`Kingdom::uint64` 등)으로 UE와 같은 표기를 쓴다 (코어는 UE 헤더 금지이므로 표준 타입에 별칭만 씌운 것)

## KingdomCore 빌드 & 테스트 (UE 불필요)

VS 개발자 명령 프롬프트에서:

```
cd Source/KingdomCore
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

## UE 빌드

`Kingdom.uproject` 우클릭 → *Generate Visual Studio project files* 후 빌드,
또는:

```
"C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat" KingdomEditor Win64 Development -project="%CD%\Kingdom.uproject" -waitmutex
```

에디터에서 Play 하면 화면 좌상단에 `KingdomCore 0.0.1-phase0 | SimClock tick=10`이 찍힌다 (Phase 0 게이트).
