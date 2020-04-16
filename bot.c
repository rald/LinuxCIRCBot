#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include <string.h>
#include <time.h>

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>



#define HOST "irc.undernet.org"
#define PORT "6667"
#define PASS NULL
#define NICK "siesto"
#define CHAN "#gametime"


#define CHUNK_MAX 512



void print(char *fmt,...) {

	char *output=NULL;

	va_list args;

	va_start(args,fmt);
	vasprintf(&output,fmt,args);
	va_end(args);

	for(int i=0;i<strlen(output);i++) {
		switch(output[i]) {
			case '\r': printf("\\r"); break;
			case '\n': printf("\\n"); break;
			default: printf("%c",output[i]);
		}
	}

	printf("\n\n");

}



char *strsub(char *s,int b,int e) {
	int n=e-b;
	char *c=calloc(n+1,sizeof(char));
	strncpy(c,s+b,n);
	return c;
}



int IRC_Connect(char *host,char *port) {

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return -1;
	}

	freeaddrinfo(servinfo);

	return sockfd;

}



int IRC_Send(int sockfd, char *fmt, ...) {

	int retVal;

	char *output = NULL;

	va_list args;
	va_start(args, fmt);
	vasprintf(&output, fmt, args);
	va_end(args);

	printf("<-- %s",output);

	retVal=send(sockfd, output, strlen(output), 0);

	if(retVal == -1) {
		perror("send");
	}

	return retVal;

}



int IRC_Recv(int sockfd,char **buf,int *buflen) {

	printf("--- recv ---\n\n");

	char chunk[CHUNK_MAX]="";
	int chunklen = 0;

	if ((chunklen = recv(sockfd, chunk, CHUNK_MAX-1, 0)) <= 0) {
	  return -1;
	}
	chunk[chunklen]='\0';

	print("<chunk>%s</chunk>",chunk);

	(*buflen)+=chunklen;
	(*buf)=realloc(*buf,(*buflen)+1);
	strncat((*buf),chunk,chunklen);
	(*buf)[*buflen]='\0';

	print("<buf buflen=\"%d\">%s</buf>",*buflen,*buf);

	return chunklen;

}



void tokenize_add(char *t,char ***tok,int *n) {
		*tok=realloc(*tok,sizeof(char*)*((*n)+1));
		(*tok)[(*n)++]=strdup(t);
}



int tokenize(char *str,char *del,char ***tok,int tot){

	int n=0;
	char *s=strdup(str);
	char *t=NULL; 
	char *p=NULL;
	int c=0;

	t=s;
	p=strstr(t,del);
	while(p && (tot==0 || c<tot)) {
		c++;
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



int tokenize_message(char *m,char ***t) {

	int n=0;

	char **a=NULL;
	int an=0;

	char **b=NULL;
	int bn=0;

	if(m[0]==':') m++;

	an=tokenize(m," :",&a,1);

	bn=tokenize(a[0]," ",&b,0);

	for(int j=0;j<bn;j++) {
		if(strlen(b[j])) tokenize_add(b[j],t,&n);
	}

	tokenize_free(&b,&bn);

	for(int j=1;j<an;j++) {
		if(strlen(a[j])) tokenize_add(a[j],t,&n);
	}

	tokenize_free(&a,&an);

	return n;
}



void parse(int sockfd,char **tokens,int numTokens) {

	if(!strcmp(tokens[0],"PING")) {

		IRC_Send(sockfd,"PONG %s\r\n",tokens[1]);

	} else if(!strcmp(tokens[1],"001")) {

		IRC_Send(sockfd,"JOIN %s\r\n",CHAN);

	} else if(!strcmp(tokens[1],"PRIVMSG")) {

		char *from=strsub(tokens[0],0,strchr(tokens[0],'!')-tokens[0]);
		char *to=strdup(tokens[2]);

		printf("from: '%s' to: '%s'\n\n",from,to);

		free(to);
		free(from);

	}

}



int main(void) {

	int sockfd;

	char *buf = NULL;
	int buflen = 0;

	char *msg = NULL;
	int msglen = 0;

	char *pos = NULL;

	int retval;

	char **tokens=NULL;
	int numTokens=0;

	

	printf("Connecting... ");

	sockfd = IRC_Connect(HOST, PORT);

	printf("OK\n");

	if(PASS) IRC_Send(sockfd,"PASS %s\r\n",PASS);
	IRC_Send(sockfd,"USER %s 0 0 :%s\r\n",NICK,NICK);
	IRC_Send(sockfd,"NICK %s\r\n",NICK);

	for(;;) {	

		retval=IRC_Recv(sockfd,&buf,&buflen);
	
		if(retval<0) break;

		if(buf && *buf) {

			while((pos=strstr(buf,"\r\n"))) {
		
				msglen=pos-buf;
				msg=realloc(msg,msglen+1);
				strncpy(msg,buf,msglen);
				msg[msglen]='\0';
				
				print("<msg>%s</msg>",msg);



				numTokens=tokenize_message(msg,&tokens);

				printf("<token>\n");
				
				for(int j=0;j<numTokens;j++) {
					printf("%d: '%s'\n",j,tokens[j]);
				}
				
				printf("</token>\n\n");

				parse(sockfd,tokens,numTokens);

				tokenize_free(&tokens,&numTokens);



				buflen-=msglen+2;		
				strncpy(buf,buf+msglen+2,buflen);
				buf=realloc(buf,buflen+1);
				buf[buflen]='\0';

				print("<buf buflen=\"%d\">%s</buf>",buflen,buf);

				free(msg);
				msg=NULL;
				msglen=0;
							
			}

		}

	}

	free(buf);
	buf = NULL;
	buflen = 0;

	return 0;

}
