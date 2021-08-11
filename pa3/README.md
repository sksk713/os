- Alloc_page에서 NR_PTES_PER_PAGE가 16이므로 vpn을 16으로 나눈 것이 outer의 index, vpn을
16으로 나눈 나머지가 outer 하나에 연결된 page table index라고 할 수 있다. 할당을 하기전에
가장 작은 page frame number부터 채우라는 조건이 있기 때문에, 좀더 쉽게 구현이 가능했다.
mapcount를 NR_PAGEFRAMES만큼 체크를 하면서 할당이 안된 곳에서 break를 줘 차례대로 1씩
증가시킬 수 있도록 처리한다. 그 다음 current의 pagetable_outer_ptes에 아직 할당이 되어 있지
않으면, 할당을 해주고 해당 outer에 연결된 pagetable의 pt_idx의 valid를 true로 해주고 pfn의
값도 업데이트 해준다. 마지막으로 해당 입력의 rw를 체크해서 writable이 가능한지 판단한다.
- Free_page는 간단하게 outerpt_idx에 해당하는 pt_idx를 구하고 해당 pte의 valid를 false로
writable도 false로 바꾼 후, 할당을 푸는 것이므로 해당 pfn의 mapcounts를 1빼주면 된다.
- Handle_page_fault에서는 Switch_process에서 했던 private와 mapcount를 보고 판단한다. Switch에
서 private에 writable이 가능했던 pte를 false로 바꾸고 private를 RW_READ로 바꿔줬었던 걸 사
용해서 해당 pte에 write를 시도할 시, private가 RW_READ인 곳을 찾아서 mapcount가 2이상이면
1 감소시키고 해당 vpn을 다른 pte에 alloc_page하면 된다. 만약 1이면 write해도 상관이 없으므
로 해당 pte[pt_idx]의 writable을 RW_WRITE로 바꿔주기만 하면 된다.
- Switch_process는 switch가 발생했을 때 list를 탐색해 switch 하고자 하는 프로세스 pid가 존재하
는지 확인을 하고 있으면, 해당 프로세스로 변경하고 current는 프로세스 list에 추가하고 버리면
된다. 여기서 만약 탐색이 실패할 경우에는 새로운 process를 만들어야 한다. process하나를 선언
하고 메모리를 할당해준 다음, 이 process의 pid를 switch할 pid로 변경한다. 그 후에 이 child의
outer_pte도 새로 할당을 해주고, 해당 child outer_pte가 current outer_pte가 바라보는 pte를 동일
하게 보게 해줘야 한다. 그런 다음 해당 mapcounts[pt_idx]를 바라보는 프로세스가 하나 더 생기
기 때문에 1 증가시켜야 한다. 마지막으로 page table을 fork하고 나서 current outer_pte의
writable이 가능한 상태였으면 current, child 둘 모두 false로 변경해주어야 하고 해당 상태를 저장
하기 위해 private를 사용해 RW_READ를 따로 저장해주었다.