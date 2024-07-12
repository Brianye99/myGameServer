#include "GameRole.h"
#include "GameMsg.h"
#include "GameProtocol.h"
#include "GameChannel.h"
#include "AOIWorld.h"
#include <algorithm>
#include <random>
#include "RandomName.h"
#include <zinx.h>
#include "ZinxTimer.h"
#include <hiredis/hiredis.h>

//����ȫ����������ض���
RandomName randomName;
//����ȫ�����������
static std::default_random_engine RoleRandomEngine(time(NULL));
//����AOIworldȫ�ֶ���
static AOIWorld gameWorld(0,400,0,400,20,20);

void GameRole::ProcTalking(std::string _content)
{
	//����������
	auto role_list = ZinxKernel::Zinx_GetAllRole();
	for (auto pRole : role_list) {
		auto pGameRole = dynamic_cast<GameRole*>(pRole);
		auto pmsg = CreatTalkBroadCast(_content);
		ZinxKernel::Zinx_SendOut(*pmsg, *(pGameRole->m_proto));
	}
}

void GameRole::ProcMoveMsg(float _x, float _y, float _z, float _v)
{
	//1��������
	//��ȡԭ�����ھ�s1
	std::list<Player*>s1 = gameWorld.GetSurroundPlayers(this);
	//�������꣬ժ���ɸ��ӣ�����¸��ӣ���ȡ����Χ���s2
	gameWorld.DelPlayer(this);
	x = _x;
	y = _y;
	z = _z;
	v = _v;
	gameWorld.AddPlayer(this);
	std::list<Player*>s2 = gameWorld.GetSurroundPlayers(this);
	//����s2,��Ԫ�ز�����s1����Ұ����
	for (Player* singlePlayer : s2) {
		if (s1.end() == std::find(s1.begin(), s1.end(), singlePlayer)) {
			ViewAppear(dynamic_cast<GameRole*>(singlePlayer));
		}
	}
	//����s1����Ԫ�ز�����s2����Ұ��ʧ
	for (Player* singlePlayer : s1) {
		if (s2.end() == std::find(s2.begin(), s2.end(), singlePlayer)) {
			ViewLost(dynamic_cast<GameRole*>(singlePlayer));
		}
	}
	//2�㲥��λ�ø���Χ���
	//��һ���Ѿ��Ǹ���֮����ˣ�
	std::list<Player*> surPlayerList = gameWorld.GetSurroundPlayers(this);
	for (Player* pSinglePlayer : surPlayerList) {
		//��ɴ����ͱ���
		pb::BroadCast* pBroadCastMsg = new pb::BroadCast();
		pb::Position* pPosition = pBroadCastMsg->mutable_p();
		pPosition->set_x(_x);
		pPosition->set_y(_y);
		pPosition->set_z(_z);
		pPosition->set_v(_v);
		pBroadCastMsg->set_pid(playerPid);
		pBroadCastMsg->set_tp(4);
		pBroadCastMsg->set_username(playerName);
		//������Χ��ҷ���
		GameRole* pGameRole = dynamic_cast<GameRole*>(pSinglePlayer);
		GameMsg* pMsg = new GameMsg(GameMsg::MSG_TYPE_BROADCAST, pBroadCastMsg);
		ZinxKernel::Zinx_SendOut(*pMsg, *(pGameRole->m_proto));
	}
}

void GameRole::ViewAppear(GameRole* _pRole)
{
	//���Լ����Ͳ�����ҵ�200��Ϣ(���ID��tp1��������/tp2��ʼλ��/tp3������Ԥ����/tp4��λ��)
	ZinxKernel::Zinx_SendOut(*(_pRole->CreateSelfPosition()), *(this->m_proto));
	//����������ҷ��Լ���200��Ϣ
	ZinxKernel::Zinx_SendOut(*(this->CreateSelfPosition()), *(_pRole->m_proto));
}

