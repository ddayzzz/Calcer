#pragma once
#include "base0.h"
#include <regex>
#include <string>
#include <vector>
#include <functional>
//这个头文件用来展示
using namespace std;
const char *title = "计算器。版权所有©2017。保留所有权利。\n";
const char *usage = "#define <ID> <Expression>";
class Expression
{
private:
	std::string _fcnname_or_expr;
	ExpNodeBase *_expTree = nullptr;
	FunctionInfo *_fptr = nullptr;
	ValueTypeBase *_result = nullptr;
	bool _isfcn;
	bool _isunk;
	//支持带有参数以及指定函数名的表达式
	void do_specify_unknown(std::vector<ExpNodeBase*> *real_para, std::vector<Log>* log)
	{
		if (_isfcn == false)
		{
			if (_isunk == false)
			{
				//非函数 且不带参数 直接计算 不可能需要重新计算
				if (_expTree == nullptr)
				{
					_expTree = Transfer_ExpToExpTree(_fcnname_or_expr, 0, _fcnname_or_expr.size() - 1, nullptr, log);
				}
				if (_result == nullptr && _expTree)
				{
					_result = _expTree->eval(log);
				}
			}
			else
			{
				//非函数 但是带参数 不存在，每一个带有未知数的表达式必须存在与函数列表中
				//
			}
		}
		else
		{
			if (_fptr == nullptr)
			{
				_fptr = getFunctionInfo(_fcnname_or_expr);
			}
			//函数 可能带有或者是不带参数 如果不带参数，就不要多次计算
			if (_fptr && _expTree && _result && _isunk == false)
				return;
			if (!_fptr)
				return;//function not fount
			if (_expTree == nullptr)
				_expTree = do_realUnkown_Function(_fcnname_or_expr, real_para, real_para->size(), log);
			if (_expTree)
			{
				_result = _expTree->eval(log);
				int j = 75;
			}
		}
	}
public:
	//指定函数名或者是表达式
	Expression(const std::string &fcnname_or_expr, bool isunk, bool isfcn) :_fcnname_or_expr(fcnname_or_expr), _isfcn(isfcn), _isunk(isunk)
	{
		//需要获取函数的表达式
	}
	//开始计算
	ValueTypeBase *Eval(std::vector<ExpNodeBase*> *real_para = nullptr, std::vector<Log>* log = nullptr)
	{
		do_specify_unknown(real_para, log);
		if (_result)
		{
			return _result;
		}
		return _result;
	}
	~Expression()
	{
		if (_expTree)
		{
			delete _expTree;
			_expTree = nullptr;
		}
	}
};
typedef bool(TrimErasePredType)(char);//这个是默认的删除特定子符的比较函数的定义，如果可调用对象返回true，则删除
std::string string_trim(const std::string &str,std::function<TrimErasePredType> pred)
{
	int u = str.size() - 1;
	int ll, uu;
	for (ll = 0; ll < str.size() && pred(str[ll]); ++ll)
		;
	for (uu = str.size()-1; uu >= ll && str[uu] == pred(str[ll]); --uu)
		;
	return str.substr(ll, uu-ll+1);
}

const auto TRIM_SPACECHAR=[](const char &c)->bool {return c == ' ' || c == '\t'; };//去掉多余的空白字符
static std::regex re_func_parameter_functionname("^[a-zA-Z]+[a-zA-Z0-9]*$");

bool vaild_parameter(const std::string &para)
{
	return std::regex_search(para, re_func_parameter_functionname);

}
bool vaild_functionname(const std::string &para)
{
	return std::regex_search(para, re_func_parameter_functionname);

}
void resolve_define_function(const std::string &expr)
{
	//需要解决：检查未知数的唯一性，然后检查函数的签名（名字，参数数量（目前不支持可变长））
	//			expr需要trim一下 expr统一再输入的时候去掉 OK
	//			空白字符不好定义 可以是' ' '\t'等等  OK
	//expr的形式
	// func myfun(x,y,z) x+y+z 但是参数定义的期间余姚忽略空格
	//解析函数名：
	size_t first_kuohao = expr.find_first_of('(');
	if (first_kuohao == std::string::npos)
	{
		//error
	}
	std::string func_name = string_trim(expr.substr(4, first_kuohao - 4), TRIM_SPACECHAR);
	//检查函数名的合法性
	if (vaild_functionname(func_name) == false)
		;//error
	//解析定义的未知数
	int index_of_lastkuhao = do_transfer_getLastKuohao(expr, first_kuohao, expr.size() - 1);
	if (index_of_lastkuhao == -1 || index_of_lastkuhao == expr.size() - 1/*不存在解析式*/)
	{
		//error
	}
	std::vector<std::pair<int, int>> unknown_list;
	do_transfer_spiltFunctionSubExp(expr, first_kuohao + 1, index_of_lastkuhao - 1, unknown_list);//解析参数列表
	int defined_unkown_count = unknown_list.size();
	//检查参数的合法性
	std::vector<std::string> vaild_parameters;
	std::string bechecked;
	for (int i = 0; i < unknown_list.size(); ++i)
	{
		bechecked = string_trim(expr.substr(unknown_list[i].first, unknown_list[i].second - unknown_list[i].first + 1),TRIM_SPACECHAR);
		if (vaild_parameter(bechecked))
			vaild_parameters.push_back(bechecked);
	}
	if (vaild_parameters.size() != unknown_list.size())
		;//error
	//解析后面的表达式
	std::string func_expr = string_trim(expr.substr(index_of_lastkuhao + 1, expr.size() - index_of_lastkuhao),TRIM_SPACECHAR);
	if (func_expr.size() <= 0)
		;//error
	FunctionInfo usd(func_name, std::string("用户定义的函数：").append(func_name), func_expr, vaild_parameters, false, USERDEFINED_FUNC_PRIORITY, USERDEFINED_FUNC_COMBINATION, vaild_parameters.size());
	functionlist.emplace(func_name, usd);
}