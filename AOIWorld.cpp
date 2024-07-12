#include "AOIWorld.h"

AOIWorld::AOIWorld(int _xBegin, int _xEnd, int _yBegin, int _yEnd, int _xCount, int _yCount)
	:xBegin(_xBegin), xEnd(_xEnd), yBegin(_yBegin), yEnd(_yEnd), xCount(_xCount), yCount(_yCount)
{
	//x���������=��x�ܽ�������-x����ʼ���꣩/x��������
	xWidth = (xEnd - xBegin) / xCount;
	yWidth = (yEnd - yBegin) / yCount;
	//������������
	for (int i = 0; i < xCount * yCount; i++) {
		Grid tmp;
		mGridsVector.push_back(tmp);
	}
}

AOIWorld::~AOIWorld()
{
}

std::list<Player*> AOIWorld::GetSurroundPlayers(Player* _player)
{
	std::list<Player*> listRet;
	//����������
	int gridID = (_player->GetX() - xBegin) / xWidth + (_player->GetY() - yBegin) / yWidth * xCount;
	//x�������������������ĵڼ���-1
	int xIndex = gridID % xCount;
	//y�������������������ĵڼ���-1
	int yIndex = gridID / xCount;
	//���������ϵĸ��������ж�
	//���ϽǸ���
	if (xIndex > 0 && yIndex > 0) {
		//�����ϽǸ����������б����뵽listRet������
		std::list<Player*> curPlayerList = mGridsVector[gridID - 1 - xCount].mPlayersList;
		listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());
	}
	//���Ϸ�����
	if (yIndex > 0) {
		//�����Ϸ������������б����뵽listRet������
		std::list<Player*>& curPlayerList = mGridsVector[gridID - xCount].mPlayersList;
		listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());
	}
	//���Ϸ�����
	if (xIndex < xCount - 1 && yIndex > 0) {
		std::list<Player*>& curPlayerList = mGridsVector[gridID + 1 - xCount].mPlayersList;
		listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());
	}
	//����߸���
	if (xIndex > 0) {
		std::list<Player*>& curPlayerList = mGridsVector[gridID - 1].mPlayersList;
		listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());
	}

	//�Լ��ĸ���(�Լ�Ҳ����Χ���)
	std::list<Player*>& curPlayerList = mGridsVector[gridID].mPlayersList;
	listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());

	//���ұ߸���
	if (xIndex < xCount - 1) {
		std::list<Player*>& curPlayerList = mGridsVector[gridID + 1].mPlayersList;
		listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());
	}
	//���½Ǹ���	
	if (xIndex > 0 && yIndex < yCount - 1) {
		std::list<Player*>& curPlayerList = mGridsVector[gridID - 1 + xCount].mPlayersList;
		listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());
	}
	//���·�����
	if (yIndex < yCount - 1) {
		std::list<Player*>& curPlayerList = mGridsVector[gridID + xCount].mPlayersList;
		listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());
	}
	//���½Ǹ���
	if (xIndex < xCount - 1 && yIndex < yCount - 1) {
		std::list<Player*>& curPlayerList = mGridsVector[gridID + 1 + xCount].mPlayersList;
		listRet.insert(listRet.begin(), curPlayerList.begin(), curPlayerList.end());
	}
	return listRet;
}

bool AOIWorld::AddPlayer(Player* _player)
{
	//������=(x-x����ʼ����)/x��������� + (y-y����ʼ����)/y�����*x���������� 
	//����������
	int gridID = (_player->GetX() - xBegin) / xWidth + (_player->GetY() - yBegin) / yWidth * xCount;
	//���ӵ�������
	mGridsVector[gridID].mPlayersList.push_back(_player);
	return true;
}

void AOIWorld::DelPlayer(Player* _player)
{
	//����������
	int gridID = (_player->GetX() - xBegin) / xWidth + (_player->GetY() - yBegin) / yWidth * xCount;
	//ժ�����
	mGridsVector[gridID].mPlayersList.remove(_player);
}