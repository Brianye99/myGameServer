#pragma once
#include <zinx.h>
#include "GameMsg.h"
#include "msg.pb.h"
#include "AOIWorld.h"


class GameProtocol;//���õ������Ա(���ԡ�����...)ʱ�����ֻ����һ��
class GameRole :
    public Irole,public Player
{
    float x = 0;
    float y = 0;//�ͻ��˶����yΪ��
    float z = 0;
    float v = 0;
    int playerPid = 0;
    std::string playerName;

    //���������������
    void ProcTalking(std::string _content);
    //�㲥λ�ø���Χ���
    void ProcMoveMsg(float _x, float _y, float _z, float _v);
    //��Ұ����
    void ViewAppear(GameRole* _pRole);
    //��Ұ��ʧ
    void ViewLost(GameRole* _pRole);

    //������Ϣ
    GameMsg* CreateIDNameLogin();
    GameMsg* CreateSurPlayers();
    GameMsg* CreateSelfPosition();
    GameMsg* CreateIDNameLogoff();
    //�㲥
    GameMsg* CreatTalkBroadCast(std::string _content);

public:
    GameRole();
    virtual ~GameRole();


    // ͨ�� Irole �̳�
    bool Init() override;

    UserData* ProcMsg(UserData& _poUserData) override;

    void Fini() override;

    GameProtocol* m_proto = NULL;

    // ͨ�� Player �̳�
    virtual int GetX() override;
    virtual int GetY() override;

};

