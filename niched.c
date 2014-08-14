#define MAX_POP 10000000
#define SZ 201   //was 1801
#define EVOMUT 0.01 //was 0.05
#define NICHE_SIZE 5 //was 6
int EXT_FREQ=100;
float EXT_SURVIVAL=0.01;
int seed=0;

//#define RENDER 

#ifdef RENDER
#include "render.h"
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <vector>

int niches_discovered=0;
int niche_orderx[SZ*SZ];
int niche_ordery[SZ*SZ];
int niche_found[SZ][SZ];

bool do_extinction=false;

using namespace std;

double r()
{
 return (double)rand()/(double)RAND_MAX;
}

typedef struct {
 int x;
 int y;
 int order;
 double evo;
 bool mutate;
} indiv;

bool ind_sort(indiv x,indiv y) {
 return x.order > y.order;
}

void reset_niche_found() {
 niches_discovered=0;
 for(int x=0;x<SZ;x++)
  for(int y=0;y<SZ;y++)
   niche_found[x][y]=0; 
}

void init_indiv(indiv* i) {
 i->x=SZ/2;
 i->y=SZ/2;
 i->evo=0.01;
 i->mutate=false;
}

void register_indiv(indiv* i) {
 if(!niche_found[i->x][i->y]) {
  niche_found[i->x][i->y]=niches_discovered+1;
  niche_orderx[niches_discovered]=i->x;
  niche_ordery[niches_discovered]=i->y;
  niches_discovered++;
 }
}

void copy_indiv(indiv* from, indiv* to) {
 to->x = from->x;
 to->y = from->y;
 to->evo = from->evo;
 to->mutate=false;
}