void GameRole::ViewLost(GameRole* _pRole)
{
	//���Լ����Ͳ�����ҵ�201��Ϣ(���ID���������)
	ZinxKernel::Zinx_SendOut(*(_pRole->CreateIDNameLogoff()), *(this->m_proto));
	//����������ҷ��Լ���201��Ϣ
	ZinxKernel::Zinx_SendOut(*(this->CreateIDNameLogoff()), *(_pRole->m_proto));
}

//�¿ͻ����������Լ�����ID������
GameMsg* GameRole::CreateIDNameLogin()
{
	pb::SyncPid* pMsg = new pb::SyncPid();
	pMsg->set_pid(playerPid);
	pMsg->set_username(playerName);
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_LOGIN_ID_NAME, pMsg);
	return pRet;
}
//�¿ͻ����������䷢����Χ��ҵ�λ��
GameMsg* GameRole::CreateSurPlayers()
{
	pb::SyncPlayers* pMsg = new pb::SyncPlayers();

	std::list<Player*> surPlayersList = gameWorld.GetSurroundPlayers(this);
	//��Χ�ж�����
	for (auto singlePlayer : surPlayersList) {
		//���ࣨ����Ϣ���ҵ����ࣨ����Ϣ�������ظ���ָ��pb::Player
		pb::Player* pPlayer = pMsg->add_ps();//
		GameRole* pRole = dynamic_cast<GameRole*>(singlePlayer);

		//���õ�����������ҵ���Ϣ
		pPlayer->set_pid(pRole->playerPid);
		pPlayer->set_username(pRole->playerName);

		//����(����Ϣ)�ҵ�����(����Ϣ)�����������ָ��pb::Position
		pb::Position* pPosition = pPlayer->mutable_p();
		pPosition->set_x(pRole->x);
		pPosition->set_y(pRole->y);
		pPosition->set_z(pRole->z);
		pPosition->set_v(pRole->v);
	}
	
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_SRD_POSTION, pMsg);
	return pRet;
}
//�¿ͻ�����������Χ��ҷ�����λ��
GameMsg* GameRole::CreateSelfPosition()
{
	//����protobuf����
	pb::BroadCast* pMsg = new pb::BroadCast();
	//дprotobuf����
	pMsg->set_pid(playerPid);
	pMsg->set_username(playerName);
	pMsg->set_tp(2);//tp=1:��������/tp=2:��ʼλ��/tp=3:������Ԥ����/tp=4:��λ��

	//����ҵ����࣬��������ָ��
	pb::Position* pPosition = pMsg->mutable_p();
	pPosition->set_x(x);
	pPosition->set_y(y);
	pPosition->set_z(z);
	pPosition->set_v(v);
	//����protobuf����
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_BROADCAST, pMsg);
	return pRet;
}

GameMsg* GameRole::CreateIDNameLogoff()
{
	//����protobuf��Ϣ
	pb::SyncPid* pMsg = new pb::SyncPid();
	//дprotobuf��Ϣ
	pMsg->set_pid(playerPid);
	pMsg->set_username(playerName);
	//����protobuf��Ϣ
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_LOGOFF_ID_NAME, pMsg);
	return pRet;
}

GameMsg* GameRole::CreatTalkBroadCast(std::string _content)
{
	pb::BroadCast* pmsg = new pb::BroadCast();
	pmsg->set_pid(playerPid);
	pmsg->set_username(playerName);
	pmsg->set_tp(1);//1������
	pmsg->set_content(_content);
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_BROADCAST, pmsg);
	return pRet;
}

GameRole::GameRole() {
	x = 100 + RoleRandomEngine() % 30;
	z = 100 + RoleRandomEngine() % 30;
	playerName = randomName.GetName();
	
	//playerName = "test";
	
}

GameRole::~GameRole() {

}

class ExitTimer: public TimeOutProc
{
	// ͨ�� TimeOutProc �̳�
	void Proc() override
	{
		ZinxKernel::Zinx_Exit();
	}
	int getTimeSec() override
	{
		//���ȫ������10s��رշ�����
		return 10;
	}
};
//ʵ����һ����ʱ��ȫ�ֶ���
static ExitTimer exitTimer;

