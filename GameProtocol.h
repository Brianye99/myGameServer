#pragma once
#include "zinx.h"

class GameChannel;
class GameRole;
class GameProtocol :
    public Iprotocol
{
    std::string szLast;//上次未来的及处理的报文
public:
    GameChannel* m_channel = NULL;
    GameRole * m_Role = NULL;
    GameProtocol();
    virtual ~GameProtocol();



    // 通过 Iprotocol 继承
    UserData* raw2request(std::string _szInput) override;
    std::string* response2raw(UserData& _oUserData) override;
    Irole* GetMsgProcessor(UserDataMsg& _oUserDataMsg) override;
    Ichannel* GetMsgSender(BytesMsg& _oBytes) override;
};

