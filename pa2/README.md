- Sjf_schedule
sjf스케쥴러는 lifespan을 기준으로 가장 짧은 거부터 처리해야 한다. 맨 처음 process의
lifespan을 pivot으로 정했다. 그 다음 readyqueue를 돌면서 pivot보다 작으면 pivot을 지속적
으로 바꿔주고 다음 실행해야 할 next 프로세스를 바꿔주었다.
- Srtf_schedule
srtf스케쥴러는 (lifespan – age)을 기준으로 해당 값이 가장 작은 거부터 preemptive하게 처리
해줘야 한다. sjf스케쥴러 구현할 때처럼 pivot을 기준으로 가장 짧은 거부터 시작되게 했다.
그 다음 current를 처리할 때 readyqueue를 다시 돌면서 가장 짧은 것이 있다면 preemptive
하게 처리하기 위해 pick_next로 가도록 했다.
- Rr_schedule
rr스케쥴러는 처음에는 조금 헷갈렸는데 설명을 읽다 보니 time quantum을 알아서 스케쥴함
수가 컨트롤 해준다고 해서 가장 첫번째 프로세스가 next로 선택되고 1tick동안 실행된 다음,
current를 다시 readyqueue 끝으로 보낸 후, 다시 pick_next로 가서 다음 프로세스를 정한다.
- Prio_schedule
prio스케쥴러는 release과정에서 waityqueue에서 가장 priority가 높은 queue를 보내야 하므
로 prio_release()를 구현했다 해당 함수에서는 waitqueue를 전부 탐색하면서 prio가 가장 큰
프로세스를 readyqueue로 보낸다.
- Pa_schedule
pa스케쥴러는 가장 prio가 큰 프로세스를 가져와야 하므로 prio_release()를 사용하고 프로세
스가 돌때마다 다른 프로세스들의 prio를 1씩 올려야 된다. 처음에 프로세스를 pick하고 난
후에 prio를 올리면 1씩 더 늦게 올라가서 current가 존재하지 않을 때, 모두 prio를 1씩 올려
주는 방법을 사용했다. 그리고 current의 prio를 prio_orig로 바꿔준 다음, readyqueue를 돌면
서 current외의 프로세스들이 prio가 MAX_PRIO 이하라면 1씩 더해주고 current_prio보다 크
거나 같다면 max값을 갱신해주었다. Readyqueue 탐색을 마치고 나와서 만약에 current_prio
가 max값보다 작거나 같다면 current를 readyqueue뒤로 보내고 pick_next로 가게 했다.
- Pcp_schedule
pcp스케쥴러는 acquire과정에서 r->owner->prio를 MAX_PRIO로 바꿔줘서 해당 리소스를 잡
는 동안 다른 프로세스가 해당 리소스를 점유할 수 없게 했다. 그리고 release과정에서 null
되기전에 r->owner->prio를 prio_orig으로 다시 바꿔주었다. 그리고 prio스케쥴러를 사용하므
로 testcase에서 prio가 같은 경우 라운드로빈 방식으로 처리해주어야 하는데 해당 기능을 추
가하지 못해서 prio함수에 pick_next에서 next->age가 temp->age보다 크고 temp->prio가
pivot과 같은 경우 next를 temp로 바꿔 주었고, current->prio가 max값이라면 현재 돌고 있는
current가 prio가 가장 높은데 같은 prio를 가진 프로세스가 존재한다는 말이므로 해당
current를 readyqueue로 보냈다.
- Pip_schedule
pip스케쥴러는 acquire과정에서 수업시간에 배운 대로 r->owner->prio의 값을 current->prio
로 바꿔줘서 다른 프로세스에게 리소스를 뺏기지 않도록 한다. 그 외에는 pcp스케쥴러와 동일하다.
