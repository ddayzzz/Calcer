#pragma once
#include "base0.h"
#include <map>
/*模拟一个局部作用域的的变量管理*/
class VarManager
{
private:
	std::map<std::string, ValueTypeBase*> _temp_vars;//临时的变量
	std::map<std::string, ValueTypeBase*> _global_vars;//全局的变量
public:

};