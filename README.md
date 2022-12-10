# 임베디드시스템 프로젝트 02분반 6조
Kotlin + C -> 임베디드 시스템과 블루투스 통신 앱 개발
## 프로젝트 소개
임베디드 시스템 수업을 통해서 배운 블루투스 모듈, 초음파 센서, 수동 부저를 이용한 출입경보 및 알림 시스템 구현
## 개발기간
- 22.12.2일 ~ 12.15일
## 구성인원
팀장 : 박종현 
팀원 : 한성현  
팀원 : 김성태
## 개발환경
- 임베디드 시스템 개발: raspberryPi 4.0의 C18
- 앱 개발: Android Studio 의 Kotlin
## 전체 시스템 구조
![전체시스템구조](https://user-images.githubusercontent.com/93969485/206841072-257098f4-40a0-4da2-bbe1-973b93f6cdda.png)
## 주요 기능
초음파 센서를 활용하여 주변 물체와의 거리를 지속 검사 

 👉 일정 거리 이내에 장애물이 존재하는 상황이 2초이상 지속될 경우 블루투스 모듈을 통해 사용자의 앱에 메세지를 전달   

동시에 수동 부저를 통해 경보 발생
## 제한조건 구현 내용
모듈들을 각각의 쓰레드로 나누어 독립적으로 작동하도록 구현 👉 멀티쓰레딩 조건 만족

초음파 센서 스레드 내에 정확히 2초간 가까운 물체를 감지했을때, 신호를 보내기 위한 변수 “count” 를 제어하기 위한 mutex함수를 사용
## 가산점 요소
블루투스 모듈과 통신하기 위한 안드로이드 전용 앱 개발 (라즈베리파이 - 스마트폰) 
## 개발 시 문제점 및 해결방안
 문제점: 초음파 센서가 읽어들이는 값이 한번씩 오차범위를 크게 벗어나는 경우 발생 

해결방법: 물체가 지정한 위험거리 안에 들었을 때 곧바로 사용자에게 알림을 보내지 않고 
          2초이상 거리를 유지했을 때 사용자에게 메시지를 보내도록 함 

문제점: 수동 부저의 펄스폭 변조 방법을 이용한 소리 크기 변경의 효과가 크지 않았음

해결방법: 소리의 주파수를 조절하여 물체와의 거리가 가까울수록 높은 주파수의 음을 출력하도록 구현
## 데모
- https://youtu.be/hf7Y9lKc_uUhttps://youtu.be/hf7Y9lKc_uU

