#include "ZinxTimer.h"
#include <sys/timerfd.h>
#include <vector>
#include <list>

TimeOutManager::TimeOutManager()
{
	//创建十个齿
	for (int i = 0; i < 10; i++) {
		std::list<TimeOutProc*> tmp;
		mTimerWheel.push_back(tmp);
	}
}

TimeOutManager::~TimeOutManager()
{
}
//定时器通道类
//创建文件描述符
bool ZinxTimer::Init()
{
	bool Ret = false;
	int ifd = timerfd_create(CLOCK_MONOTONIC, 0);//创建定时器，失败返回-1
	if (ifd >= 0) {
		//?
		struct itimerspec timerValue = { {1,0} ,{1,0} };//初始到期时间为1s，间隔时间为1s
		if (timerfd_settime(ifd, 0, &timerValue, NULL) == 0) {
			Ret = true;
			m_timerFd = ifd;
		}

	}
	return Ret;
}
//读取超时次数
bool ZinxTimer::ReadFd(std::string& _input)
{
	bool Ret = false;
	char buf[8] = "";
	if (sizeof(buf) == read(m_timerFd, buf, sizeof(buf))) {
		Ret = true;
		//char类型写入string
		_input.assign(buf, sizeof(buf));
	}
	return Ret;
}

bool ZinxTimer::WriteFd(std::string& _output)
{
	return false;
}
//关闭文件描述符
void ZinxTimer::Fini()
{
	close(m_timerFd);
	m_timerFd = -1;//防止之后程序访问
}
//返回当前定时器的文件描述符
int ZinxTimer::GetFd()
{
	return m_timerFd;
}

std::string ZinxTimer::GetChannelInfo()
{
	return "TimerFd";
}
//返回处理超时事件的对象
AZinxHandler* ZinxTimer::GetInputNextStage(BytesMsg& _oInput)
{
	return &TimeOutManager::getInstance();
}
//定时器管理类
//单例模式3：类加载时创建实例
TimeOutManager TimeOutManager::Single;
IZinxMsg* TimeOutManager::InternelHandle(IZinxMsg& _oInput)
{
	//八字节无符号数
	unsigned long iTimeoutCount = 0;//超时次数
	GET_REF2DATA(BytesMsg, obytes, _oInput);

	//obytes是八字节string，一个一个字节拷贝过来
	obytes.szData.copy((char*)&iTimeoutCount, sizeof(iTimeoutCount), 0);
	while ((iTimeoutCount--) > 0) {//为了防止子函数执行时间过长导致定时器少执行
		//移动当前的刻度
		curTooth++;
		curTooth %= 10;//时间齿环
		std::list<TimeOutProc*>m_cache;
		//遍历齿中的任务列表
		//1.若圈数为0，执行任务，摘除本节点，重新添加
		//2.否则，圈数-1
		//用迭代器遍历，而不用based for原因：
		//要摘除任务并移动，摘除任务时列表会断，遍历不下去了。
		for (auto itr = mTimerWheel[curTooth].begin(); itr != mTimerWheel[curTooth].end();) {
			if ((*itr)->iCount <= 0) {
				//把执行函数缓存起来最后统一执行，防止执行函数对遍历产生冲突（比如删除自己）
				m_cache.push_back((*itr));//缓存执行任务列表
				auto p_tmp = *itr;//存起来未删除的itr之，防止溢出
				itr = mTimerWheel[curTooth].erase(itr);//删除当前itr后，返回下一个itr
				addTask(p_tmp);
			}
			else {
				(*itr)->iCount--;
				++itr;//不在for里面++，删除后会溢出
			}//统一执行任务
			for (auto task : m_cache) {
				task->Proc();
			}
		}

	}
	return nullptr;
}

AZinxHandler* TimeOutManager::GetNextHandler(IZinxMsg& _oNextMsg)
{
	return nullptr;
}
//管理类的添加任务函数
void TimeOutManager::addTask(TimeOutProc* p_task)
{
	//如果先计算偏移量后相加会溢出
	//计算插入到哪个齿
	int insertTooth = (p_task->getTimeSec() + curTooth) % 10;
	//插入
	mTimerWheel[insertTooth].push_back(p_task);
	//计算圈数
	p_task->iCount = p_task->getTimeSec() / 10;
}
//管理类的删除任务函数
void TimeOutManager::delTask(TimeOutProc* p_task)
{
	//引用为了同步修改原始值
	for (std::list < TimeOutProc*> &task_list: mTimerWheel){
		for (TimeOutProc* task : task_list) {
			if (task == p_task) {
				//摘掉指定的task
				task_list.remove(task);
				//返回调用函数的位置
				return;
			}
		}
	}
}
