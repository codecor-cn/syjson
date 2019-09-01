#include <stdio.h>
#include <stdlib.h>

struct node{
	int data;
	struct node *next;
};

typedef struct node * PtrToNode; 

PtrToNode revert(PtrToNode l1)
{
	PtrToNode L,head,first,second,tmp;
	L = (PtrToNode) malloc(sizeof(struct node));
	head = l1->next;
	first = head->next;
	second = first->next;
	while(second != NULL)
	{
		tmp=second->next;
		second->next = first;
		first = second;
		second = tmp;
	}
	L->next = second;

	return L;
}

PtrToNode init()
{
	PtrToNode L,s;	
	L = (PtrToNode)malloc(sizeof(struct node));
	L->next = NULL;
	int n;
	int d;
	scanf("%d",&n);
	for(int i=0;i<n;i++)
	{
		scanf("%d",&d);
		s = (PtrToNode)malloc(sizeof(struct node));
		//printf("%p",s);
		s->data=d;
		s->next = L->next;
		L->next = s;
		//s->next = NULL;
	}
	return L;
}

void Print(PtrToNode L)
{
	PtrToNode p;
	p = L->next;
	while(p != NULL)
	{
		printf("%d",p->data);
		p = p->next;
	} 
}
int main(){

	PtrToNode l1,l2;

	l1 = init();
	Print(l1);
	l2 = revert(l1);
	Print(l2);
	return 0;
}
