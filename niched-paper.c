#define MAX_POP 10000000
#define SZ 1801
#define EVOMUT 0.01 //was 0.05
#define NICHE_SIZE 5

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
using namespace std;
double r()
{
 return (double)rand()/(double)RAND_MAX;
}

typedef struct {
 int x;
 int y;
 double evo;
} indiv;

void init_indiv(indiv* i) {
 i->x=SZ/2;
 i->y=SZ/2;
 i->evo=0.05;
}

void copy_indiv(indiv* from, indiv* to) {
 to->x = from->x;
 to->y = from->y;
 to->evo = from->evo;
}

void mutate_indiv(indiv* x) {
 if (r()<x->evo) {
  int inc=1;

  if (r()<0.5)
   inc = -1;

  if (r()<0.5) {
   x->x+=inc;
   if(x->x<0) x->x=0;
   else if(x->x>=SZ) x->x=SZ-1;
  }
  else {
   if(x->y<0) x->y=0;
   else if(x->y>=SZ) x->y=SZ-1;
   x->y+=inc;
  }
 }
 
 if(r()<EVOMUT) {
  x->evo+= r()*0.01-0.005;
  if(x->evo<0.0) x->evo=0.0;
 }
}


typedef struct {
 int psize;
 indiv i[MAX_POP];
 int niche_cnt[SZ][SZ];
 double niches[SZ][SZ];
} pop;


void reset_pop(pop* p) {
 p->psize=0;
 for (int x=0;x<SZ;x++)
  for (int y=0;y<SZ;y++) {
   p->niche_cnt[x][y]=0;
   p->niches[x][y]=0.0;
  }
}

void init_pop(pop* p) {
 reset_pop(p);
 p->psize=1;
 init_indiv(& p->i[0]);
}

pop pop1;
pop pop2;
pop* cur_pop= &pop1;
pop* new_pop= &pop2;

void survive(indiv* c, pop* newp) {
 if (newp->niche_cnt[c->x][c->y]<NICHE_SIZE) 
  if(newp->psize < MAX_POP) {
   newp->niche_cnt[c->x][c->y]++;
   newp->niches[c->x][c->y]+=c->evo;
   copy_indiv(c,& newp->i[newp->psize]);
   newp->psize++;
  }
}

void make_new_pop(pop* newp, pop* old) {
 reset_pop(newp);
 for(int i=0;i<old->psize;i++) {
  indiv c;

  copy_indiv(&old->i[i],&c);
  mutate_indiv(&c);
  survive(&c,newp);

  copy_indiv(&old->i[i],&c);
  mutate_indiv(&c);
  survive(&c,newp);
 }

}

double pop_avg(pop* p) {
 double evosum=0.0;
 for(int i=0;i<p->psize;i++)
  evosum+=p->i[i].evo;
 return evosum/p->psize;
}

int pop_niches(pop* p) {
 int cnt=0;
 for(int x=0;x<SZ;x++)
  for(int y=0;y<SZ;y++)
   if(p->niche_cnt[x][y]>0)
    cnt++;
 return cnt;
}

int main(int argc, char** argv) {
 srand(time(NULL)); 
 init_pop(cur_pop);
 ofstream log1(argv[1]);
 ofstream log2(argv[2]);
 for(int i=0;i<3001;i++) {
  if(i%100==0)
   log1 << i << " " << cur_pop->psize << " " << pop_niches(cur_pop) << " " << pop_avg(cur_pop) << endl;
  make_new_pop(new_pop,cur_pop);
  
  pop* t=new_pop;
  new_pop=cur_pop;
  cur_pop=t;
 }

int sample_factor=100; 
 for(int i=0;i<cur_pop->psize;i+=sample_factor) {
  log2 << cur_pop->i[i].x << " " << cur_pop->i[i].y << " " << cur_pop->i[i].evo << endl;
 }

 return 0;
}

