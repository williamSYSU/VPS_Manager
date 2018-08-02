#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cJSON.h"

#define PASS_LEN 8
#define PORT_MIN 50020	//端口下限值
#define PORT_MAX 60000	//端口上限值
#define AGENT_MIN 60020	//代理分配的端口下限
#define AGENT_MAX 70000	//代理分配的端口上限
#define EXPIRED_LIMIT 60 * 60 * 24	//设置过期期限

const char *server_ip = "162.243.59.69";
const char *encrypt_method = "chacha20";
const char *protocol = "orgin";
const char *obfuscation = "plain";

char *file_loc = "/etc/shadowsocks.json";
//char *file_loc = "/home/ghost/vps-server/json/shadowsocks.json";
cJSON *root;		//配置文件根指针
cJSON *port_passwd;	//配置文件port_passwd指针

/*doit
	功能:字符串转cJSON结构
	参数：
		text:转换字符串
*/
void doit(char *text)	
{
	root=cJSON_Parse(text);

	port_passwd = cJSON_GetObjectItem(root,"port_password");	//指向pord_password
}

/*readFile
	功能:读取json文件
	参数:
		filename:文件名
*/
void readFile(char *filename)
{
	FILE *f;
	long len;
	char *data;
	
	f=fopen(filename,"rb");
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);
	fread(data,1,len,f);
	fclose(f);
	doit(data);
	free(data);
}

/*count
	功能:统计文件长度
	参数:
		buffer:统计字符串
*/
long count(char *buffer)
{
	long count = 0;
	while(buffer[count] != '\0')
		count++;

	return count;
}

/*writeFile
	功能:写回并更新文件
*/
void writeFile()	
{
	FILE *file = fopen (file_loc, "wb+");
	char *out = cJSON_Print(root);

	//printf("The out is : %s\n",out);
	fwrite (out, sizeof(char), count(out),file);
	fclose(file);
	free(out);

	//输出写回成功信息
	printf("Write File: Successfully!\n");
}

/*backFile
	功能:备份文件回管理目录
*/
void backFile()
{
	system("cp /etc/shadowsocks.json ./shadowsock-backup.json");

	//输出备份信息
	printf("Backup: Successfully!\n");
}

/*findUser
	功能:查找json文件中是否已经存在该端口
	参数:
		port:端口号
*/
int findUser(char *port)
{
	cJSON *user = port_passwd->child;
	while(user->next != 0) {
		if (strcmp(user->string,port) != 0) 		
			user = user->next;
		else
			return 1;
	}
	if (strcmp(user->string,port) != 0)	//判断最后一个元素
		return 0;
	else
		return 1;
}

/*countUser
	功能:获取总用户数
*/
int countUser()
{
	
	return atoi(cJSON_GetObjectItem(root, "total_user")->valuestring);
}

/*recountUser
	功能:重新统计总用户数
*/
void recountUser()
{

	//重新统计用户数
	int count = 0;
	char *str_count = (char *)malloc(4);
	cJSON *total_user = cJSON_GetObjectItem(root, "total_user");
	cJSON *user = port_passwd->child;
	while(user->next != 0) {
		user = user->next;
		count++;
		//printf("%s\n",cJSON_Print(user));
	}
	sprintf(str_count, "%d", ++count);
	total_user->valuestring = str_count;
	
	printf("RECOUNT: Successfully!\n");
	//end-重新统计
}

/*changeCount
	功能:更新总用户数
	参数:
		
*/
void changeCount(int num)
{
	char *str_count = (char *)malloc(4);
	cJSON *total_user = cJSON_GetObjectItem(root, "total_user");
	sprintf(str_count, "%d", atoi(total_user->valuestring) + num);	//获取，加上num，转换
	total_user->valuestring = str_count;	//更新
}

