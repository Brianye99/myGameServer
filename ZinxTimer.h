#pragma once
#include "zinx.h"
#include<vector>
#include<list>
class ZinxTimer :
    public Ichannel
{
    // ͨ�� Ichannel �̳�
    bool Init() override;
    bool ReadFd(std::string& _input) override;
    bool WriteFd(std::string& _output) override;
    void Fini() override;
    int GetFd() override;
    std::string GetChannelInfo() override;
    AZinxHandler* GetInputNextStage(BytesMsg& _oInput) override;
public:
    int m_timerFd = -1;
};


//���崿��������
class TimeOutProc {
public:
    virtual void Proc() = 0;
    virtual int getTimeSec() = 0;
    //������ʣȦ��
    int iCount = -1;
};

//��ʱ��������
class TimeOutManager :public AZinxHandler {
    //����ģʽ1�����캯��˽�л�
    //������̬��Ա�����洢��Ψһ����
    TimeOutManager();
    virtual ~TimeOutManager();
    static TimeOutManager Single;
    //����ʱ����
    std::vector<std::list<TimeOutProc*>> mTimerWheel;
    //��ǰ�̶����ĸ�����
    int curTooth = 0;
    //��,ʮ����
    int tooth = 10;
public:
    // ͨ�� AZinxHandler �̳�
    IZinxMsg* InternelHandle(IZinxMsg& _oInput) override;
    AZinxHandler* GetNextHandler(IZinxMsg& _oNextMsg) override;

    void addTask(TimeOutProc* p_task);
    void delTask(TimeOutProc* p_task);
    //����ģʽ2���ṩ��ȡ�����Ľӿ�(��������
    static TimeOutManager& getInstance() {
        return Single;
    }
};