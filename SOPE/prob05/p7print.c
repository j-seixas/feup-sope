// PROGRAMA p07 client NOT WORKING
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>


typedef struct{
  int soma, sub, mult;
  double div;
  char d[2];
} Calq;

typedef struct{
  int x, y;
} Numbers;

int main(void){
  Numbers num;
  Calq final;
  int fifo_req, fifo_ans;
  char message[100];
  do {
    fifo_req=open("/tmp/myfifo_req",O_WRONLY);
    if (fifo_req==-1) sleep(1);
  } while (fifo_req==-1);
  do {
    fifo_ans=open("/tmp/myfifo_ans",O_RDONLY);
    if (fifo_ans==-1) sleep(1);
  } while (fifo_ans==-1);

  fgets(message, 100, stdin);
  sscanf(message, "%d %d", &num.x, &num.y);
  write(fifo_req, &num, sizeof(num));
  close(fifo_req);

  read(fifo_ans, &final, sizeof(final));
  printf("Soma: %d \nSubtracao: %d \nMultiplicacao: %d\n", final.soma, final.sub, final.mult);
  if(final.d[0] == 'x'){
    printf("Divisao Impossivel\n");
  } else {
    printf("Divisao: %f \n", final.div);
  }
  close(fifo_ans);
  return 0;
}
