#include "demo.h"

int main()
{
	resolve_define_function("func kk(x,y) sin(x+y)");
	//init_RE(std::string());
	
	init_functionlist();
	init_constslist();
	std::string s = "sin(x+y)";
	std::vector<Log> lo;
	//auto p = Transfer_ExpToExpTree(s, 0, s.size() - 1,nullptr,&lo);
	//auto j = BuiltInFuntionCalc(nullptr, 2, nullptr);
	//��һ�����
	Expression ep("kk",true,true);
	Number *nn = new Number(2.33);
	Number *ss = new Number(58);
	std::vector<ExpNodeBase*> par{ nn ,ss};
	auto reee = ep.Eval(&par,&lo);
	std::cout << reee->getDisplayName() << std::endl;
	//�ڶ������
	Expression ep1("kk", true, true);
	Number *aa = nn->clone();
	Number *bb = ss->clone();
	std::vector<ExpNodeBase*> par1{ aa,bb };
	auto jk = ep1.Eval(&par1, &lo);
	std::cout << jk->getDisplayName() << std::endl;
	//�������ڴ��ظ��ͷ�
	//ExpNodeBase* nj = new Number(j->asLongDouble().first);
	//ExpNodeBase *nnn = new Number(*nn);
	//Expression ep2("kk", true, true);
	//std::vector<ExpNodeBase*> par{ j->clone(),nn->clone() };
	//auto i=ep2.Eval(&par, &lo);
	//ep2.~Expression();
	ep.~Expression();
	ep1.~Expression();
	return 0;
}