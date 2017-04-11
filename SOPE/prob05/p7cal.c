// PROGRAMA p07 server  NOT WORKING
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

void calculate(int x, int y, int f){
  Calq final;
  final.soma = x + y;
  final.sub = x - y;
  final.mult = x*y;
  if(y == 0){
    final.d[0] = 'x';
    final.div = 0;
  } else {
    final.d[0] = ' ';
    final.div = (double)x/y;
  }
  write(f, &final, sizeof(final));
}

int main(void){
  Numbers num;

  int calq = 1;
  int fifo_req, fifo_ans;
  mkfifo("/tmp/myfifo_req",0660);
  fifo_req=open("/tmp/myfifo_req",O_RDONLY);
  mkfifo("/tmp/myfifo_ans",0660);
  fifo_ans=open("/tmp/myfifo_ans",O_WRONLY);

  do{
    read(fifo_req, &num, sizeof(num));
    if(num.x == 0 && num.y == 0)
      calq = 0;
    else
      calculate(num.x,num.y, fifo_ans);
  } while(calq);


  close(fifo_req);
  close(fifo_ans);
  return 0;
}
