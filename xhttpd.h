#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<dirent.h>
#include<time.h>
#include<ctype.h>
#define N 4096 
#define SERVER_NAME "xhttpd"
#define SERVER_URL "http://127.0.0.1:5998/"
#define PROTOCOL "HTTP/1.1"
#define FORMAT_DATE "%A,%d %b %Y %H:%M:%S GMT"

static void send_error(int status,char *title,char *extra_header,char *text);
static void file_infos(char *dir,char *name);
static char *get_mine_type(char *name);
static int hexit(char c);
static void strencode(char *to,size_t tosize,const char *from);
static void strdecode(char *to,char *from);





static void send_headers(int status,char *title,char *extra_header,char *mine_type,off_t length,time_t mod)
{
	time_t now;
	char timebuf[100];

	printf("%s %d %s\r\n",PROTOCOL,status,title);
	printf("Server:%s\r\n",SERVER_NAME);
	now = time((time_t*)0);
	strftime(timebuf,sizeof(timebuf),FORMAT_DATE,gmtime(&now));
	printf("Date:%s\r\n",timebuf);

	if(extra_header !=NULL)
		printf("%s\r\n",extra_header);

	if(mine_type!=NULL)
		printf("Content-Type:%s\r\n",mine_type);

	if(length >=0)
		printf("Content-Length:%ld\r\n",(int64_t)length);

	if(mod !=(time_t)-1){
		strftime(timebuf,sizeof(timebuf),FORMAT_DATE,gmtime(&mod));
		printf("Last-Modified:%s\r\n",timebuf);
	}

	printf("Connection:close\r\n");
	printf("\r\n");

}
static void send_error(int status,char *title,char *extra_header,char *text)
{
	send_headers(status,title,extra_header,"text/html",-1,-1);
	printf("<html><head><title>%d %s</title></head>\n<body bgcolor=\"#cc9999\"><h4>%d %s</h4>\n",
			status,title,status,title);
	printf("%s\n",text);
	printf("<hr>\n<address><a href=\"%s\">%s</a></address>\n</body></html>\n",
			SERVER_URL,SERVER_NAME);
	fflush(stdout);

	exit(1);

}
static void file_infos(char *dir,char *name)
{
	static char encoded_name[N];
	static char path[N];
	char timestr[16];
	struct stat sb;

	strencode(encoded_name,sizeof(encoded_name),name);
	snprintf(path,sizeof(path),"%s/%s",dir,name);

	if(lstat(path,&sb)<0)
		printf("<a href=\"%s\">%-32.32s</a>\n",encoded_name,name);
	else{
		strftime(timestr,sizeof(timestr),"%d%b%Y %H:%M",localtime(&sb.st_mtime));
		printf("<a href=\"%s\">%-32.32s</a>   %15s %14ld\n",encoded_name,name,timestr,(int64_t)sb.st_size);
	}

}

static char *get_mine_type(char *name)
{
	char *dot;
	dot = strrchr(name,'.');

	if(dot == (char*)0)
		return "text/plain;charset=iso-8859-1";
	if(strcmp(dot,".html")==0 || strcmp(dot,".htm")==0)
		return "text/html;charset=iso-8859-1";
	if(strcmp(dot,".jpg")==0 || strcmp(dot,".jpeg")==0)
		return "image/jpeg";
	if(strcmp(dot,".gif")==0)
		return "image/gif";
	if(strcmp(dot,".png")==0)
		return "image/png";
	if(strcmp(dot,".css")==0)
		return "text/css";
	if(strcmp(dot,".au")==0)
		return "audio/basic";
	if(strcmp(dot,".wav")==0)
		return "audio/wav";
	if(strcmp(dot,".avi")==0)
		return "video/x-msvideo";
	if(strcmp(dot,".mp3")==0)
		return "audio/mpeg";
	if(strcmp(dot,".mp4")==0)
		return "video/mp4";

	return "text/plain;charset=iso-8859-1";

}


static int hexit(char c)
{
	if(c>='0' && c<='9')
		return c-'0';
	if(c>='a' && c<='f')
		return c-'a'+10;
	if(c>='A' && c<='F')
		return c-'A'+10;

	return 0;
}

static void strencode(char *to,size_t tosize,const char *from)
{
	int tolen;
	for(tolen=0;*from!='\0' && tolen +4<tosize;++from){
		if(isalnum(*from) || strchr("/_.-~",*from)!=(char*)0){
			*to = *from;
			++to;
			++tolen;
		}else {
			sprintf(to,"%%%02x",(int)*from & 0xff);
			to +=3;
			tolen +=3;
		}
	}
	*to = '\0';

}

static void strdecode(char *to,char *from)
{
	for(;*from !='\0';++to,++from){
		if(from[0]=='%' && isxdigit(from[1]) && isxdigit(from[2])){

			*to = hexit(from[1])*16+hexit(from[2]);
			from +=2;
		}else
			*to = *from;
	}
	*to = '\0';

}
