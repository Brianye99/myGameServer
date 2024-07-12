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
	//取名
	auto num = PoolRandomEngine() % mNamePool.size();
	std::string lastName = mNamePool[num]->mLastName;
	//取性
	auto randomIndex = PoolRandomEngine() % mNamePool[num]->mFirstNameVector.size();
	std::string firstName = mNamePool[num]->mFirstNameVector[randomIndex];
	//若本性的所有名都取完了，把性删掉
	if (mNamePool[num]->mFirstNameVector.size() <= 0) {
		delete mNamePool[num];
		mNamePool.erase(mNamePool.begin() + num);
	}
	return lastName + " " + firstName;
}

void RandomName::releaseName(std::string _name)
{
	//分割名字
	auto space_pos = _name.find(" ", 0);
	auto last = _name.substr(0, space_pos);
	auto first = _name.substr(space_pos + 1, _name.size() - space_pos - 1);

	bool found = false;
	//节点没被删除
	for (NamePoolNode* singleNode : mNamePool) {
		if (last == singleNode->mLastName) {
			found = true;
			singleNode->mFirstNameVector.push_back(last);
		}
	}
	//节点已经删除,创建节点
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
	std::fstream LastName("random_last.txt", std::ios::in);//性
	if (!LastName.is_open()) { perror("LastName error"); }
	std::fstream FirstName("random_first.txt", std::ios::in);//名
	if (!FirstName.is_open()) { perror("FIrstName error"); }

	//截取的时候根据文件内不同可能会截取到\n或者空格
	//   geline默认以\n分割，并且会把\n读入
	//   >>读的时候默认以空格分割，并且不读入空格

	//读取所有名字组成一个线性表
	while (FirstName >> tmpName) {
		tmpFirstNameVector.push_back(tmpName);
	}
	//读取所有性，创建姓名池节点，拷贝名字组成的线性表
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
