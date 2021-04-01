#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0

typedef struct _process {
	int id; // process ID
	int return_time; 
	int waiting_time;
	int arrive_time;
	int response_time;
	int turnaround_time;
	int priority; // 우선 순위
	int completed; // process 작업이 처리되었는지 여부
	int burst; // 실행 시간
}Process;

int p_count = 0; //process의 개수
int i_q1 = 0; // Queue 1 내 프로세스 개수
int i_q2 = 0; // Queue 2 내 프로세스 개수
int i_q3 = 0; // Queue 3 내 프로세스 개수

int class_num; // 프로세스 입력 시 몇번째 큐로 들어갈지 결정
// 입력된 프로세스 정보를 받기 위한 임시 변수
int sub_id;
int sub_arrive_time;
int sub_burst;
int sub_priority;

Process *q1; // Queue 1
Process *q2; // Queue 2
Process *q3; // Queue 3

int idx = 0; // thread 배열에 대한 index
pthread_t thread[5]; // 스레드

sem_t semaphore; // 세마포어


void p_init(Process q[], int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		q[i].waiting_time = 0;
		q[i].return_time = 0;
		q[i].response_time = 0;
		q[i].completed = FALSE;
	}
}

void print_table(Process p[], int n)
{
	int i;

	puts("\t*-----*------------*-------------*----------*-------------*-----------------*--------------*-----------------*");
	puts("\t| PID | Burst Time | Arrive Time | Priority | Return Time |  Response Time  | Waiting Time | Turnaround Time |");
	puts("\t*-----*------------*-------------*----------*-------------*-----------------*--------------*-----------------*");

	for (i = 0; i < n; i++)
	{
		printf("\t| %3d |     %3d    |     %3d     |    %3d   |     %3d     |      %3d        |      %3d     |        %3d      |\n",
			p[i].id, p[i].burst, p[i].arrive_time, p[i].priority, p[i].return_time, p[i].response_time, p[i].waiting_time, p[i].turnaround_time);

		puts("\t*-----*------------*-------------*----------*-------------*-----------------*--------------*-----------------*");
	}

	puts("\n");
}

int compare_by_return_time(const void *a, const void *b)
{
	//받아온 인자를 (Process *)로 형변환 해줍니다.
	Process *pA = (Process *)a;
	Process *pB = (Process *)b;

	// return_time의 대소관계에 따라 리턴값을 다르게 정의합니다.
	if (pA->return_time < pB->return_time)
		return -1;

	else if (pA->return_time > pB->return_time)
		return 1;

	else
		return 0;
}

void quick_sort_by_return_time(Process q[], int len)
{//len : Queue의 길이
	qsort(q, len, sizeof(Process), compare_by_return_time);
}

int compare_by_priority(Process *a, Process *b)
{
	//받아온 인자를 (Process *)로 형변환 해줍니다.
	Process *pA = (Process *)a;
	Process *pB = (Process *)b;

	//priority가 서로 같다면 실행 시간이 더 짧은 프로세스를 우선적으로 처리합니다.
	if (a->priority == b->priority) {
		if (a->burst != b->burst) {
			if (a->burst < b->burst)
				return -1;
			else
				return 1;
		}
		else
			return 0;
	}
	else {// priority의 대소관계에 따라 리턴값을 다르게 정의합니다.
		if (a->priority < b->priority)
			return -1;
		else if (a->priority > b->priority)
			return 1;
	}
}

void quick_sort_by_priority_time()
{// Queue 1 내 프로세스를 우선순위가 높은 순으로 퀵 정렬합니다.
	qsort(q1, i_q1, sizeof(Process), compare_by_priority);
}

