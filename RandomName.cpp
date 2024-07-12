#include "RandomName.h"
#include <fstream>
#include<iostream>
#include<errno.h>
#include<random>

//#include <unistd.h>

//
static std::default_random_engine PoolRandomEngine(time(NULL));
RandomName::RandomName()
{
}

std::string RandomName::GetName()
{
	//ȡ��
	auto num = PoolRandomEngine() % mNamePool.size();
	std::string lastName = mNamePool[num]->mLastName;
	//ȡ��
	auto randomIndex = PoolRandomEngine() % mNamePool[num]->mFirstNameVector.size();
	std::string firstName = mNamePool[num]->mFirstNameVector[randomIndex];
	//�����Ե���������ȡ���ˣ�����ɾ��
	if (mNamePool[num]->mFirstNameVector.size() <= 0) {
		delete mNamePool[num];
		mNamePool.erase(mNamePool.begin() + num);
	}
	return lastName + " " + firstName;
}

void RandomName::releaseName(std::string _name)
{
	//�ָ�����
	auto space_pos = _name.find(" ", 0);
	auto last = _name.substr(0, space_pos);
	auto first = _name.substr(space_pos + 1, _name.size() - space_pos - 1);

	bool found = false;
	//�ڵ�û��ɾ��
	for (NamePoolNode* singleNode : mNamePool) {
		if (last == singleNode->mLastName) {
			found = true;
			singleNode->mFirstNameVector.push_back(last);
		}
	}
	//�ڵ��Ѿ�ɾ��,�����ڵ�
	if (!found) {
		NamePoolNode* newnode = new NamePoolNode();
		newnode->mLastName = last;
		newnode->mFirstNameVector.push_back(first);
		mNamePool.push_back(newnode);
	}
}

void RandomName::LoadFile()
{
	/*char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		std::cout << "Current working dir: " << cwd << std::endl;
	}
	else {
		perror("getcwd() error");
	}*/


	std::vector<std::string> tmpFirstNameVector;
	std::string tmpName;
	std::fstream LastName("random_last.txt", std::ios::in);//��
	if (!LastName.is_open()) { perror("LastName error"); }
	std::fstream FirstName("random_first.txt", std::ios::in);//��
	if (!FirstName.is_open()) { perror("FIrstName error"); }

	//��ȡ��ʱ������ļ��ڲ�ͬ���ܻ��ȡ��\n���߿ո�
	//   gelineĬ����\n�ָ���һ��\n����
	//   >>����ʱ��Ĭ���Կո�ָ���Ҳ�����ո�

	//��ȡ�����������һ�����Ա�
	while (FirstName >> tmpName) {
		tmpFirstNameVector.push_back(tmpName);
	}
	//��ȡ�����ԣ����������ؽڵ㣬����������ɵ����Ա�
	while (LastName >> tmpName) {
		NamePoolNode* namepoolNode = new NamePoolNode();
		namepoolNode->mLastName = tmpName;
		namepoolNode->mFirstNameVector = tmpFirstNameVector;
		mNamePool.push_back(namepoolNode);
	}

}

RandomName::~RandomName()
{
}
