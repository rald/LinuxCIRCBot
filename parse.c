#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void tokenize_add(char *t,char ***tok,int *n) {
		*tok=realloc(*tok,sizeof(char*)*((*n)+1));
		(*tok)[(*n)++]=strdup(t);
}


int tokenize(char *str,char *del,char ***tok){

	int n=0;
	char *s=strdup(str);
	char *t=NULL; 
	char *p=NULL;

	t=s;
	p=strstr(t,del);
	while(p) {
		*p='\0';
		tokenize_add(t,tok,&n);
		t=p+strlen(del);
		p=strstr(t,del);
	}
	if(*t) tokenize_add(t,tok,&n);

	free(s);

	return n;

}



void tokenize_free(char ***tok,int *n) {
	for(int i=0;i<(*n);i++) {
		free((*tok)[i]);
		(*tok)[i]=NULL;
	}
	(*n)=0;
	free((*tok));
	(*tok)=NULL;
}



int parse(char *m,char ***t) {

	int n=0;

	char **a=NULL;
	int an=0;

	char **b=NULL;
	int bn=0;

	an=tokenize(m," :",&a);

	bn=tokenize(a[0]," ",&b);

	for(int j=0;j<bn;j++) {
		tokenize_add(b[j],t,&n);
	}

	tokenize_free(&b,&bn);

	for(int j=1;j<an;j++) {
		tokenize_add(a[j],t,&n);
	}

	tokenize_free(&a,&an);

	return n;
}



int main() {

	char *msg[]={
		"irc.undernet.org 000 PRIVMSG :hello world http://hello.io",
		NULL
	};

	char **tokens=NULL;
	int numTokens=0;

	for(int i=0;msg[i];i++) {

		numTokens=parse(msg[i],&tokens);

		for(int j=0;j<numTokens;j++) {
			printf("%s\n",tokens[j]);
		}
		printf("\n");

		tokenize_free(&tokens,&numTokens);

	}

}
