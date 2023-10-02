# 2020 운영체제 프로젝트 모음.
스케줄러 구현

### 💻 Technology
+ C 

### 🔭 Work
+ 구조
<img src="https://github.com/DanielPark827/Inha_OS_CPU_Scheduling_3_1/assets/59303671/98001bc5-9023-479d-ac79-3c603a63ed46.png" width="400" height="200"/>

1.1.1 큐 간 스케줄링
- Priority 스케줄링
- 우선순위는 priority -> SJF -> HRN 순입니다.

1.1.2 큐 내부 스케줄링
1) NPPS(Non-preemptive Priority Scheduling Algorithm) : 사용자가 지정한 우선순위(priority)가 높은 순으로 프로세스를 스케줄링 후 작업을 처리합니다.
2) SJF(Shortest Job First) : 실행시간(burst)가 짧은 순으로 프로세스를 스케줄링 후 작업을 처리합니다.
3) HRN(Highest Response-Ratio Next) : ‘{대기 시간 + 실행 시간(burst)} / 실행 시간(burst)’으로 프로세스의 우선순위를 지정한 후, 우선순위가 높은 순으로 프로세스 작업을 처리합니다. (난이도 (상)을 의도하였습니다.)

1.2 설계 의도
큐간 스케줄링에서 priority 스케줄링은 사용자가 우선순위를 직접 지정한다는 프로세스라면 가장 중요하고 먼저 처리되야하는 프로세스라고 생각하여 가장 높은 우선순위를 갖도록 하였습니다.
이후 SJF 스케줄링을 통해 가장 짧은 실행시간을 갖는 프로세스를 처리하게 하여 스케줄링의 효율성을 높이고자 하였습니다.
마지막으로 SJF에서의 단점 중 하나인 Starvation을 보완하기 위해, HRN 스케줄링을 세번째 우선순위로 설정하게 되었습니다.

1.3 강조점
1) pthread 라이브러리와 semaphore 라이브러리를 사용하여 thread을 이용한 스케줄링을 구현하였습니다.
2) Multi-level queue 스케줄링을 구현하여 난이도 (중)을 충족시키고자 의도하였습니다.
3) HRN 스케줄링을 구현하여 난이도 (상)을 충족시키고자 의도하였습니다.

2. 스케줄러 구현 코드 설명

2.1 개발 환경
본 프로젝트는 ‘Visual Studio 2017’ 환경에서 C언어를 통해 수행되었습니다.

2.2 스케줄링 시나리오
1. main함수에서 프로세스 정보를 받아 Queue 1, 2, 3에 프로세스 정보를 삽입합니다.
2. main함수에서 pthread_create(&thread[0], NULL, MLQS, NULL);를 호출하여 Multi-level queue 스케줄링을 시작합니다.
3. MLQS 함수에서 pthread_create(&thread[idx], NULL, NPPS, NULL);를 호출하여 NPPS 스케줄링을 첫 번째로 실행합니다.
4. MLQS 함수에서 pthread_create(&thread[idx], NULL, SJF, NULL); 를 호출하여 SJF 스케줄링을 두 번째로 실행합니다.
5. MLQS 함수에서 pthread_create(&thread[idx], NULL, HRN, NULL); 를 호출하여 HRN 스케줄링을 세 번째로 실행합니다.
6. MLQS 함수에서 pthread_join(thread[0], NULL);를 호출하여 최종적인 스케줄링을 종료합니다.