void gantt_chart(Process p[], int len)
{
	int i, j;
	printf("\t ");

	for (i = 0; i < len; i++)
	{
		for (j = 0; j < p[i].burst; j++)
		{
			printf("--");
		}
		printf(" ");
	}

	printf("\n\t|");

	for (i = 0; i < len; i++)
	{
		for (j = 0; j < p[i].burst - 1; j++)
		{
			printf(" ");
		}
		printf("%2d", p[i].id);

		for (j = 0; j < p[i].burst - 1; j++)
		{
			printf(" ");
		}

		printf("|");
	}

	printf("\n\t ");

	for (i = 0; i < len; i++)
	{
		for (j = 0; j < p[i].burst; j++)
		{
			printf("--");
		}
		printf(" ");
	}
	printf("\n\t");
	printf("0");

	for (i = 0; i < len; i++)
	{
		for (j = 0; j < p[i].burst; j++)
		{
			printf("  ");
		}
		if (p[i].return_time > 9)
		{
			printf("\b");
		}
		printf("%d", p[i].return_time);
	}
	printf("\n");
}

//**************************************************//
// Q1 : Non-preemptive Priority Scheduling Algorithm
void Cal_for_npps(Process p[], int len)
{
	int i, j;
	int check; // TRUE : 모든 프로세스가 완료되지 않음, FALSE : 모든 프로세스 완료
	int min; // Priority가 가장 높은 index 저장
	int time = 0; // 현재 시간

	// Queue 1의 첫번째 process의 정보를 갱신합니다.
	p[0].return_time = p[0].burst;
	p[0].turnaround_time = p[0].return_time - p[0].arrive_time;
	p[0].response_time = 0;
	p[0].completed = TRUE;
	
	//현재 시간을 Queue 1의 첫번째 process의 burst로 갱신합니다.
	time = p[0].burst;

	for (j = 1; j < i_q1; j++)
	{
		p[j].response_time = time - p[j].arrive_time;
		p[j].return_time = time + p[j].burst;
		p[j].turnaround_time = p[j].return_time - p[j].arrive_time;
		p[j].waiting_time = time - p[min].arrive_time;
		p[j].completed = TRUE;

		// 현재 시간을 현재 최고 우선순위 process의 실행 시간을 더하여 갱신
		time += p[min].burst;
	}
}

void *NPPS(void *arg)
{
	int i;
	int total_waiting_time = 0;
	int total_turnaround_time = 0;
	int total_response_time = 0;

	// 프로세스 작업이 처리된 후 갱신되어야할 변수 초기화
	p_init(q1, i_q1);

	// 우선순위가 높은 순으로 프로세스 정렬
	quick_sort_by_priority_time();

	// 정렬 후 프로세스 작업 처리에 따른 프로세스 정보 갱신
	Cal_for_npps(q1, i_q1);

	//스케줄링 결과 출력
	for (i = 0; i < i_q1; i++)
	{
		total_waiting_time += q1[i].waiting_time;
		total_turnaround_time += q1[i].turnaround_time;
		total_response_time += q1[i].response_time;
	}

	//간트 차트 출력을 위해 return_time순으로 정렬
	quick_sort_by_return_time(q1, i_q1);

	printf("\tNPPS\n\n");

	gantt_chart(q1, i_q1);

	printf("\n\tAverage_Waiting_Time     : %-2.2lf\n", (double)total_waiting_time / (double)i_q1);
	printf("\tAverage_Turnaround_Time  : %-2.2lf\n", (double)total_turnaround_time / (double)i_q1);
	printf("\tAverage_Response_Time    : %-2.2lf\n\n", (double)total_response_time / (double)i_q1);

	print_table(q1, i_q1);
	pthread_exit(0);
}

//**************************************************//
// Q3 : HRN(Highest Response-Ratio Next)

