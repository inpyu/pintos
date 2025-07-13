# 🧭 중앙대학교 25-1 운영체제 프로젝트 2: PintOS Deadlock 교차로 문제

## 📌 프로젝트 개요

본 프로젝트는 PintOS 환경에서 **데드락(Deadlock)** 개념을 기반으로 교차로 내 차량(스레드)의 동기화 및 충돌 회피 로직을 구현하는 것을 목표로 합니다. 각 차량은 출발지(A/B/C/D)에서 도착지(A/B/C/D)까지 **우측통행** 원칙에 따라 이동하며, 다음 요구사항을 만족해야 합니다.

## ⚙️ 실행 환경

* PintOS (소스 코드: [제공된 구글 드라이브 링크](https://drive.google.com/file/d/1_SvGtcsb345_z5xrZlYDfwSvQj3mSXqs/view?usp=sharing))
* 실행 명령어 예시:

  ```bash
  ../../utils/pintos crossroads aAA:bBD:cCD:dDB:fAB5.12:gAC6.13
  ```

## ✅ 필수 요구사항

### 1. 🧱 Deadlock 처리 및 방지

* 데드락 발생 조건:
  Mutual Exclusion / Hold and Wait / No Preemption / Circular Wait
* 처리 방식:

  * **Prevent/ Avoid/ Detect** 중 하나 이상 사용
  * 교차로 진입 시 신호등 동기화(sem, lock)를 이용한 충돌 회피
  * 현실적 시뮬레이션 구현 (차량 1대씩만 통과하는 단순 방식을 금지)

### 2. 🚦 우선순위 동기화 기법 구현

* 기존 `synch.c/h` 외에 **우선순위 기반 동기화 도구** 직접 구현

  * 신호등 (traffic light)
  * 일반 차량 vs. 앰뷸런스 간 우선권 처리

### 3. 🚗 차량 이동 로직

* 7x7 교차로 맵 내에서 A\~D 차량은 경로를 따라 이동
* 차량당 1칸씩 이동, 동시에 같은 칸 접근 금지
* `vehicle.c`에 정의된 경로 사용 (수정 금지)

### 4. ⏱ 단위 스텝 기반 동기화

* 모든 차량이 이동 또는 대기한 후 `crossroads_step`을 증가시켜야 함
* `ats.h` 내 `unitstep_changed()` 호출 필수
* sleep 등 시간 대기 함수 사용 금지

### 5. 🚑 앰뷸런스 우선 통과 구현

* 차량 명령 예: `fAB5.12`

  * f는 A→B로 이동, 5 스텝에 출발, 12 스텝 안에 도착
* 앰뷸런스는 일반 차량보다 우선 통과
* 골든타임 미준수 시 실패 처리

## 📂 구현 파일

* 수정 가능: `vehicle.c/h`, `crossroads.c`, `map.c`, `blinker.c/h`
* 수정 금지: `ats.c/h`, `crossroads.h`

## 📝 보고서 작성 내용

* 데드락 회피 전략 설명
* 동기화 기법 설계 (우선순위 포함)
* 앰뷸런스 시간 제약 처리 방식
* 실험 시나리오 및 결과 정리 (예: aBD\:bCD\:cAB\:dCA 등)
* 문제 해결 과정과 고려사항 정리
