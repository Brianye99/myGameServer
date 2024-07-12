#pragma once
#include <zinx.h>
#include "GameMsg.h"
#include "msg.pb.h"
#include "AOIWorld.h"


class GameProtocol;//不用到类里成员(属性、函数...)时后可以只声明一下
class GameRole :
    public Irole,public Player
{
    float x = 0;
    float y = 0;//客户端定义的y为高
    float z = 0;
    float v = 0;
    int playerPid = 0;
    std::string playerName;

    //发送聊天给所有人
    void ProcTalking(std::string _content);
    //广播位置给周围玩家
    void ProcMoveMsg(float _x, float _y, float _z, float _v);
    //视野出现
    void ViewAppear(GameRole* _pRole);
    //视野消失
    void ViewLost(GameRole* _pRole);

    //创建信息
    GameMsg* CreateIDNameLogin();
    GameMsg* CreateSurPlayers();
    GameMsg* CreateSelfPosition();
    GameMsg* CreateIDNameLogoff();
    //广播
    GameMsg* CreatTalkBroadCast(std::string _content);

public:
    GameRole();
    virtual ~GameRole();


    // 通过 Irole 继承
    bool Init() override;

    UserData* ProcMsg(UserData& _poUserData) override;

    void Fini() override;

    GameProtocol* m_proto = NULL;

    // 通过 Player 继承
    virtual int GetX() override;
    virtual int GetY() override;

};