void *HRN(void *arg)
{
	int i;
	int time, locate; //현재 시간과 프로세스 위치 저장을 위한 변수
	int total_burst_time = 0;
	int total_waiting_time = 0;
	int total_turnaround_time = 0;
	int total_response_time = 0;

	float hrr, temp; // HRN 알고리즘의 우선순위를 저장할 변수

	// 프로세스 작업이 처리된 후 갱신되어야할 변수 초기화
	p_init(q3, i_q3);

	// total_burst_time 연산
	for (i = 0; i < i_q3; i++)
	{
		total_burst_time += q3[i].burst;
	}

	// 현재 시간이 total_burst_time에 도달할때까지 루프문이 진행됩니다.
	for (time = q3[0].arrive_time; time < total_burst_time;)
	{
		hrr = -99999;

		for (i = 0; i < i_q3; i++)
		{
			//완료되지 않은 프로세스일 경우
			if (q3[i].completed != TRUE)
			{
				// HRN 알고리즘에 사용될 우선순위 계산
				temp = (q3[i].burst + (time - q3[i].arrive_time)) / q3[i].burst;
				
				// 프로세스의 우선순위가 hrr보다 크다면
				if (hrr < temp)
				{
					//우선순위 및 인덱스 갱신
					hrr = temp;
					locate = i;
				}
			}
		}

		// 우선순위가 가장 높은 프로세스 작업 처리 후 현재 시간 갱신
		time += q3[locate].burst;

		// 처리된 프로세스 정보 갱신
		q3[locate].waiting_time = time - q3[locate].arrive_time - q3[locate].burst;
		q3[locate].turnaround_time = time - q3[locate].arrive_time;
		q3[locate].return_time = q3[locate].turnaround_time + q3[locate].arrive_time;
		q3[locate].response_time = q3[locate].waiting_time;
		q3[locate].completed = TRUE;

		total_waiting_time += q3[locate].waiting_time;
		total_turnaround_time += q3[locate].turnaround_time;
		total_response_time += q3[locate].response_time;
	}
	//간트 차트 출력을 위해 return_time순으로 정렬
	quick_sort_by_return_time(q3, i_q3);

	printf("\tHRN Algorithm\n\n");

	gantt_chart(q3, i_q3);

	printf("\n\tAverage_Waiting_Time     : %-2.2lf\n", (double)total_waiting_time / (double)i_q3);
	printf("\tAverage_Turnaround_Time  : %-2.2lf\n", (double)total_turnaround_time / (double)i_q3);
	printf("\tAverage_Response_Time    : %-2.2lf\n\n", (double)total_response_time / (double)i_q3);

	print_table(q3, i_q3);
	pthread_exit(0);
}

//**************************************************//
// Q3 : SJF(Shortest Job First)

void cal_for_sjf(Process *p, int len)
{
	int i, j;
	int cur_time = 0; //현재 시간
	int min = 0; // 최소 실행 시간을 갖는 process의 index를 저장

	// 첫 번째 프로세스는 이전에 아무 프로세스도 없기 때문에 바로 실행
	p[0].completed = TRUE;
	p[0].return_time = p[0].burst;
	p[0].turnaround_time = p[0].burst - p[0].arrive_time;
	p[0].waiting_time = 0;

	//첫번째 프로세스 작업 처리 후 현재 시간 갱신
	cur_time = p[0].burst;

	for (i = 1; i < len; i++)//최소 실행 시간을 가지는 프로세스 작업 처리를 위한 루프
	{
		for (j = 1; j < len; j++)
		{
			if (p[j].completed == TRUE)
			{//이미 완료된 프로세스일 경우 다음 루프로 이동
				continue;
			}

			else
			{//처리되지 않은 프로세스의 경우 min 갱신
				min = j;
				break;
			}
		}

		for (j = 1; j < len; j++)
		{//최소 실행 시간을 가지면서, 처리되지 않은 프로세스 탐색
			if ((p[j].completed == FALSE) && (p[j].burst < p[min].burst))
			{//탐색 성공 시 min 갱신
				min = j;
			}
		}

		// 최소 실행 시간을 갖는 프로세스 작업 처리 후 프로세스 정보 및 현재 시간 갱신
		p[min].waiting_time = cur_time - p[min].arrive_time;
		p[min].completed = TRUE;
		cur_time += p[min].burst;
		p[min].return_time = cur_time;
		p[min].turnaround_time = p[min].return_time - p[min].arrive_time;
	}
}


