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

//创建全局随机姓名池对象
RandomName randomName;
//创建全局随机数引擎
static std::default_random_engine RoleRandomEngine(time(NULL));
//创建AOIworld全局对象
static AOIWorld gameWorld(0,400,0,400,20,20);

void GameRole::ProcTalking(std::string _content)
{
	//发给所有人
	auto role_list = ZinxKernel::Zinx_GetAllRole();
	for (auto pRole : role_list) {
		auto pGameRole = dynamic_cast<GameRole*>(pRole);
		auto pmsg = CreatTalkBroadCast(_content);
		ZinxKernel::Zinx_SendOut(*pmsg, *(pGameRole->m_proto));
	}
}

void GameRole::ProcMoveMsg(float _x, float _y, float _z, float _v)
{
	//1跨网格处理
	//获取原来的邻居s1
	std::list<Player*>s1 = gameWorld.GetSurroundPlayers(this);
	//更新坐标，摘除旧格子，添加新格子，获取新周围玩家s2
	gameWorld.DelPlayer(this);
	x = _x;
	y = _y;
	z = _z;
	v = _v;
	gameWorld.AddPlayer(this);
	std::list<Player*>s2 = gameWorld.GetSurroundPlayers(this);
	//遍历s2,若元素不属于s1，视野出现
	for (Player* singlePlayer : s2) {
		if (s1.end() == std::find(s1.begin(), s1.end(), singlePlayer)) {
			ViewAppear(dynamic_cast<GameRole*>(singlePlayer));
		}
	}
	//遍历s1，若元素不属于s2，视野消失
	for (Player* singlePlayer : s1) {
		if (s2.end() == std::find(s2.begin(), s2.end(), singlePlayer)) {
			ViewLost(dynamic_cast<GameRole*>(singlePlayer));
		}
	}
	//2广播新位置给周围玩家
	//这一步已经是更新之后的了？
	std::list<Player*> surPlayerList = gameWorld.GetSurroundPlayers(this);
	for (Player* pSinglePlayer : surPlayerList) {
		//组成待发送报文
		pb::BroadCast* pBroadCastMsg = new pb::BroadCast();
		pb::Position* pPosition = pBroadCastMsg->mutable_p();
		pPosition->set_x(_x);
		pPosition->set_y(_y);
		pPosition->set_z(_z);
		pPosition->set_v(_v);
		pBroadCastMsg->set_pid(playerPid);
		pBroadCastMsg->set_tp(4);
		pBroadCastMsg->set_username(playerName);
		//遍历周围玩家发送
		GameRole* pGameRole = dynamic_cast<GameRole*>(pSinglePlayer);
		GameMsg* pMsg = new GameMsg(GameMsg::MSG_TYPE_BROADCAST, pBroadCastMsg);
		ZinxKernel::Zinx_SendOut(*pMsg, *(pGameRole->m_proto));
	}
}

void GameRole::ViewAppear(GameRole* _pRole)
{
	//向自己发送参数玩家的200消息(玩家ID，tp1聊天内容/tp2初始位置/tp3动作（预留）/tp4新位置)
	ZinxKernel::Zinx_SendOut(*(_pRole->CreateSelfPosition()), *(this->m_proto));
	//向参数里的玩家发自己的200消息
	ZinxKernel::Zinx_SendOut(*(this->CreateSelfPosition()), *(_pRole->m_proto));
}

void GameRole::ViewLost(GameRole* _pRole)
{
	//向自己发送参数玩家的201消息(玩家ID和玩家姓名)
	ZinxKernel::Zinx_SendOut(*(_pRole->CreateIDNameLogoff()), *(this->m_proto));
	//向参数里的玩家发自己的201消息
	ZinxKernel::Zinx_SendOut(*(this->CreateIDNameLogoff()), *(_pRole->m_proto));
}

//新客户端上香向自己发送ID和名称
GameMsg* GameRole::CreateIDNameLogin()
{
	pb::SyncPid* pMsg = new pb::SyncPid();
	pMsg->set_pid(playerPid);
	pMsg->set_username(playerName);
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_LOGIN_ID_NAME, pMsg);
	return pRet;
}
//新客户端上线向其发送周围玩家的位置
GameMsg* GameRole::CreateSurPlayers()
{
	pb::SyncPlayers* pMsg = new pb::SyncPlayers();

	std::list<Player*> surPlayersList = gameWorld.GetSurroundPlayers(this);
	//周围有多个玩家
	for (auto singlePlayer : surPlayersList) {
		//子类（子消息）挂到父类（父消息），返回父类指针pb::Player
		pb::Player* pPlayer = pMsg->add_ps();//
		GameRole* pRole = dynamic_cast<GameRole*>(singlePlayer);

		//设置到遍历到的玩家的消息
		pPlayer->set_pid(pRole->playerPid);
		pPlayer->set_username(pRole->playerName);

		//子类(子消息)挂到父类(父消息)，返回子类的指针pb::Position
		pb::Position* pPosition = pPlayer->mutable_p();
		pPosition->set_x(pRole->x);
		pPosition->set_y(pRole->y);
		pPosition->set_z(pRole->z);
		pPosition->set_v(pRole->v);
	}
	
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_SRD_POSTION, pMsg);
	return pRet;
}
//新客户端上线向周围玩家发送其位置
GameMsg* GameRole::CreateSelfPosition()
{
	//定义protobuf数据
	pb::BroadCast* pMsg = new pb::BroadCast();
	//写protobuf数据
	pMsg->set_pid(playerPid);
	pMsg->set_username(playerName);
	pMsg->set_tp(2);//tp=1:聊天内容/tp=2:初始位置/tp=3:动作（预留）/tp=4:新位置

	//子类挂到父类，返回子类指针
	pb::Position* pPosition = pMsg->mutable_p();
	pPosition->set_x(x);
	pPosition->set_y(y);
	pPosition->set_z(z);
	pPosition->set_v(v);
	//发送protobuf数据
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_BROADCAST, pMsg);
	return pRet;
}

