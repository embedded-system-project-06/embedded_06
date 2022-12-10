#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <stdlib.h>
#include <pthread.h> //쓰레드 관련 함수
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>

#define TRIGPIN 15
#define ECHOPIN 18
#define PWM1 13
#define BAUD_RATE 115200

static const char *UART2_DEV = "/dev/ttyAMA1";         // UART2 연결을 위한 장치 파일
unsigned char serialRead(const int fd);                // 1Byte 데이터를 수신하는 함수
void serialWrite(const int fd, const unsigned char c); // 1Byte 데이터를 송신하는 함수
int flag = 0;
int res = 0;
int count = 0;
int nocount = 0;
const char *danger = " stranger is coming!!\n";
const char *noDanger = "  \n";

pthread_mutex_t mid;

// 1Byte 데이터를 수신하는 함수
unsigned char serialRead(const int fd)
{
    unsigned char x;
    if (read(fd, &x, 1) != 1) // read 함수를 통해 1바이트 읽어옴
        return -1;
    return x; //읽어온 데이터 반환
}

// 1Byte 데이터를 송신하는 함수
void serialWrite(const int fd, const unsigned char c)
{
    write(fd, &c, 1); // write 함수를 통해 1바이트 씀
}

void initMyTone(int gpio, int freq)
{
    pwmSetRange(100);
    int temp = 192000;
    temp = temp / freq;
    pwmSetClock(temp);
    pwmWrite(gpio, 0);
}

void *threadFunc1(void *data) //초음파 센서 사용 쓰레드
{
    float distance, start, end;
    while (1)
    {
        digitalWrite(TRIGPIN, 0);
        digitalWrite(TRIGPIN, 1);
        delayMicroseconds(10);
        digitalWrite(TRIGPIN, 0);
        while (digitalRead(ECHOPIN) == 0)
            start = micros();
        while (digitalRead(ECHOPIN) == 1)
            end = micros();
        distance = (end - start) / 58;
        printf("distance(cm): %f\n", distance);

        if (distance < 50 && distance > 30)
        {
            pthread_mutex_lock(&mid);
            count++;
            res = 1;
            printf("alarm!\n");
            printf("res is now 1\n");
            pthread_mutex_unlock(&mid);
        }
        else if (distance < 30 && distance > 10)
        {
            pthread_mutex_lock(&mid);
            count++;
            res = 2;
            printf("alarm!!\n");
            printf("res is now 2\n");
            pthread_mutex_unlock(&mid);
        }
        else if (distance < 10 && distance > 0)
        {
            pthread_mutex_lock(&mid);
            count++;
            res = 3;
            printf("alarm!!!\n");
            printf("res is now 3\n");
            pthread_mutex_unlock(&mid);
        }
        else if (distance > 50)
        {
            pthread_mutex_lock(&mid);
            count = 0;
            res = 0;
            nocount++;
            printf("res is now 0\n");
            pthread_mutex_unlock(&mid);
        }
        delay(200);
    }
}

void *threadFunc2(void *data) // 부저 사용 쓰레드
{
    printf("now testing.\n");
    while (1)
    {
        int temp = 192000;
        if (res == 1 && flag == 1)
        {
            temp = temp / 262;
            pwmSetClock(temp);
            pwmWrite(PWM1, 80);
            delay(50);
        }
        else if (res == 2 && flag == 1)
        {
            temp = temp / 523;
            pwmSetClock(temp);
            pwmWrite(PWM1, 80);
            delay(50);
        }
        else if (res == 3 && flag == 1)
        {
            temp = temp / 1047;
            pwmSetClock(temp);
            pwmWrite(PWM1, 80);
            delay(50);
        }
        else if (res == 0 || flag == 0)
        {
            pwmWrite(PWM1, 0);
            delay(50);
        }
    }
}

void *threadFunc3(void *data) // 블루투스 모듈 사용 쓰레드
{
    int fd_serial;     // UART2 파일 서술자
    unsigned char dat; //데이터 임시 저장 변수
    int temp;

    if ((fd_serial = serialOpen(UART2_DEV, BAUD_RATE)) < 0)
    { // UART2 포트 오픈
        printf("Unable to open serial device.\n");
    }
    while (1)
    {
        if (count >= 10 && flag == 1)
        {
            write(fd_serial, danger, strlen(danger));
            pthread_mutex_lock(&mid);
            count = 0;
            pthread_mutex_unlock(&mid);
        }
        if (nocount >= 10 || flag == 0)
        {
            write(fd_serial, noDanger, strlen(noDanger));
            pthread_mutex_lock(&mid);
            nocount = 0;
            pthread_mutex_unlock(&mid);
        }
        if (serialDataAvail(fd_serial))
        {                                //읽을 데이터가 존재한다면,
            dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
            temp = (int)dat - 48;
            if (temp == 1)
            {
                flag = 1;
            }
            else if (temp == 0)
            {
                flag = 0;
            }
            printf("%d\n", flag);
            fflush(stdout);
            //입력 받은 데이터를 다시 보냄 (Echo)
        }
        delay(10);
    }
}

int main(int argc, char **argv)
{

    pthread_t p_thread1; // thread ID 저장 변수
    pthread_t p_thread2;
    pthread_t p_thread3;
    int r_val1; // pthread 관련 함수 반환 값 저장
    int r_val2;
    int r_val3;
    int status;      // thread 종료시 반환하는 값 저장 변수
    int value = 229; //쓰레드 호출 함수에 전달할 인자

    pthread_mutex_init(&mid, NULL);

    if (wiringPiSetupGpio() < 0)
    {
        printf("wiringPiSetup() is failed\n");
        return 1;
    }

    pinMode(TRIGPIN, OUTPUT);
    pinMode(ECHOPIN, INPUT);
    pinMode(PWM1, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    initMyTone(PWM1, 262);

    // thread 생성
    r_val1 = pthread_create(&p_thread1, NULL, threadFunc1, (void *)&value);
    if (r_val1 < 0)
    { //반환 값이 0보다 작으면 쓰레드 생성 오류
        perror("pthread_create() error\n");
        exit(0);
    }
    r_val2 = pthread_create(&p_thread2, NULL, threadFunc2, (void *)&value);
    if (r_val2 < 0)
    { //반환 값이 0보다 작으면 쓰레드 생성 오류
        perror("pthread_create() error\n");
        exit(0);
    }
    r_val3 = pthread_create(&p_thread3, NULL, threadFunc3, (void *)&value);
    if (r_val3 < 0)
    { //반환 값이 0보다 작으면 쓰레드 생성 오류
        perror("pthread_create() error\n");
        exit(0);
    }
    // thread 가 종료될 때까지 대기. 종료시 반환 값은 status 에 저장
    r_val1 = pthread_join(p_thread1, (void **)&status);
    if (r_val1 < 0)
    { //반환 값이 0보다 작으면 pthread_join 오류
        perror("pthread_join() error\n");
        exit(0);
    }

    r_val2 = pthread_join(p_thread2, (void **)&status);
    if (r_val2 < 0)
    { //반환 값이 0보다 작으면 pthread_join 오류
        perror("pthread_join() error\n");
        exit(0);
    }

    r_val3 = pthread_join(p_thread3, (void **)&status);
    if (r_val3 < 0)
    { //반환 값이 0보다 작으면 pthread_join 오류
        perror("pthread_join() error\n");
        exit(0);
    }

    pthread_mutex_destroy(&mid);

    return 0;
}