void *SJF(void *arg)
{
	int i;
	int total_waiting_time = 0;
	int total_turnaround_time = 0;
	int total_response_time = 0;

	// 프로세스 작업이 처리된 후 갱신되어야할 변수 초기화
	p_init(q2, i_q2);

	//프로세스 작업 처리에 따른 프로세스 정보 갱신
	cal_for_sjf(q2, i_q2);

	//프로세스 작업이 처리된 후 수정되어야할 정보 갱신
	for (i = 0; i < i_q2; i++)
	{
		q2[i].return_time = q2[i].turnaround_time + q2[i].arrive_time;
		q2[i].response_time = q2[i].waiting_time;
		total_waiting_time += q2[i].waiting_time;
		total_turnaround_time += q2[i].turnaround_time;
		total_response_time += q2[i].response_time;
	}

	printf("\tSJF Algorithm\n\n");

	//간트 차트 출력을 위해 return_time순으로 정렬
	quick_sort_by_return_time(q2, i_q2);

	gantt_chart(q2, i_q2);

	printf("\n\tAverage_Waiting_Time     : %-2.2lf\n", (double)total_waiting_time / (double)i_q2);
	printf("\tAverage_Turnaround_Time  : %-2.2lf\n", (double)total_turnaround_time / (double)i_q2);
	printf("\tAverage_Response_Time    : %-2.2lf\n\n", (double)total_response_time / (double)i_q2);

	print_table(q2, i_q2);
}

void *MLQS(void *arg)
{

	sem_wait(&semaphore); //semaphore 기다리기
	//NPPS 스케줄링을 thread[1]에서 실행
	printf("------------------------------------------------------------------\n");
	pthread_create(&thread[1], NULL, NPPS, NULL);
	pthread_join(thread[1], NULL);
	printf("------------------------------------------------------------------\n");
	

	//SJF 스케줄링을 thread[2]에서 실행
	pthread_create(&thread[2], NULL, SJF, NULL);
	pthread_join(thread[2], NULL);
	printf("------------------------------------------------------------------\n");

	
	//HRN 스케줄링을 thread[3]에서 실행
	pthread_create(&thread[3], NULL, HRN, NULL);
	pthread_join(thread[3], NULL);
	sem_post(&semaphore); //

	pthread_exit(0);
}


int main()
{
	sem_init(&semaphore, 0, 1); // semaphore 초기화

	//Queue 1, 2, 3 초기화
	q1 = (Process *)malloc(sizeof(Process) * 100);
	q2 = (Process *)malloc(sizeof(Process) * 100);
	q3 = (Process *)malloc(sizeof(Process) * 100);

	// 파일을 통해 프로세스 정보를 받기
	FILE *fpp = NULL;
	fpp = fopen("sample.txt", "r");

	// 파일 받아오기 실패 시 오류 메시지 출력
	if (fpp == NULL)
	{
		printf("OPEN ERROR\n");
		return 0;
	}

	while (!feof(fpp))
	{
		// 프로세스 정보 받기
		fscanf(fpp, "%d %d %d %d %d",
			&class_num, &sub_id, &sub_arrive_time, &sub_burst, &sub_priority);

		if (feof(fpp))
		{
			break;
		}

		if (class_num == 1)// Queue 1에 프로세스 삽입
		{
			q1[i_q1].id = sub_id;
			q1[i_q1].arrive_time = sub_arrive_time;
			q1[i_q1].burst = sub_burst;
			q1[i_q1].priority = sub_priority;
			i_q1++;
		}
		else if (class_num == 2)// Queue 2에 프로세스 삽입
		{
			q2[i_q2].id = sub_id;
			q2[i_q2].arrive_time = sub_arrive_time;
			q2[i_q2].burst = sub_burst;
			q2[i_q2].priority = sub_priority;
			i_q2++;
		}
		else if (class_num == 3)// Queue 3에 프로세스 삽입
		{
			q3[i_q3].id = sub_id;
			q3[i_q3].arrive_time = sub_arrive_time;
			q3[i_q3].burst = sub_burst;
			q3[i_q3].priority = sub_priority;
			i_q3++;
		}

		// 프로세스 총 개수 +1
		p_count++;
	}

	// Multi level queue 스케줄링을 thread[0]에서 실행
	pthread_create(&thread[0], NULL, MLQS, NULL);
	pthread_join(thread[0], NULL);

	// 할당한 리소스 반환
	fclose(fpp);
	free(q1);
	free(q2);
	free(q3);

	system("pause");
	return 0;
}