GameMsg* GameRole::CreateIDNameLogoff()
{
	//定义protobuf消息
	pb::SyncPid* pMsg = new pb::SyncPid();
	//写protobuf消息
	pMsg->set_pid(playerPid);
	pMsg->set_username(playerName);
	//发送protobuf消息
	GameMsg* pRet = new GameMsg(GameMsg::MSG_TYPE_LOGOFF_ID_NAME, pMsg);
	return pRet;
}

GameMsg* GameRole::CreatTalkBroadCast(std::string _content)
{
	pb::BroadCast* pmsg = new pb::BroadCast();
	pmsg->set_pid(playerPid);
	pmsg->set_username(playerName);
	pmsg->set_tp(1);//1是聊天
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
	// 通过 TimeOutProc 继承
	void Proc() override
	{
		ZinxKernel::Zinx_Exit();
	}
	int getTimeSec() override
	{
		//玩家全部下线10s后关闭服务器
		return 10;
	}
};
//实例化一个定时器全局对象
static ExitTimer exitTimer;

bool GameRole::Init()
{
	//如果线上没玩家，本玩家作为第一个玩家上线Init，删除退出定时器
	if (ZinxKernel::Zinx_GetAllRole().size() <= 0) {
		TimeOutManager::getInstance().delTask(&exitTimer);
	}


	//添加自己到游戏世界
	bool bRet =false;
	playerPid = m_proto->m_channel->GetFd();//设置玩家ID为当前连接的fd,获取文件描述符，这个是唯一的
	bRet = gameWorld.AddPlayer(this);

	if (bRet == true) {
		//向自己发送ID和名称
		GameMsg* pMsg = CreateIDNameLogin();//auto
		ZinxKernel::Zinx_SendOut(*pMsg, *m_proto);

		//向自己发送周围玩家位置
		pMsg = CreateSurPlayers();
		ZinxKernel::Zinx_SendOut(*pMsg, *m_proto);

		//向周围玩家发送自己的位置
		std::list<Player*> surPlayers = gameWorld.GetSurroundPlayers(this);//auto

		//player*
		for (auto singlePlayer : surPlayers) {
			pMsg = CreateSelfPosition();

			auto pRole = dynamic_cast<GameRole*>(singlePlayer);
			ZinxKernel::Zinx_SendOut(*pMsg, *(pRole->m_proto));
		}
	}

	//记录当前姓名到redis的Game_name
	//1连接redis
	redisContext* conn = redisConnect("127.0.0.1", 6379);
	//2发送lpush命令
	if (conn != NULL) {
		redisReply* reply = (redisReply*)redisCommand(conn, "rpush usr_name %s", playerName.c_str());
		freeReplyObject(reply);
		redisFree(conn);
	}
	return bRet;
}
//处理游戏相关的用户请求
UserData* GameRole::ProcMsg(UserData& _poUserData)
{
	//从协议层获得转换成16进制后的信息，是MutiMsg类型
	GET_REF2DATA(MultiMsg, input, _poUserData);
	for (auto singleGameMsg : input.m_Msgs) {
		//调试信息
		//这里pMsg为protobuf类型
		std::cout << "type is " << singleGameMsg->enMsgType << std::endl;
		std::cout << singleGameMsg->pMsg->Utf8DebugString() << std::endl;

		//定义positon指针来获取坐标（不能在switch中内部定义）
		//pb::Position* pNewPosition = dynamic_cast<pb::Position*>(singleGameMsg->pMsg);
		//switch (singleGameMsg->enMsgType)
		//{
		//	//把聊天消息发给所有玩家
		//case:GameMsg::MSG_TYPE_CHAT_CONTENT

		//default:
		//	break;
		//}
	}
	return nullptr;
}

void GameRole::Fini()
{
	//向周围玩家发送下线的消息
	std::list<Player*> surPlayerList = gameWorld.GetSurroundPlayers(this);

	//Player*
	for (auto singlePlayer : surPlayerList) {
		GameMsg* pMsg = CreateIDNameLogoff();
		GameRole* pRole = dynamic_cast<GameRole*>(singlePlayer);
		ZinxKernel::Zinx_SendOut(*pMsg, *(pRole->m_proto));
	}
	gameWorld.DelPlayer(this);

	//最后一个玩家下线后，开始计时，20s没有新玩家上线就退出服务器
	if (ZinxKernel::Zinx_GetAllRole().size() <= 0) {
		TimeOutManager::getInstance().addTask(&exitTimer);
	}

	//从redis中删除下线玩家姓名
	redisContext* conn = redisConnect("127.0.0.1", 6379);
	if (NULL != conn) {
		redisReply* reply = (redisReply*)redisCommand(conn, "lrem usr_name 1 %s", playerName.c_str());
		freeReplyObject(reply);
		redisFree(conn);
	}

}


//实现Player类的纯虚函数
int GameRole::GetX()
{
	return (int)x;
}

int GameRole::GetY()
{
	return (int)z;//客户端定义y为高，z为纵坐标
}
