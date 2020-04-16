#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

int main(int argc,char **argv) {
	CURL *curl;
	FILE *fout;
	int result;
	char path[PATH_MAX]="./downloads/";

	curl=curl_easy_init();

	strcat(path,argv[2]);
	
	fout=fopen(path,"wb");

	curl_easy_setopt(curl,CURLOPT_URL,argv[1]);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,fout);
	curl_easy_setopt(curl,CURLOPT_FAILONERROR,1L);
	curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
	curl_easy_setopt(curl,CURLOPT_IPRESOLVE,CURL_IPRESOLVE_V4);

	if((result=curl_easy_perform(curl))==CURLE_OK) {
		printf("OK: Download Success\n");
	} else {
		printf("ERROR: %s\n",curl_easy_strerror(result));
	}

	fclose(fout);

	curl_easy_cleanup(curl);

	return 0;
}
