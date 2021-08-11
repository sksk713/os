- Process_command가 실행이 되면 parse_command에서 인자들을 parsing한 후에 토큰의
개수와 인자들을 run_command로 넘긴다 run_command에서 change directory, history
외의 command를 실행하기 위해 fork()를 이용해 부모 자식으로 나눈다. 그 후에 기본
디폴트 path가 /bin인 execvp() 함수를 이용해 다른 command등을 실행
- 일단 가장 먼저 History를 LIST_HEAD 함수로 초기화를 하고난 후에, 입력 Command는
PA0에서 사용했던 list_add_tail 함수를 이용해 History에 Push, list_for_each 함수로
history를 처음부터 탐색하면서 처리된 Command를 list_entry 함수를 이용해서 값을 불러
왔다. 하지만 testcase인 ls subir, !8: !9 등이 문자열을 할당을 제대로 못해서 출력이
되지 않았다.

Homework fail