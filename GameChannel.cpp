#include "GameChannel.h"
#include "GameRole.h"



GameChannel::GameChannel(int _fd) :ZinxTcpData(_fd) {

}

GameChannel::~GameChannel() {
	if (NULL != m_proto) {
		ZinxKernel::Zinx_Del_Proto(*m_proto);
		//手动释放协议对象
		delete m_proto;
	}
}

AZinxHandler* GameChannel::GetInputNextStage(BytesMsg& _oInput)
{
	return m_proto;
}

ZinxTcpData* GameConnFact::CreateTcpDataChannel(int _fd)
{
	//创建TCP通道对象
	auto pChannel = new GameChannel(_fd);
	//创建协议对象
	auto pProtocol = new GameProtocol();
	//创建玩家对象
	auto pRole = new GameRole();
	
	//绑定协议对象
	pChannel->m_proto = pProtocol;
	//绑定通道对象
	pProtocol->m_channel =pChannel;

	//绑定协议对象和玩家对象
	pProtocol->m_Role = pRole;
	pRole->m_proto = pProtocol;


	//将协议对象添加到kernel，参数为指针
	ZinxKernel::Zinx_Add_Proto(*pProtocol);
	//添加玩家对象
	ZinxKernel::Zinx_Add_Role(*pRole);

	return pChannel;
}
