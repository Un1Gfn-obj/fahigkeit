// gcc -std=gnu11 -Wall -Wextra -D _GNU_SOURCE -o non_divergent_timer.{out,c} && ./non_divergent_timer.out

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h> // usleep()
#include <sys/time.h> // struct timeval (us) // struct timespec (ns)

/*
#include <time.h> // clock_t
// <sys/time.h> timercmp timeradd timersub
// available for timeval (us)only
// not available for timespec (ns)
__attribute__((deprecated)) static void clock_gettime2(){
  const clock_t clockid=CLOCK_REALTIME;
  struct timespec ts={};
  struct timeval tv={};
  assert(0==clock_getres(clockid,&ts));
  assert(0==ts.tv_sec); // long
  assert(1==ts.tv_nsec); // long
  assert(0==clock_gettime(clockid,&ts));
  printf("%lds ",ts.tv_sec);
  printf("%ldns ",ts.tv_nsec);
  printf("-> ");
  TIMESPEC_TO_TIMEVAL(&tv,&ts);
  printf("%lds ",tv.tv_sec);
  printf("%ldus ",tv.tv_usec);
  puts("");
}
*/

static void timeval_show(struct timeval *tv){
  if(tv==NULL){
    struct timeval tv2={};
    assert(0==gettimeofday(&tv2,NULL));
    timeval_show(&tv2);
  }else{
    printf("%lds ",tv->tv_sec);
    printf("%ldus ",tv->tv_usec);
    puts("");
  }
}

int main(){

  // clock_gettime2();

  struct timeval until={};
  // assert(0==gettimeofday(&until,&((struct timezone){})));
  assert(0==gettimeofday(&until,NULL));
  timeval_show(&until);

  for(;;){
    timeradd(&until,(&(struct timeval){1,0}),&until); // Increment until by 1s
    struct timeval now={};
    assert(0==gettimeofday(&now,&((struct timezone){})));
    assert(timercmp(&until,&now,>));
    timersub(&until,&now,&now); // now <- until - now
    assert(now.tv_sec==0);
    usleep(now.tv_usec);
    timeval_show(NULL); // (tv_usec-until.tv_usec) should not diverge
    puts("+1s");
  }

  return 0;

}