/*setTime
	功能:设置开始时间
	参数:
		str_start_tp:开始时间戳字符指针
		str_start_tf:开始时间通俗表示字符指针
		month:当为0时，设置开始时间；不为0，设置结束时间
*/
void setTime(char *str_tp, char *str_tf, double month)
{
	time_t tmp_time;
	struct tm *ptm;
	if (month == 0)	//为0表示开始时间
		tmp_time = time(NULL);			//获取当前时间
	else	//否则为结束时间
		tmp_time = time(NULL) + 60 * 60 * 24 * 31 * month;	//计算结束时间戳
	
	ptm = localtime(&tmp_time);	//转换成当地时间
	
	sprintf(str_tp,"%d",tmp_time);	//日期转换成字符串（长整型日期）
	strftime(str_tf, 64, "%Y-%m-%d %X", ptm);	//格式化时间
}

/*scanTimer
	功能:扫描用户结束时间，并于当前时间做比较，到期则删除端口
*/
void scanTimer()
{
	time_t current_time, end_time;
	double diff_time;
	cJSON *user = port_passwd->child;
	int total = countUser();
	int i = 1;
	
	current_time = time(NULL);
	//扫描所有端口
	while(i <= total) {
		//当端口为代理端口，跳过之前的用户端口
		if (atoi(user->string) >= AGENT_MIN)
		{
			//当端口为激活端口
			cJSON *activated = cJSON_GetObjectItem(user, "activated");
			if (activated->type == cJSON_True)  {
				char *str_end_tp = (char *)malloc(64);
				str_end_tp = cJSON_GetObjectItem(user, "end_tp")->valuestring;	//获取结束时间戳
				if (str_end_tp != NULL)
					end_time = (time_t)atol(str_end_tp);	//字符串转time_t

				diff_time = difftime(current_time, end_time);

				if (diff_time > EXPIRED_LIMIT) {//过期一天
					printf("The port %s has expired.\n", user->string);
					char str_port[64];
					strcpy(str_port, user->string);	//要求定长字符串
					deleteUser(str_port);	//删除该端口
					
					//写回文件并重读
					writeFile();
					readFile(file_loc);
					i = 1;
					user = port_passwd->child;
					total = countUser();
					continue;
				}
			}
		}
		
		i++;
		user = user->next;
	}
}

/*activateUser
	功能:激活用户
	参数:
		port:端口号
		mon:激活使用时间
*/
void activateUser(char *port, char *mon)
{
	cJSON *user = port_passwd->child;
	cJSON *activate;
	//如果不存在端口
	if (findUser(port) == 0) {
		printf("ERROR: The port %s doesn't exit!\n", port);
		return;
	}
	
	//定位端口
	while (strcmp(user->string,port) != 0) 
		user = user->next;
	
	//更新激活字段
	activate = cJSON_GetObjectItem(user, "activated");	//获取激活字段
	if (activate->type == cJSON_False) {	//当原来为flase则激活
		activate->type = cJSON_True;
	}
	else {	//原来已被激活
		printf("ERROR: The port %s has been already activated!\n", port);
		return;
	}

	//更新开始时间
	char *str_start_tp = (char *)malloc(64);	//开始时间戳
	char *str_start_tf = (char *)malloc(64);	//开始时间通俗表示
	setTime(str_start_tp, str_start_tf, 0);		//设置开始时间
	cJSON_GetObjectItem(user, "start_tp")->valuestring = str_start_tp;
	cJSON_GetObjectItem(user, "start_tf")->valuestring = str_start_tf;
	
	
	//更新使用时长
	double month = atof(mon);	//字符串转数字
	cJSON_GetObjectItem(user, "month")->valuestring = mon;

	//更新结束时间
	char *str_end_tp = (char *)malloc(64);	//结束时间戳
	char *str_end_tf = (char *)malloc(64);	//结束时间通俗表示
	setTime(str_end_tp, str_end_tf, month);	//设置结束时间
	cJSON_GetObjectItem(user, "end_tp")->valuestring = str_end_tp;
	cJSON_GetObjectItem(user, "end_tf")->valuestring = str_end_tf;

	//输出激活成功
	printf("ACTIVATE: Successfully!\n");
}