bool GameRole::Init()
{
	//�������û��ң��������Ϊ��һ���������Init��ɾ���˳���ʱ��
	if (ZinxKernel::Zinx_GetAllRole().size() <= 0) {
		TimeOutManager::getInstance().delTask(&exitTimer);
	}


	//����Լ�����Ϸ����
	bool bRet =false;
	playerPid = m_proto->m_channel->GetFd();//�������IDΪ��ǰ���ӵ�fd,��ȡ�ļ��������������Ψһ��
	bRet = gameWorld.AddPlayer(this);

	if (bRet == true) {
		//���Լ�����ID������
		GameMsg* pMsg = CreateIDNameLogin();//auto
		ZinxKernel::Zinx_SendOut(*pMsg, *m_proto);

		//���Լ�������Χ���λ��
		pMsg = CreateSurPlayers();
		ZinxKernel::Zinx_SendOut(*pMsg, *m_proto);

		//����Χ��ҷ����Լ���λ��
		std::list<Player*> surPlayers = gameWorld.GetSurroundPlayers(this);//auto

		//player*
		for (auto singlePlayer : surPlayers) {
			pMsg = CreateSelfPosition();

			auto pRole = dynamic_cast<GameRole*>(singlePlayer);
			ZinxKernel::Zinx_SendOut(*pMsg, *(pRole->m_proto));
		}
	}

	//��¼��ǰ������redis��Game_name
	//1����redis
	redisContext* conn = redisConnect("127.0.0.1", 6379);
	//2����lpush����
	if (conn != NULL) {
		redisReply* reply = (redisReply*)redisCommand(conn, "rpush usr_name %s", playerName.c_str());
		freeReplyObject(reply);
		redisFree(conn);
	}
	return bRet;
}
//������Ϸ��ص��û�����
UserData* GameRole::ProcMsg(UserData& _poUserData)
{
	//��Э�����ת����16���ƺ����Ϣ����MutiMsg����
	GET_REF2DATA(MultiMsg, input, _poUserData);
	for (auto singleGameMsg : input.m_Msgs) {
		//������Ϣ
		//����pMsgΪprotobuf����
		std::cout << "type is " << singleGameMsg->enMsgType << std::endl;
		std::cout << singleGameMsg->pMsg->Utf8DebugString() << std::endl;

		//����positonָ������ȡ���꣨������switch���ڲ����壩
		//pb::Position* pNewPosition = dynamic_cast<pb::Position*>(singleGameMsg->pMsg);
		//switch (singleGameMsg->enMsgType)
		//{
		//	//��������Ϣ�����������
		//case:GameMsg::MSG_TYPE_CHAT_CONTENT

		//default:
		//	break;
		//}
	}
	return nullptr;
}

void GameRole::Fini()
{
	//����Χ��ҷ������ߵ���Ϣ
	std::list<Player*> surPlayerList = gameWorld.GetSurroundPlayers(this);

	//Player*
	for (auto singlePlayer : surPlayerList) {
		GameMsg* pMsg = CreateIDNameLogoff();
		GameRole* pRole = dynamic_cast<GameRole*>(singlePlayer);
		ZinxKernel::Zinx_SendOut(*pMsg, *(pRole->m_proto));
	}
	gameWorld.DelPlayer(this);

	//���һ��������ߺ󣬿�ʼ��ʱ��20sû����������߾��˳�������
	if (ZinxKernel::Zinx_GetAllRole().size() <= 0) {
		TimeOutManager::getInstance().addTask(&exitTimer);
	}

	//��redis��ɾ�������������
	redisContext* conn = redisConnect("127.0.0.1", 6379);
	if (NULL != conn) {
		redisReply* reply = (redisReply*)redisCommand(conn, "lrem usr_name 1 %s", playerName.c_str());
		freeReplyObject(reply);
		redisFree(conn);
	}

}


//ʵ��Player��Ĵ��麯��
int GameRole::GetX()
{
	return (int)x;
}

int GameRole::GetY()
{
	return (int)z;//�ͻ��˶���yΪ�ߣ�zΪ������
}
