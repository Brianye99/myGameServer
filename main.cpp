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
//	// ͨ�� Player �̳�
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

//�����ػ�����
void Daemonlize() {
	//1fork�����ӽ���
	int pid = fork();
	if (pid < 0) { exit(-1); }
	if (pid > 0) {
		//2�������˳�
		exit(0);
	}
	//3�ӽ��̿����Ự�����ûỰpid
	pid_t sid = setsid();
	//4�ӽ��� ����ִ��·��������Ҫ
	//5�ӽ��� �ض���3���ļ���������/dev/null �ڶ��ļ�
	int nullfd = open("/dev/null", O_RDWR);
	if (nullfd >= 0) {
		dup2(nullfd, 0);
		dup2(nullfd, 1);
		dup2(nullfd, 2);
		close(nullfd);
	}
	//���̼��
	while (1) {
		pid = fork();
		if (pid < 0) {
			exit(-1);
		}
		//�����̵��ӽ����˳�
		if (pid > 0) {
			int status = 0;
			wait(&status);
			if (0 == status) {
				exit(0);
			}
		}
		else { break; }//�ӽ�������ѭ��ִ����Ϸҵ��
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
	/*��Ӽ���ͨ��:�˿ںź�����*/
	ZinxKernel::Zinx_Add_Channel(*(new ZinxTCPListen(8899, new GameConnFact())));
	ZinxKernel::Zinx_Add_Channel(*(new ZinxTimer()));
	ZinxKernel::Zinx_Run();
	ZinxKernel::ZinxKernelFini();

}
