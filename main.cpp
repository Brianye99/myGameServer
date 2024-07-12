//#include<iostream>
//#include<zinx.h>
#include "GameChannel.h"
#include "GameMsg.h"
#include "msg.pb.h"
#include <cstdio>
#include "AOIWorld.h"
#include "RandomName.h"
#include "ZinxTimer.h"

//test
//class myPlayer :public Player {
//public:
//	myPlayer(int _x, int _y, std::string _name) :x(_x), y(_y), name(_name) {}
//	int x;
//	int y;
//	std::string name;
//	// 通过 Player 继承
//	virtual int GetX() override
//	{
//		return x;
//	}
//	virtual int GetY() override
//	{
//		return y;
//	}
//};

extern RandomName randomName;

//创建守护进程
void Daemonlize() {
	//1fork创建子进程
	int pid = fork();
	if (pid < 0) { exit(-1); }
	if (pid > 0) {
		//2父进程退出
		exit(0);
	}
	//3子进程开启会话，设置会话pid
	pid_t sid = setsid();
	//4子进程 设置执行路径（不必要
	//5子进程 重定向3个文件描述符到/dev/null 黑洞文件
	int nullfd = open("/dev/null", O_RDWR);
	if (nullfd >= 0) {
		dup2(nullfd, 0);
		dup2(nullfd, 1);
		dup2(nullfd, 2);
		close(nullfd);
	}
	//进程监控
	while (1) {
		pid = fork();
		if (pid < 0) {
			exit(-1);
		}
		//父进程等子进程退出
		if (pid > 0) {
			int status = 0;
			wait(&status);
			if (0 == status) {
				exit(0);
			}
		}
		else { break; }//子进程跳出循环执行游戏业务
	}
}

int main() {

	/*AOIWorld w(20, 200, 50, 230, 6, 6);
	myPlayer p1(60, 107, "1");
	myPlayer p2(91, 118, "2");
	myPlayer p3(147, 133, "3");
	w.AddPlayer(&p1);
	w.AddPlayer(&p2);
	w.AddPlayer(&p3);

	auto srd_list = w.GetSurroundPlayers(&p1);
	for (auto single : srd_list)
	{
		std::cout << dynamic_cast<myPlayer*>(single)->name << std::endl;
	}*/
	
	Daemonlize();
	randomName.LoadFile();

	ZinxKernel::ZinxKernelInit();
	/*添加监听通道:端口号和连接*/
	ZinxKernel::Zinx_Add_Channel(*(new ZinxTCPListen(8899, new GameConnFact())));
	ZinxKernel::Zinx_Add_Channel(*(new ZinxTimer()));
	ZinxKernel::Zinx_Run();
	ZinxKernel::ZinxKernelFini();

}
