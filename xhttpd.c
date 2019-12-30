#include "xhttpd.h"

int main(int argc,char *argv[])
{
	char line[N*2],method[N*2],path[N*2],protocol[N*2],idx[N*4],location[N*4];
	char *file;
	size_t len;
	int ich,i,n;
	struct stat sb;
	FILE *fp;
	struct dirent **dl;

	if(argc !=2)
		send_error(500,"Internal Error",NULL,"Config error - no dir specified.");

	if(chdir(argv[1])<0)
		send_error(500,"Internal Error",NULL,"config error - couldn't chdir.");

	if(fgets(line,sizeof(line),stdin)==NULL)
		send_error(400,"Bad Request",NULL,"No request found.");

	if(sscanf(line,"%[^ ] %[^ ] %[^ ]",method,path,protocol)!=3)
		send_error(400,"Bad Request",NULL,"can't parse request.");

	while(fgets(line,sizeof(line),stdin)!=NULL)
	{
		if(strcmp(line,"\n")==0 || strcmp(line,"\r\n")==0)
			break;
	}

	if(strcasecmp(method,"GET")!=0)
		send_error(501,"Not Implemented",NULL,"That method is not implemented.");

	if(path[0]!='/')
		send_error(400,"Bad Request",NULL,"Bad filename.");

	file = &(path[1]);

	strdecode(file,file);

	if(file[0]=='\0')
		file = "./";

	len = strlen(file);
	if(file[0]=='/' || strcmp(file,"..")==0
									|| strncmp(file,"../",3)==0
									|| strstr(file,"/../")!=NULL
									|| strcmp(&(file[len-3]),"/..")==0)
	{
		send_error(400,"Bad Request",(char*)0,"Illegal filename.");
	}

	if(stat(file,&sb)<0)
		send_error(404,"Not Found",(char*)0,"File not found.");

	if(S_ISDIR(sb.st_mode)){
		if(file[len-1]!='/'){
			snprintf(location,sizeof(location),"Location:%s/",path);
			send_error(302,"Found",location,"Dirctories must end with a slash");
		}

		send_headers(200,"OK",NULL,"text/html",-1,sb.st_mtime);

		printf("<html><head><title>Index of %s</title></head>\n<body bgcolor=\"#99cc99\"><h4>Index of %s</h4>\n<pre>\n",file,file);

		n = scandir(file,&dl,NULL,alphasort);
		if(n<0)
			perror("scandir");
		else
			for(i=0;i<n;++i)
				file_infos(file,dl[i]->d_name);
		printf("</pre>\n<hr>\n<address><a href=\"%s\">%s</a></address>\n</body></html>\n",SERVER_URL,SERVER_NAME);
	}else{
		fp = fopen(file,"r");
		if(fp==(FILE*)0)
			send_error(403,"Forbidden",(char*)0,"File is protected.");

		send_headers(200,"OK",(char*)0,get_mine_type(file),sb.st_size,sb.st_mtime);

		while((ich = getc(fp))!=EOF)
			putchar(ich);
	}
	fflush(stdout);
	exit(0);
}
