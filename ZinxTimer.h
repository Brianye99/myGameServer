#pragma once
#include "zinx.h"
#include<vector>
#include<list>
class ZinxTimer :
    public Ichannel
{
    // 通过 Ichannel 继承
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


//定义纯虚来管理
class TimeOutProc {
public:
    virtual void Proc() = 0;
    virtual int getTimeSec() = 0;
    //任务所剩圈数
    int iCount = -1;
};

//定时器管理类
class TimeOutManager :public AZinxHandler {
    //单例模式1：构造函数私有化
    //声明静态成员变量存储，唯一单例
    TimeOutManager();
    virtual ~TimeOutManager();
    static TimeOutManager Single;
    //创建时间轮
    std::vector<std::list<TimeOutProc*>> mTimerWheel;
    //当前刻度在哪个齿上
    int curTooth = 0;
    //齿,十个齿
    int tooth = 10;
public:
    // 通过 AZinxHandler 继承
    IZinxMsg* InternelHandle(IZinxMsg& _oInput) override;
    AZinxHandler* GetNextHandler(IZinxMsg& _oNextMsg) override;

    void addTask(TimeOutProc* p_task);
    void delTask(TimeOutProc* p_task);
    //单例模式2：提供获取单例的接口(引用类型
    static TimeOutManager& getInstance() {
        return Single;
    }
};