/*prolongUser
	功能:延长用户的使用时间（支持减少使用时间）
	参数:
		port:端口号
		mon:延长到时间（月，支持负数，小数）
*/
void prolongUser(char *port, char *mon)
{
	cJSON *user = port_passwd->child;

	//如果不存在端口
	if (findUser(port) == 0) {
		printf("ERROR: The port %s doesn't exit!\n", port);
		return;
	}
	
	//定位端口
	while (strcmp(user->string,port) != 0) 
		user = user->next;

	//如果端口未被激活
	if (cJSON_GetObjectItem(user, "activated")->type == cJSON_False) {
		printf("ERROR: The port %s can only be prolong before been activated!\n", port);
		return;
	}

	//更新使用时长
	double month = atof(mon) + atof(cJSON_GetObjectItem(user, "month")->valuestring);	//字符串转数字
	char *n_mon = (char *)malloc(4);
	sprintf(n_mon, "%f", month);
	cJSON_GetObjectItem(user, "month")->valuestring = n_mon;

	//更新结束时间
	time_t tmp_time, pre_time;
	struct tm *ptm;
	char *str_end_tp = (char *)malloc(64);	//结束时间戳
	char *str_end_tf = (char *)malloc(64);	//结束时间通俗表示
	
	//根据原来的开始时间戳来计算新的结束时间
	pre_time = atol(cJSON_GetObjectItem(user, "start_tp")->valuestring);
	tmp_time = pre_time + 60 * 60 * 24 * 31 * month;	//计算结束时间戳
	ptm = localtime(&tmp_time);	//转换成当地时间
	sprintf(str_end_tp,"%d",tmp_time);	//日期转换成字符串（长整型日期）
	strftime(str_end_tf, 64, "%Y-%m-%d %X", ptm);	//格式化时间

	cJSON_GetObjectItem(user, "end_tp")->valuestring = str_end_tp;
	cJSON_GetObjectItem(user, "end_tf")->valuestring = str_end_tf;

	//输出延长成功
	printf("PROLONG: Successfully!\n");
}

/*addUser
	功能:添加端口，即添加用户，分单个添加和批量添加
	参数:
		month:使用时间（月）
		type:为0：批量添加操作；为1，单个添加操作。
*/
void addUser(char *mon, int type)
{
	char *password = (char *)malloc(PASS_LEN + 1);
	cJSON *user;
	int i;

	//生成端口
	int port_num = getCurrentPort();//得到当前可以使用的端口号
	char *port = (char *)malloc(10);
	sprintf(port,"%d",port_num);	//数字转字符串
	/*旧版本
	while (findUser(port) == 1) {
		port_num++;
		sprintf(port,"%d",port_num);
	}*/

	//生成随机密码
	//srand((unsigned)time(NULL));	//放到main函数中
	for (i = 0; i < PASS_LEN; i++) 
		password[i] = 'a' + rand() % 26;
	password[i] = '\0';
	
	
	//设置开始时间
	char *str_start_tp = (char *)malloc(64);	//开始时间戳
	char *str_start_tf = (char *)malloc(64);	//开始时间通俗表示
	setTime(str_start_tp, str_start_tf, 0);		//设置开始时间
	
	//设置使用时长
	double month = atof(mon);	//字符串转数字

	//设置结束时间
	char *str_end_tp = (char *)malloc(64);	//结束时间戳
	char *str_end_tf = (char *)malloc(64);	//结束时间通俗表示
	setTime(str_end_tp, str_end_tf, month);	//设置结束时间
	
	//添加用户
	cJSON_AddItemToObject(port_passwd,port,user=cJSON_CreateObject());	//创建cJSON对象

	if (type == cJSON_True)	//单个添加
		cJSON_AddBoolToObject(user,"activated",cJSON_True);	//设置激活字段（初始为真）
	else	//批量添加
		cJSON_AddBoolToObject(user,"activated",cJSON_False);//设置激活字段（初始为假）

	cJSON_AddStringToObject(user,"password",password);	//设置用户密码
	cJSON_AddStringToObject(user,"month",mon);			//设置用户使用时间（月）
	cJSON_AddStringToObject(user,"start_tf",str_start_tf);	//设置开始时间（通俗）
	cJSON_AddStringToObject(user,"end_tf",str_end_tf);		//设置结束时间（通俗）
	cJSON_AddStringToObject(user,"start_tp",str_start_tp);	//设置开始时间戳
	cJSON_AddStringToObject(user,"end_tp",str_end_tp);		//设置结束时间戳
	
	//更新总用户数
	changeCount(1);	
	
	//输出添加成功信息
	if (type == cJSON_True)	//单个添加
		printf("ADD: Successfully!\n==The port is : %s\n==The password is :%s.\n",port, password);
	else {	//批量添加
		//输出配置信息
		/*
		//printf("\nServer Settings:\n");
		printf("\nServer IP (Host): %s\n", server_ip);
		printf("(Remote) Port: %s\n", user->string);
		printf("Password: %s\n", password);
		printf("Encrypt Method: %s\n", encrypt_method);
		printf("Protocol: %s\n", protocol);
		printf("Obfuscation: %s\n", obfuscation);*/
		printf("\n==The port is : %s\n==The password is :%s.\n",port, password);
	}
	free(password);
}

