#pragma once
#include "base0.h"
#include <map>
/*ģ��һ���ֲ�������ĵı�������*/
class VarManager
{
private:
	std::map<std::string, ValueTypeBase*> _temp_vars;//��ʱ�ı���
	std::map<std::string, ValueTypeBase*> _global_vars;//ȫ�ֵı���
public:

};