void mutate_indiv(indiv* x) {
 x->mutate=false;
 if (r()<x->evo) {
  x->mutate=true;
  int inc=1;

  if (r()<0.5)
   inc = -1;

  if (r()<0.5) {
   x->x+=inc;
   //if(x->x<0) x->x=0;
   //else if(x->x>=SZ) x->x=SZ-1;
   if(x->x<0) x->x=SZ-1;
   else if(x->x>=SZ) x->x=0;
  }
  else {
   x->y+=inc;
   //if(x->y<0) x->y=0;
   //else if(x->y>=SZ) x->y=SZ-1;
   if(x->y<0) x->y=SZ-1;
   else if(x->y>=SZ) x->y=0;
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
 int order[MAX_POP];
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
 register_indiv(& p->i[0]);
}

void survive(indiv* c, pop* newp) {
 if (newp->niche_cnt[c->x][c->y]<NICHE_SIZE) 
  if(newp->psize < MAX_POP) {
   newp->niche_cnt[c->x][c->y]++;
   newp->niches[c->x][c->y]+=c->evo;
   copy_indiv(c,&newp->i[newp->psize]);
   newp->psize++;
  }
  register_indiv(c);
}

double dist(int x1,int y1,int x2,int y2) {

 if(x2<x1)
 {
 int t=x2;
 x2=x1;
 x1=t;
 }

 if(y2<y1)
 {
 int t=y2;
 y2=y1;
 y1=t;
 }

 int xdist1 = x2-x1;
 int xdist2 = x1+(SZ-x2);
 int ydist1 = y2-y1;
 int ydist2 = y1+(SZ-y2);
 
 //wrap-around
 if(xdist2<xdist1)
  xdist1=xdist2;
 if(ydist2<ydist1)
  ydist1=ydist2;

 return sqrt(xdist1*xdist1+ydist1*ydist1);
}


void survive_ex_list(indiv* c, pop* newp,vector<int>& survival) {
 bool die=true;
 int surv_size=survival.size();

 for(int i=0;i<surv_size;i++)
 {
   int j = survival[i];
   if(niche_orderx[j]==c->x && niche_ordery[j]==c->y)
    die=false;
 }

 if(!die)
 if(newp->niche_cnt[c->x][c->y]<NICHE_SIZE) 
  if(newp->psize < MAX_POP) {
   newp->niche_cnt[c->x][c->y]++;
   newp->niches[c->x][c->y]+=c->evo;
   copy_indiv(c,& newp->i[newp->psize]);
   newp->psize++;
  }
}

void survive_ex(indiv* c, pop* newp,int ex,int ey,double rad) {
 if(dist(c->x,c->y,ex,ey)>rad)
 if(newp->niche_cnt[c->x][c->y]<NICHE_SIZE) 
  if(newp->psize < MAX_POP) {
   newp->niche_cnt[c->x][c->y]++;
   newp->niches[c->x][c->y]+=c->evo;
   copy_indiv(c,& newp->i[newp->psize]);
   newp->psize++;
  }
}

void make_new_pop(pop* newp, pop* old) {

 vector<indiv> inside;
 vector<indiv> outside;

 reset_pop(newp);
  
 for(int i=0;i<old->psize;i++) { 
  old->order[i]=i;
 }
  //sort(old->i,old->i+old->psize,ind_sort);

  random_shuffle(&old->order[0],&old->order[old->psize-1]);
  for(int j=0;j<old->psize;j++) {
   int i=old->order[j];
   indiv c;

  for(int z=0;z<2;z++) {
  copy_indiv(&old->i[i],&c);
  mutate_indiv(&c);
  if(c.mutate)
      outside.push_back(c);
  else
      inside.push_back(c);
  }
  }
  for(int i=0;i<inside.size();i++)
    survive(&inside[i],newp);
  for(int i=0;i<outside.size();i++)
    survive(&outside[i],newp);
  //survive(&c,newp);
 
 /*

 for(int z=0;z<2;z++)
 {
  for(int j=0;j<old->psize;j++) {
   int i=old->order[j];
   indiv c;

  copy_indiv(&old->i[i],&c);
  mutate_indiv(&c);
  survive(&c,newp);

 }
 }
 */
 
/*
  copy_indiv(&old->i[i],&c);
  mutate_indiv(&c);
  survive(&c,newp);
 }
*/

}

void extinction(pop* newp, pop* old) {
 int loc_x=r()*SZ;
 int loc_y=r()*SZ;
 reset_pop(newp);

 vector<int> survival_list;

 for(int i=0;i<niches_discovered;i++) survival_list.push_back(i);

 random_shuffle(survival_list.begin(),survival_list.end());

 int surv = EXT_SURVIVAL; //((float)niches_discovered*EXT_SURVIVAL);
 if(surv<1)
  surv=1;

 survival_list.erase(survival_list.begin()+surv,survival_list.end());
 
 for(int i=0;i<old->psize;i++) {
  indiv c;
  copy_indiv(&old->i[i],&c);
  //survive_ex(&c,newp,loc_x,loc_y,SZ/3);
  survive_ex_list(&c,newp,survival_list);
 }

 reset_niche_found();
 for(int i=0;i<newp->psize;i++)
  register_indiv(&newp->i[i]);
}


double pop_min(pop* p) {
 double evosum=10000.0;
 for(int i=0;i<p->psize;i++)
  if(p->i[i].evo<evosum)
   evosum=p->i[i].evo;
 return evosum;
}

double pop_max(pop* p) {
 double evosum=0.0;
 for(int i=0;i<p->psize;i++)
  if(p->i[i].evo>evosum)
   evosum=p->i[i].evo;
 return evosum;
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

ofstream* log_1;
ofstream* log_2;
pop pop1;
pop pop2;
pop* cur_pop= &pop1;
pop* new_pop= &pop2;
int gen=0;

void do_one_gen() {
  if(gen%EXT_FREQ==0)
   (*log_1) << gen << " " << cur_pop->psize << " " << pop_niches(cur_pop) << " " << pop_avg(cur_pop) << endl;

  if(gen>0 && (gen+1)%EXT_FREQ==0 && do_extinction) {
   extinction(new_pop,cur_pop);
   //make_new_pop(new_pop,cur_pop);
  } else {
   make_new_pop(new_pop,cur_pop);
  }

  pop* t=new_pop;
  new_pop=cur_pop;
  cur_pop=t;
  gen++;
}

int main(int argc, char** argv) {
 srand(time(NULL)); 
 init_pop(cur_pop);
 log_1= new ofstream(argv[1]);
 log_2= new ofstream(argv[2]);
 do_extinction = argv[3][0]=='e';
 EXT_FREQ = atoi(argv[4]); 
 EXT_SURVIVAL = atof(argv[5]);
 seed= atoi(argv[6]);
 srand(seed);

 cout << "do_extinction: " << do_extinction << endl;
 cout << "EXT_FREQ: " << EXT_FREQ << endl;
 cout << "EXT_SURV: " << EXT_SURVIVAL << endl; 
 #ifndef RENDER
 while(gen<10001) { 
  if(gen%1000==0)
   cout << gen << endl;
  do_one_gen();
 }
 int sample_factor=100; 
 for(int i=0;i<cur_pop->psize;i+=sample_factor) {
  (*log_2) << cur_pop->i[i].x << " " << cur_pop->i[i].y << " " << cur_pop->i[i].evo << endl;
 }
 #else

	// initialize and run program
	glutInit(&argc, argv);                                      // GLUT initialization
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );  // Display Mode
	glutInitWindowSize(640,480);					// set window size
	glutCreateWindow("display");								// create Window
//	glutFullScreen();
	glutDisplayFunc(display);									// register Display Function
	glutIdleFunc( display );									// register Idle Function
       glutKeyboardFunc( keyboard );								// register Keyboard Handler
	initialize();
	glutMainLoop();												// run GLUT mainloo
 #endif

 delete(log_1);
 delete(log_2);
 return 0;
}

#ifdef RENDER

void display() 
{
        double avg_evo=pop_avg(cur_pop);
        double max_evo=pop_max(cur_pop);
        double min_evo=pop_min(cur_pop);
        if(gen%100==0)
	cout << gen << " " << avg_evo << " " << niches_discovered << endl;
	glClearColor(0.2f,0.0,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		     // Clear Screen and Depth Buffer
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-3.0f);			
 
	/*
	 * Triangle code starts here
	 * 3 verteces, 3 colors.
	 */
	glBegin(GL_QUADS);					
        float szf = (float)SZ;
        float sz= 2.0f/szf;
        for(int i=0;i<cur_pop->psize;i++) {
         int x=cur_pop->i[i].x;
         int y=cur_pop->i[i].y;
         float evo= cur_pop->i[i].evo;
 	 float dx = 2.0* (((float)x)/szf-0.5);
         float dy=  2.0* (((float)y)/szf-0.5);
		/*
                if (evo>avg_evo)
		 glColor4f(1.0f,0.0f,0.0f,0.2);			
                else
		 glColor4f(1.0f,1.0f,1.0f,0.2);			
		*/
		evo= (evo-min_evo)/(max_evo-min_evo);
		float col=evo;
		glColor4f(col,col,col,0.3f);
		glVertex3f( dx, dy, 0.0f);		
		glVertex3f( dx+sz,dy, 0.0f);		
		glVertex3f( dx+sz,dy+sz, 0.0f);		
		glVertex3f( dx,dy+sz, 0.0f);		
        }
	glEnd();				
 
	glutSwapBuffers();
	for(int i=0;i<5;i++)
         do_one_gen();
}
#endif