/*deleteUser
	功能:删除端口，即删除用户
	参数:
		port:端口号
*/
void deleteUser(char *port)
{	
	
	//删除指定端口
	cJSON_DeleteItemFromObject(port_passwd,port);
	//总数减一
	changeCount(-1);

	//输出删除成功信息
	printf("DELETE: Successfully! The port is %s.\n",port);
}

/*changePasswd
	功能:修改端口密码
	参数:
		port:端口号
*/
void changePasswd(char *port)
{
	char *password = (char *)malloc(PASS_LEN + 1);
	cJSON *user = port_passwd->child;
	int i;
	
	while(user->next != 0) {
		if (strcmp(user->string,port) != 0) 		
			user = user->next;
		else
			break;
	}
	if (strcmp(user->string,port) != 0) {	//判断最后一个元素
		printf("CHANGE: Failed! The port %s doesn't exit.\n",port);
		return;
	}
	
	//生成随机密码
	srand(time(NULL));
	for (i = 0; i < PASS_LEN; i++) 
		password[i] = 'a' + rand() % 26;
	password[i] = '\0';

	printf("The previous password of port %s is %s.\n",user->string,cJSON_GetObjectItem(user, "password")->valuestring);
	cJSON_GetObjectItem(user, "password")->valuestring = password;
	
	//输出添加成功信息
	printf("CHANGE: Successfully!\n==The port is : %s\n==The new password is :%s.\n",port, password);
	//free(password);
}

/*refreshSSR
	功能:更新shadowsockr软件（只有更新了才能使用）
*/
void refreshSSR()
{
	system("/etc/init.d/shadowsocks restart");
}

/*getUserPasswd
	功能:获得某个端口的密码
	参数:
		port:端口号
*/
char *getUserPasswd(char *port)
{
	cJSON *user = port_passwd->child;
	while(user->next != 0) {
		if (strcmp(user->string,port) != 0) 		
			user = user->next;
		else
			return cJSON_GetObjectItem(user, "password")->valuestring;	//返回密码
	}
	if (strcmp(user->string,port) != 0)	//判断最后一个元素
		return 0;
	else
		return cJSON_GetObjectItem(user, "password")->valuestring;
}

/*getCurrentPort
	功能:得到当前未被使用的端口号
*/
int getCurrentPort()
{
	cJSON *current_port = cJSON_GetObjectItem(root, "current_port");	//获取cJSON对象
	int current = atoi(current_port->valuestring);	//获取端口
	sprintf(current_port->valuestring,"%d",current + 1);	//更新端口号，加1
	
	return current;
}

