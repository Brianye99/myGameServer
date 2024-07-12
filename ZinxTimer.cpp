#include "ZinxTimer.h"
#include <sys/timerfd.h>
#include <vector>
#include <list>

TimeOutManager::TimeOutManager()
{
	//����ʮ����
	for (int i = 0; i < 10; i++) {
		std::list<TimeOutProc*> tmp;
		mTimerWheel.push_back(tmp);
	}
}

TimeOutManager::~TimeOutManager()
{
}
//��ʱ��ͨ����
//�����ļ�������
bool ZinxTimer::Init()
{
	bool Ret = false;
	int ifd = timerfd_create(CLOCK_MONOTONIC, 0);//������ʱ����ʧ�ܷ���-1
	if (ifd >= 0) {
		//?
		struct itimerspec timerValue = { {1,0} ,{1,0} };//��ʼ����ʱ��Ϊ1s�����ʱ��Ϊ1s
		if (timerfd_settime(ifd, 0, &timerValue, NULL) == 0) {
			Ret = true;
			m_timerFd = ifd;
		}

	}
	return Ret;
}
//��ȡ��ʱ����
bool ZinxTimer::ReadFd(std::string& _input)
{
	bool Ret = false;
	char buf[8] = "";
	if (sizeof(buf) == read(m_timerFd, buf, sizeof(buf))) {
		Ret = true;
		//char����д��string
		_input.assign(buf, sizeof(buf));
	}
	return Ret;
}

bool ZinxTimer::WriteFd(std::string& _output)
{
	return false;
}
//�ر��ļ�������
void ZinxTimer::Fini()
{
	close(m_timerFd);
	m_timerFd = -1;//��ֹ֮��������
}
//���ص�ǰ��ʱ�����ļ�������
int ZinxTimer::GetFd()
{
	return m_timerFd;
}

std::string ZinxTimer::GetChannelInfo()
{
	return "TimerFd";
}
//���ش���ʱ�¼��Ķ���
AZinxHandler* ZinxTimer::GetInputNextStage(BytesMsg& _oInput)
{
	return &TimeOutManager::getInstance();
}
//��ʱ��������
//����ģʽ3�������ʱ����ʵ��
TimeOutManager TimeOutManager::Single;
IZinxMsg* TimeOutManager::InternelHandle(IZinxMsg& _oInput)
{
	//���ֽ��޷�����
	unsigned long iTimeoutCount = 0;//��ʱ����
	GET_REF2DATA(BytesMsg, obytes, _oInput);

	//obytes�ǰ��ֽ�string��һ��һ���ֽڿ�������
	obytes.szData.copy((char*)&iTimeoutCount, sizeof(iTimeoutCount), 0);
	while ((iTimeoutCount--) > 0) {//Ϊ�˷�ֹ�Ӻ���ִ��ʱ��������¶�ʱ����ִ��
		//�ƶ���ǰ�Ŀ̶�
		curTooth++;
		curTooth %= 10;//ʱ��ݻ�
		std::list<TimeOutProc*>m_cache;
		//�������е������б�
		//1.��Ȧ��Ϊ0��ִ������ժ�����ڵ㣬�������
		//2.����Ȧ��-1
		//�õ�����������������based forԭ��
		//Ҫժ�������ƶ���ժ������ʱ�б��ϣ���������ȥ�ˡ�
		for (auto itr = mTimerWheel[curTooth].begin(); itr != mTimerWheel[curTooth].end();) {
			if ((*itr)->iCount <= 0) {
				//��ִ�к��������������ͳһִ�У���ִֹ�к����Ա���������ͻ������ɾ���Լ���
				m_cache.push_back((*itr));//����ִ�������б�
				auto p_tmp = *itr;//������δɾ����itr֮����ֹ���
				itr = mTimerWheel[curTooth].erase(itr);//ɾ����ǰitr�󣬷�����һ��itr
				addTask(p_tmp);
			}
			else {
				(*itr)->iCount--;
				++itr;//����for����++��ɾ��������
			}//ͳһִ������
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
//����������������
void TimeOutManager::addTask(TimeOutProc* p_task)
{
	//����ȼ���ƫ��������ӻ����
	//������뵽�ĸ���
	int insertTooth = (p_task->getTimeSec() + curTooth) % 10;
	//����
	mTimerWheel[insertTooth].push_back(p_task);
	//����Ȧ��
	p_task->iCount = p_task->getTimeSec() / 10;
}
//�������ɾ��������
void TimeOutManager::delTask(TimeOutProc* p_task)
{
	//����Ϊ��ͬ���޸�ԭʼֵ
	for (std::list < TimeOutProc*> &task_list: mTimerWheel){
		for (TimeOutProc* task : task_list) {
			if (task == p_task) {
				//ժ��ָ����task
				task_list.remove(task);
				//���ص��ú�����λ��
				return;
			}
		}
	}
}