/*countFalse
	功能:获得所有无效字段的端口，并统计数量
*/
void countFalse()
{
	cJSON *user = port_passwd->child;
	int total = countUser();
	int i = 1, count = 0;
	
	printf("The cJSON_False port:\n");
	while (i <= total) {
		cJSON *activated = cJSON_GetObjectItem(user, "activated");
		if (activated && activated->type == cJSON_False){
			count++;
			printf("==port %s\n",user->string);
		}
		i++;
		user = user->next;	
	}
	printf("Total false port: %d\n", count);
}

/*countFalse
	功能:删除所有无效字段的端口
*/
void deleteFalse()
{
	cJSON *user = port_passwd->child;
	int total = countUser();
	int i = 1;

	printf("Begin to delete false port...\n");
	while (i <= total) {
		cJSON *activated = cJSON_GetObjectItem(user, "activated");
		if (activated && activated->type == cJSON_False){
			char str_port[64];
			strcpy(str_port, user->string);	//要求定长字符串
			deleteUser(str_port);	//删除该端口

			//写回文件并重读
			writeFile();
			readFile(file_loc);
			i = 1;
			user = port_passwd->child;
			total = countUser();
			continue;
		}
		i++;
		user = user->next;
	}
	printf("==Delte false end.\n");
}

/*infoUser
	功能:显示某个端口的信息
	参数:
		port:端口号
*/
void infoUser(char *port)
{
	cJSON *user = cJSON_GetObjectItem(port_passwd, port);
	if (user != NULL)
	{
		printf("Port \"%s\":%s\n", port, cJSON_Print(user));
	}
	else
		printf("The port %s doesn't exit.\n", port);
}

/*printHelp
	功能:显示本程序使用命令
*/
void printHelp()
{
	printf("========================Command Manual========================\n");
	printf("-add {port} {month}\t\t添加一个用户，设置使用时间\n");
	printf("-batch {number of ports}\t批量添加用户，输入添加个数\n");
	printf("-activate {port} {month}\t激活某个端口，并设置使用时间\n");
	printf("-prolong {port} {month}\t\t延长某个端口的使用时间\n");
	printf("-delete {port}\t\t\t删除某个端口\n");
	printf("-get {port}\t\t\t得到某个端口的密码\n");
	printf("-change {port}\t\t\t改变某个端口的密码\n");
	printf("-info {port}\t\t\t获得某个端口的信息\n");
	printf("-back\t\t\t\t备份配置文件到当前目录\n");
	printf("-refresh\t\t\t更新ShadowsockR软件\n");
	printf("-scan\t\t\t\t扫描定时器，删除过期端口\n");
	printf("-count\t\t\t\t获得总用户数\n");
	printf("-recount\t\t\t重新统计总用户数\n");
	printf("-countfalse\t\t\t统计无效端口数\n");
	printf("-deletefalse\t\t\t删除无效端口数\n");
	printf("-help\t\t\t\t显示程序帮助信息\n");
	printf("=============================END=============================\n");
}

/*
	功能:主函数
	参数:
		argc:参数个数（包含manager）
		argv[1]:操作命令（-add,-delete,-get,-back,-refresh,-change等）
		argv[2]:个别操作命令参数，一般为端口号
		argv[3]:一般为使用月数
*/
int main (int argc, char *argv[])
{
	printf("===========START=========\n");
	system("date");
	printf("\n");
	srand((unsigned)time(NULL));
	system("cp /etc/shadowsocks.json ./shadowsock-backup.json");	//备份文件

	if (argc > 1){
		readFile(file_loc);	//读取文件
		//添加用户
		if (strcmp(argv[1],"-add") == 0) {
			if (argc < 3) {	//命令参数不足
				printf("ERROR:Not enough params!\n");
				printf("TIPS:manager -add {month}\n");
				printf("==========END==========\n");
				return 1;
			}
			
			addUser(argv[2], cJSON_True);
			writeFile();
			backFile();
			//system("/etc/init.d/shadowsocks restart &");
		}
		//批量添加用户
		else if(strcmp(argv[1],"-batch") == 0) {
			printf("ADD BATCH USERS:\n");
			if (argc < 3) {	//命令参数不足
				printf("ERROR:Not enough params!\n");
				printf("TIPS:manager -batch {number of ports}\n");
				printf("==========END==========\n");
				return 1;
			}
			int count = atoi(argv[2]);
			int i;
			for (i = 0; i < count; i++)
				addUser("0", cJSON_False);	//使用时间为0
			
			writeFile();
			backFile();
		}
		//删除用户
		else if (strcmp(argv[1],"-delete") == 0) {
			if (argc < 3) {	//命令参数不足
				printf("ERROR:Not enough params!\n");
				printf("TIPS:manager -delete {port}\n");
				printf("==========END==========\n");
				return 1;
			}
			if(findUser(argv[2]) == 1)			
				deleteUser(argv[2]);
			else	//端口不存在
				printf("ERROR: The port does not exist.\n");

			writeFile();
			//system("/etc/init.d/shadowsocks restart &");
		}
		//得到某个端口密码
		else if (strcmp(argv[1],"-get") == 0) {
			if (argc < 3) {	//命令参数不足
				printf("ERROR:Not enough params!\n");
				printf("TIPS:manager -get {port}\n");
				printf("==========END==========\n");
				return 1;
			}
			printf("The password of port %s is : %s\n",argv[2],getUserPasswd(argv[2]));
		}
		//备份配置文件到管理目录
		else if (strcmp(argv[1],"-back") == 0) {
			backFile();
		}
		//更新shadowsocksr软件
		else if (strcmp(argv[1],"-refresh") == 0) {
			refreshSSR();
		}
		//更改某个端口的密码
		else if (strcmp(argv[1],"-change") == 0) {
			if (argc < 3) {	//命令参数不足
				printf("ERROR:Not enough params!\n");
				printf("TIPS:manager -change {port}\n");
				printf("==========END==========\n");
				return 1;
			}
			changePasswd(argv[2]);
			writeFile();
			backFile();
		}
		//获得总用户数
		else if (strcmp(argv[1],"-count") == 0) {
			printf("Total user: %d\n",countUser());
		}
		//获得总用户数
		else if (strcmp(argv[1],"-recount") == 0) {
			recountUser();
			writeFile();
			backFile();
		}
		//扫描定时器
		else if (strcmp(argv[1],"-scan") == 0) {
			scanTimer();
			//writeFile();
			backFile();
		}
		//激活端口
		else if (strcmp(argv[1],"-activate") == 0) {
			if (argc < 4) {
				printf("ERROR:Not enough params!\n");
				printf("TIPS:manager -activate {port} {month}\n");
				printf("==========END==========\n");
				return 1;
			}
			activateUser(argv[2], argv[3]);
			writeFile();	
			backFile();	
		}
		//延长端口使用时间
		else if (strcmp(argv[1],"-prolong") == 0) {
			if (argc < 4) {
				printf("ERROR:Not enough params!\n");
				printf("TIPS:manager -prolong {port} {month}\n");
				printf("==========END==========\n");
				return 1;
			}
			prolongUser(argv[2], argv[3]);
			writeFile();
			backFile();	
		}
		//统计无效字段端口的数量
		else if (strcmp(argv[1],"-countfalse") == 0) {
			countFalse();
		}
		//删除无效字段端口的数量
		else if (strcmp(argv[1],"-deletefalse") == 0) {
			deleteFalse();
		}
		//显示帮助信息
		else if (strcmp(argv[1],"-help") == 0) {
			printHelp();
		}
		//显示某个端口的全部信息
		else if (strcmp(argv[1],"-info") == 0) {
			if (argc < 3) {	//命令参数不足
				printf("ERROR:Not enough params!\n");
				printf("TIPS:manager -info {port}\n");
				printf("==========END==========\n");
				return 1;
			}
			infoUser(argv[2]);
		}
		//test作用
		else if (strcmp(argv[1],"-test") == 0) {
			//countUser();
			//deleteFalse();
			//writeFile();		
		}
		else {
			printf("ERROR:Invalid command!\n");
		}
	}
	printf("==========END==========\n");
	//cJSON_Delete(root);	//清空root

	return 0;
}

