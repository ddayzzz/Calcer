//这个函数解析器的定义
#include <queue>
#include <stack>
#include <string>
#include <utility>
#include <string>
#include <map>
#include <memory>
# include <iterator>
#include "json\json.h"
const long double PI = 3.1415926535898l;//PI常数
const long double DEGTORAD = PI / 180;//角度制转弧度制
//函数结合性的枚举类型：注意对于自定义函数不支持自定义结合性：因为我在处理自定义函数式将其视为一个计算节点的数字
enum class FcnCombine { Left, Right };
//代表类型的presentfor的类型
enum class PresentType{UNK,CONST};
//节点的类型 这是为了避免dynamic_cast的开销；
/*
NUM：数字类型；FCN：函数类型（非叶子）；PRE：代表类型（可以视为未知数、非数字类型和常量）；MARTX：矩阵类型
SET：集合类型；
*/
//定义用户函数的默认值
const int USERDEFINED_FUNC_PRIORITY = 0;//用户定义的函数的默认优先级
const FcnCombine USERDEFINED_FUNC_COMBINATION = FcnCombine::Left;//默认用户定义的函数的优先级
enum class ExpNodeBaseType {NUM,FCN,PRE,MATRIX,SET};
//日志定义的错误信息
enum class LogType {NONE,ERROR,WARNNING,UNKNOWN};
//定义的默认的数值的类型
typedef long double FloatType;//默认的浮点类型
typedef int Int32;//默认的整数类型
typedef int BoolType;//默认的布尔类型 布尔类型0为假 非0为真
struct ExpNodeBase;
struct ValueTypeBase;

//函数的参数的描述的信息
struct ParameterDetail {
	std::string _description;//他们的解释信息
	ExpNodeBase *_condition;//获取代数的值的时候判断是否满足条件（即是否符合定义域，一般而言这个需要返回0或者1，当然了这需要我的解析器zici）
	std::string _errorMsg;//当满足_condition的值为false时的出错消息
	ParameterDetail() = default;
	ParameterDetail(const std::string &des, const std::string &cond, const std::string error) :\
		_description(des), _errorMsg(error), _condition(nullptr) {}
};
//参数的基本需要包含的内容
struct Parameter {
	int _paraID;//参数的索引值 从0开始
	std::string _name;//参数的标识符
	ParameterDetail _detail;//详细的参数信息，用组合而不是继承
	Parameter(int id, const std::string &name):_paraID(id), _name(name),_detail(ParameterDetail()){}
	Parameter(int id, const std::string &name, const std::string &des, const std::string &cond, const std::string error) :\
		_paraID(id), _name(name), _detail(ParameterDetail(des,cond,error)) {}
};

//日志信息
struct Log {
	LogType _logtype;//日志信息的类型
	std::string _message;//日志的消息
	std::string _exp;//发生错误的表达式信息
	Log(LogType logtype, const std::string &info, const std::string &exp = std::string()) :_logtype(logtype), _message(info), _exp(exp) {}
	Log(LogType logtype, const char *msg, const char *exp = nullptr) :_logtype(logtype), _message(msg) { if (exp) _exp = exp; }
	//这个相当于默认的构造函数 默认为没有错误
	Log(LogType logtype = LogType::NONE) :_logtype(logtype) {}
};
//常量描述的信息
struct ExpNodeBase {
	/*
	ExpNodeBase *pChild;//第一个孩子指针
	ExpNodeBase *pBrother;//同一级别的第一个对象
	~ExpNodeBase() {}
	ExpNodeBase():pChild(nullptr),pBrother(nullptr){}
	*/
	//纯虚函数：返回表达式的有意义的值的指针
	virtual ValueTypeBase* eval(std::vector<Log>* log = nullptr) = 0;
	//动态识别类型：避免RTTI的开销
	virtual ExpNodeBaseType getType()
	{
		return ExpNodeBaseType::FCN;
	}
	ExpNodeBase* pChild;//第一个孩子指针
	ExpNodeBase* pBrother;//同一级别的第一个对象
	ExpNodeBase() : pChild(nullptr), pBrother(nullptr) {}
	virtual ~ExpNodeBase()
	{
		ExpNodeBase *pnode = pChild, *temp;
		while (pnode)
		{
			temp = pnode->pBrother;
			delete pnode;
			pnode = temp;
		}

	}
};
//表达式树的叶子类型
struct ValueTypeBase :public ExpNodeBase
{
	//如果式叶子结点，那么可以继承这些接口，如果不需要，继承默认的版本就可以了
	virtual std::string getDisplayName() { return std::string("没有定义的类型"); }

	virtual std::pair<FloatType, Log> asFloatType() const { return { FloatType(),Log(LogType::ERROR,"不支持将操作数转换为Long Double类型") }; };//转换为数值类型
	virtual std::pair<FloatType, Log> asInt32Type() const { return { FloatType(),Log(LogType::ERROR,"不支持将操作数转换为整数类型") }; };//转换为整数类型
	virtual std::pair<FloatType, Log> asBoolType() const { return { FloatType(),Log(LogType::ERROR,"不支持将操作数转换为布尔类型") }; };//转换为布尔类型

	virtual std::pair<ValueTypeBase*, Log> opt_add(ValueTypeBase* r) { return { nullptr,Log(LogType::ERROR,"操作数类型不支持操作符\'+\'") }; }//加法
	virtual std::pair<ValueTypeBase*, Log> opt_divide(ValueTypeBase* r) { return { nullptr,Log(LogType::ERROR,"操作数类型不支持操作符\'-\'") }; }//减法
	virtual std::pair<ValueTypeBase*, Log> opt_subtract(ValueTypeBase* r) { return { nullptr,Log(LogType::ERROR,"操作数类型不支持操作符\'*\'") }; }//乘法
	virtual std::pair<ValueTypeBase*, Log> opt_multiply(ValueTypeBase* r) { return { nullptr,Log(LogType::ERROR,"操作数类型不支持操作符\'/\'") }; }//除法
	virtual std::pair<ValueTypeBase*, Log> opt_reverse() { return { nullptr,Log(LogType::ERROR,"操作数类型不支持\'逆元操作\'") }; }//求逆元
	virtual ~ValueTypeBase() {}
	virtual ValueTypeBase *clone() = 0;//这个时克隆一个指针 会进行深拷贝，一次解决内存释放的问题
	//输出的值的类型
	virtual std::ostream &display_value(std::ostream &os) { return os; }
};
//函数表达式类：用来描述函数的表达式的要求，比如操作数对象要求，参数数量等
class FunctionInfo {
private:
	
public:
	std::vector<std::string> _signatures;//函数的签名 函数可能会有重复的签名
	std::string _expression;//函数的解析式
	std::string _FcnName;//函数的显示名称
	std::string _displayname;//显示的名称
	std::map<std::string, Parameter> _map_nameToInfo;//参数名字和到参数信息的映射
	bool _isbuiltin;//是否是内置的表达式
	int _priority;//函数的优先级
	FcnCombine _combine;//函数的结合性
	int _unkcount;//未知数的的数量
	FunctionInfo(const std::string &fcnname,const std::string &disname,std::string &exp,
		std::initializer_list<Parameter> parainit,bool builtin,int prior,FcnCombine comb,int unkcount):
		_priority(prior), _unkcount(unkcount),_expression(exp),_FcnName(fcnname),_displayname(disname),
		_isbuiltin(builtin),_combine(comb)
	{
		for (auto i =parainit.begin(); i != parainit.end(); ++i)
		{
			_map_nameToInfo.emplace(i->_name, *i);//根据参数的名字建立映射表
		}
	}
	//支持从已经给定的参数列表(vector存储的是未知数的名字，这个名字应该是唯一的)中插入函数，参数都是使用普通的Parameter(基本类型)构造的
	FunctionInfo(const std::string &fcnname, const std::string &disname, std::string &exp,
		const std::vector<std::string> &paraslist, bool builtin, int prior, FcnCombine comb, int unkcount) :
		_priority(prior), _unkcount(unkcount), _expression(exp), _FcnName(fcnname), _displayname(disname),
		_isbuiltin(builtin), _combine(comb)
	{
		int l = 0;
		for (; l < paraslist.size(); ++l)
		{
			_map_nameToInfo.emplace(paraslist[l], Parameter(l, paraslist[l]));
		}
	}
	void insertSignature(const char *signature)
	{
		_signatures.push_back(signature);
	}
};
//获取函数的信息返回FunctionInfo的指针，如果id不存在于全局函数表中，那么返回nullptr
FunctionInfo *getFunctionInfo(const std::string &id);
//根据表达式串e[w(lower),w[upper]，转换为表达式树。返回一个指针和日志信息。如果表达式是一个带有参数的表达式，那么请指定实参列表
ExpNodeBase *Transfer_ExpToExpTree(std::string e, int lower, int upper, std::map<std::string, Parameter>* = nullptr, std::vector<Log>* = nullptr);
struct Const {
	std::string _description;//常量的描述信息
	std::string _exp;//常量的表达式
	ValueTypeBase *_value = nullptr;//计算得到的表达式的值

	//这个是可以从表达式中载入的常量计算式 这样的话就需要析构函数
	Const(const std::string &exp, const std::string &des = std::string()) :_description(des), _exp(exp) {
		if (_exp.size() > 0)
		{
			auto res = Transfer_ExpToExpTree(_exp, 0, _exp.size() - 1);//常量的表达式不能包含变量
			if (res && res->eval())
			{
				_value = res->eval()->clone();
				delete res;
			}
		}
	}
	Const(ValueTypeBase *restree, const std::string &des = std::string()) :_value(restree->clone()), _exp(std::string()), _description(des)
	{

	}
	~Const()
	{
		if (_value)
			delete _value;
		_value = nullptr;
	}
	ValueTypeBase *getValueExp()
	{
		if (_value)
			return _value;
		else
			return nullptr;
	}
	//一定要深拷贝
	Const(const Const &rhs)
	{
		_description = rhs._description;
		_exp = rhs._exp;
		_value = rhs._value->clone();
	}
	Const &operator=(const Const &rhs)
	{
		_description = rhs._description;
		_exp = rhs._exp;
		_value = rhs._value->clone();
		return *this;
	}
};
//全局函数表
std::map<std::string, FunctionInfo> functionlist;
std::map<std::string, Const> global_constslist;//这个保存全局的常量列表
//这个是初始化的全局函数列表
void init_functionlist()
{
	Parameter p1(0,"x","左加数", "none", "error");
	Parameter p2(1,"y","右加数", "none", "error");
	//FunctionExpression fe(std::string("加法运算"),"22", true,"none", std::initializer_list<Parameter>{ p1,p2 });
	FunctionInfo finfo(std::string("+"), std::string("加法运算、集合并集运算符"), std::string("x+y"), { p1,p2 },true, 4, FcnCombine::Left, 2);
	finfo.insertSignature("Number(Number,Number)");
	finfo.insertSignature("Set(Set,Set)");
	functionlist.emplace("+", finfo);

	Parameter p3(0,"x","被减数", "none", "error");
	Parameter p4(1,"y","减数", "none", "error");
	FunctionInfo finfo1(std::string("-"), std::string("减法运算、集合差集运算符"), std::string("x-y"), { p3,p4 }, true, 4, FcnCombine::Left, 2);
	finfo1.insertSignature("Number(Number,Number)");
	finfo1.insertSignature("Set(Set,Set)");
	functionlist.emplace("-", finfo1);

	Parameter p5(0,"x", "参数1", "none", "error");
	Parameter p6(1,"y", "参数2", "none", "error");
	//Parameter p7(2,"z", "参数3", "none", "error");
	FunctionInfo finfo2(std::string("kk"), std::string("kk运算符"), std::string("x+y") ,{ p5,p6 },false, USERDEFINED_FUNC_PRIORITY, USERDEFINED_FUNC_COMBINATION, 2);
	finfo2.insertSignature("Number(Number,Number)");
	//functionlist.emplace("kk", finfo2);

	Parameter p7(0, "x", "角度（弧度制）", "none", "error");
	FunctionInfo finfo3(std::string("sin"), std::string("正弦函数"), std::string("sin(x)"), { p7 }, true, 4, FcnCombine::Left, 1);
	finfo3.insertSignature("Number(Number,Number)");
	functionlist.emplace("sin", finfo3);

	Parameter p8(0, "x", "表达式1", "none","error");
	Parameter p9(1, "y", "表达式2", "none", "error");
	//FunctionExpression fe(std::string("加法运算"),"22", true,"none", std::initializer_list<Parameter>{ p1,p2 });
	FunctionInfo finfo4(std::string("!="), std::string("不等于运算符"), std::string("x!=y"), { p1,p2 }, true, 7, FcnCombine::Left, 2);
	finfo4.insertSignature("Bool(Number,Number)");
	functionlist.emplace("!=", finfo4);
}
//初始化从文件中载入的全局常量列表
void init_constslist()
{
	//Const c1("3.1415926", "PI");//注意：如果这么写就会导致内存的释放
	global_constslist.emplace("PI", Const("3.1415926", "PI"));
	
}
Const *getConst(const std::string &constname)
{
	auto cfind = global_constslist.find(constname);
	if (cfind != global_constslist.end())
	{
		return &(cfind->second);
	}
	else
	{
		return nullptr;
	}
}
//表达式树的基本类型：拥有一个长子儿子节点和第一个同级的节点两个指针域：即树的双亲表示法



//从ValueTypeBase派生的叶子类型
class Number :public ValueTypeBase
{
private:
	FloatType _val = 0.0;
	void strTonNum(const std::string &s)
	{
		int TEN = 1;
		double DECMIAL = 0.1;
		bool nodec = true;
		int posofdec = s.find_first_of('.', 0);
		if (posofdec == std::string::npos)
			posofdec = s.size();
		int i;
		for (i = posofdec - 1; i >= 0; --i)
		{
			_val += (s[i] - '0')*TEN;
			TEN = TEN * 10;
		}
		for (posofdec = posofdec + 1; posofdec <= s.size() - 1; ++posofdec)
		{
			_val += (s[posofdec] - '0')*DECMIAL;
			DECMIAL /= 10;
		}

	}
public:
	Number(const std::string &s) { strTonNum(s); }
	Number(const FloatType &v) :_val(v) {}

	ValueTypeBase* eval(std::vector<Log>* log) override { return this; }
	ExpNodeBaseType getType() override
	{
		return ExpNodeBaseType::NUM;
	}
	std::string getDisplayName() override { return std::string("数字类型"); }
	std::pair<FloatType, Log> asFloatType() const override { return { _val,Log() }; }
	std::pair<ValueTypeBase*, Log> opt_add(ValueTypeBase* r) override
	{
		//return {new Number(_val + r.eval()->asLongDouble());
		if (r)
		{
			auto p = r->eval()->asFloatType();
			if (p.second._logtype == LogType::NONE)
			{
				return { new Number(_val + p.first),Log() };
			}
			else
			{
				return { nullptr,Log(LogType::ERROR,"右操作数不支持转换为Long Double类型") };
			}
		}
		return { nullptr,Log(LogType::ERROR,"右操作数不能为空") };
	}
	std::pair<ValueTypeBase*, Log> opt_subtract(ValueTypeBase* r) override
	{
		auto p = r->eval()->asFloatType();
		if (p.second._logtype == LogType::NONE)
		{
			return { new Number(_val - p.first),Log() };
		}
		else
		{
			return { nullptr,Log(LogType::ERROR,"右操作数不支持转换为Long Double类型") };
		}
	}
	std::pair<ValueTypeBase*, Log> opt_multiply(ValueTypeBase* r) override
	{
		//return {new Number(_val + r.eval()->asLongDouble());
		auto p = r->eval()->asFloatType();
		if (p.second._logtype == LogType::NONE)
		{
			return { new Number(_val * p.first),Log() };
		}
		else
		{
			return { nullptr,Log(LogType::ERROR,"右操作数不支持转换为Long Double类型") };
		}
	}
	std::pair<ValueTypeBase*, Log> opt_divide(ValueTypeBase* r) override
	{
		auto p = r->eval()->asFloatType();
		if (p.second._logtype == LogType::NONE)
		{
			if (p.first == 0.0l)
			{
				return {nullptr,Log(LogType::ERROR,"除数不能为0") };
			}
			return { new Number(_val / p.first),Log() };
		}
		else
		{
			return { nullptr,Log(LogType::ERROR,"右操作数不支持转换为Long Double类型") };
		}
	}
	std::pair<ValueTypeBase*, Log> opt_reverse() override
	{
			return { new Number(0.0l-_val),Log() };
	}
	std::ostream &display_value(std::ostream &os) override { return os<<_val; }
	//析构函数 但是没有什么可以释放的
	virtual ~Number(){}
	virtual Number *clone()
	{
		return new Number(*this);
	}
};

//从ValueTypeBase派生的叶子类型：存储整数的集合类型
class Set :public ValueTypeBase
{
private:
public:
	Set(std::initializer_list<ValueTypeBase*> init) 
	{ 
		//第一步：计算每一个元素的值 转换为INT32类型
		//第二步：使用位向量的形式存储
	}
	ValueTypeBase* eval(std::vector<Log>* log) override { return this; }
	ExpNodeBaseType getType() override
	{
		return ExpNodeBaseType::SET;
	}
	std::string getDisplayName() override { return std::string("集合类型"); }
	std::pair<ValueTypeBase*, Log> opt_add(ValueTypeBase* r) override
	{
		return { nullptr,Log(LogType::ERROR,"右操作数不支持转换为Long Double类型") };
	}
	std::pair<ValueTypeBase*, Log> opt_subtract(ValueTypeBase* r) override
	{

		return { nullptr,Log(LogType::ERROR,"右操作数不支持转换为Long Double类型") };

	}
	std::pair<ValueTypeBase*, Log> opt_multiply(ValueTypeBase* r) override
	{
		return { nullptr,Log(LogType::ERROR,"右操作数不支持转换为Long Double类型") };

	}
	//析构函数 但是没有什么可以释放的
	virtual ~Set() {}
	virtual Set *clone()
	{
		return new Set(*this);
	}
	virtual std::ostream &display_value(std::ostream &os) { return os; }
};
//这个是“代表”类型，“代表”类型可以代表数字、表达式树的根节点和其他可以存在于树上的类型
class Present :public ExpNodeBase {

public:
	int _id;//这个式当type=UNK的时候使用，表示的是未知数在函数的编号，从0开始的
	PresentType _type;
	ExpNodeBase* _presentfor;//指代的指针
	Present(PresentType type, ExpNodeBase *presentfor = nullptr, int id = 0) :_type(type), _id(id), _presentfor(presentfor) {}
	ExpNodeBaseType getType() override { return ExpNodeBaseType::PRE; }
	ValueTypeBase* eval(std::vector<Log>* log) override {
		if (_presentfor)
		{
			return _presentfor->eval();//返回被代表的指针
		}
		//error
		return nullptr;
	}
	virtual ~Present() override
	{
		if (_type == PresentType::CONST)
			return;//常量不会释放
		if (_presentfor)
		{
			delete _presentfor;
			_presentfor = nullptr;
		}
	}
};
//内置函数计算
//计算内置的函数 需要指定函数的信息，未知数的实际数量，参数（以长子节点开始，一次访问他的兄弟，默认从左往右计算）
ValueTypeBase* BuiltInFuntionCalc(const FunctionInfo *fptr, int realunk, ExpNodeBase *paras, std::vector<Log> *log = nullptr)
{
	//还需要优化一下
	if (!fptr)
	{
		if (log)
			log->emplace_back(LogType::ERROR, "没有指定函数指针类型");
		return nullptr;
	}
	if (realunk == 2)
	{
		//检查是否有足够地内容
		if (!(paras && paras->pBrother))
		{
			if (log)
				log->emplace_back(LogType::ERROR, "函数的参数与实际的参数不相符", fptr->_FcnName);
			return nullptr;
		}
		//二元函数
		std::pair<ValueTypeBase*, Log> calles;
		if (fptr->_FcnName == "+")
			calles = paras->eval()->opt_add(paras->pBrother->eval());
		else if (fptr->_FcnName == "-")
			calles = paras->eval()->opt_subtract(paras->pBrother->eval());
		else if (fptr->_FcnName == "/")
			calles = paras->eval()->opt_divide(paras->pBrother->eval());
		else if (fptr->_FcnName == "*")
			calles = paras->eval()->opt_multiply(paras->pBrother->eval());
		else
		{
			//unknown opts
			if (log)
				log->emplace_back(LogType::ERROR, std::string("运算符或函数\'").append(fptr->_FcnName).append("\'与实际参数数量不符或者是类型不兼容"));
			return nullptr;
		}
		if (calles.second._logtype == LogType::NONE)
			return calles.first;
		if (log)
			log->push_back(calles.second);
		return nullptr;
	}
	else
	{
		if (realunk == 1)
		{
			if (!paras)
			{
				if (log)
					log->emplace_back(LogType::ERROR, "函数的参数与实际的参数不相符", fptr->_FcnName);
				return nullptr;
			}
			//一元函数注意取反的问题 使用number的逆元操作
			std::pair<ValueTypeBase*, Log> calles;
			if (fptr->_FcnName == "-")
				calles = paras->eval()->opt_reverse();
			else if (fptr->_FcnName == "sin")
			{
				auto fir = paras->eval(log);
				if (fir == nullptr)
				{
					if (log)
						log->emplace_back(LogType::ERROR, "函数的参数与实际的参数不相符", fptr->_FcnName);
					return nullptr;
				}
				auto fcncalls = fir->asFloatType();
				if (fcncalls.second._logtype == LogType::NONE)
					return new Number(sinl(fcncalls.first));
				//error
				if (log) log->push_back(fcncalls.second);
				return nullptr;
				//不需要进行下次的判断
			}
			else
			{
				//unknown opts
				if (log)
					log->emplace_back(LogType::ERROR, std::string("运算符或函数\'").append(fptr->_FcnName).append("\'与实际参数数量不符或者是类型不兼容"));
				return nullptr;
			}
			if (calles.second._logtype == LogType::NONE)
				return calles.first;
			if (log)
				log->push_back(calles.second);
			return nullptr;
		}
	}
}
//函数、操作符类型
class Function:public ExpNodeBase {
private:
	//设置函数的信息指针
	void do_setFcnPtr()
	{
		if (fcnID.size()>0)
		{
			auto i = getFunctionInfo(fcnID);
			fcnPtr = i;
		}
	}
protected:
	FunctionInfo* fcnPtr;//函数的信息指针
	int realUnknown;//函数解析过程中的实际的参数 函数的实际的参数、参数类型决定于函数的计算行为
	std::string fcnID;//函数的唯一的标识名称的ID
	ValueTypeBase* result = nullptr;//用来保存计算的结果，这是一个堆上的内容，注意要释放。之所以这么做是因为需要保持多态的性质
public:
	ExpNodeBaseType getType() override { return ExpNodeBaseType::FCN; }
	FunctionInfo *getFcnExpPtr() { return fcnPtr; }
	Function(const std::string &fn):fcnID(fn){ do_setFcnPtr(); }
	Function(const char c) { fcnID.push_back(c); do_setFcnPtr();}
	virtual bool setParameterPtr(std::vector<ExpNodeBase*> &real,std::vector<Log>* =nullptr) = 0;//这个无论是否是自定义函数都需要这个
	virtual bool setParameterPtr(std::stack<ExpNodeBase*> &sta, std::vector<Log>* = nullptr) = 0;//操作数需要这个 使用栈模式的
	virtual ~Function() {}//因为UserFuntion的result指针和内置函数的是公用的 所以就需要下放释放操作，result的指针只由builtinfunction析构释放。因为userfunction和builtfunction通过fcnexpr关联
	
};
//用户定义的函数
/*
一般形式为fcn(x,y,x+z)这都是可以的，算法在执行的过程中，将会将函数的解析式进行计算然后保存在fcnExpr中，最后带入实参
*/
class UserFunction :public Function
{
private:
	ExpNodeBase *fcnExpr;//实际的函数的解析式 当不是内置函数的时候需要解析；内置则不管
	//构造树的辅助函数：带入函数的解析式的表达式树 修改present的未知数类型的present
	bool do_trasfer_presentRealParameter(std::vector<ExpNodeBase*> &real, ExpNodeBase *exptree,std::vector<Log> *log=nullptr)
	{
		ExpNodeBase* pchild = exptree->pChild;
		bool res=true;
		while (pchild)
		{
			if (pchild->getType() == ExpNodeBaseType::FCN)
			{
				//仍然式解析式
				res=res & do_trasfer_presentRealParameter(real, pchild,log);//那么进行下一层的递归
			}
			else
			{
				if (pchild->getType() == ExpNodeBaseType::NUM)
					;
				else
				{
					if (pchild->getType() == ExpNodeBaseType::PRE)
					{
						Present *pre = static_cast<Present*>(pchild);//dynamical
						if (pre)
						{
							if (pre->_type == PresentType::UNK)
							{
								if(pre->_id <real.size())
									pre->_presentfor = real[pre->_id];
								else
								{
									if (log) log->emplace_back(LogType::ERROR, "指定的参数不足");
									return false;
								}
							}
							else
							{
								if (pre->_type == PresentType::CONST)
									;//常量已经得到了值 这个我就不管了
							}
						}
					}
					else
					{
						if (log) log->emplace_back(LogType::ERROR, "未定义的类型");
						return false;
					}
				}
			}
			pchild = pchild->pBrother;
		}
		return res;
	}
	bool setParameterPtr(std::stack<ExpNodeBase*> &sta, std::vector<Log>*) { return false; }//仅仅定义 不需要
public:
	UserFunction(const std::string &fcnname):Function(fcnname),fcnExpr(nullptr){}
	ValueTypeBase* eval(std::vector<Log>* log) override {
		if (fcnExpr)
		{
			if (result == nullptr)
				result = fcnExpr->eval(log);
			return result;
		}
		else
		{
			//没有指定参数
			if (log)
				log->emplace_back(LogType::ERROR, "函数没有指定的表达式");
		}
		return nullptr;
	}
	
	bool setParameterPtr(std::vector<ExpNodeBase*> &real, std::vector<Log>* log)
	{
		if (fcnExpr==nullptr)
		{
			fcnExpr = Transfer_ExpToExpTree(fcnPtr->_expression, 0, fcnPtr->_expression.size() - 1, &fcnPtr->_map_nameToInfo);
			if (fcnExpr)
			{
				//update result
				if (result)
					delete result;//清除已经计算过的信息
				realUnknown = real.size();
				return do_trasfer_presentRealParameter(real, fcnExpr,log);
			}
			else
			{
				if (log)
					log->emplace_back(LogType::ERROR, "无法构建表达式树", fcnPtr->_expression);
				return false;
			}
		}
		else
		{
			return true;
		}
	}
	virtual ~UserFunction()
	{
		if (fcnExpr)
		{
			delete fcnExpr;
			fcnExpr = nullptr;
		}
	}
};
//内置的函数的定义；与用户的区别是不需要解析式，直接使用指针保存参数信息
class BuiltInFunction :public Function {
private:
	//ValueTypeBase* result = nullptr;//用来保存计算的结果，这是一个堆上的内容，注意要释放。之所以这么做是因为需要保持多态的性质
protected:
public:
	BuiltInFunction(const std::string &fcnid) :Function(fcnid) {}
	BuiltInFunction(char c) :Function(c) {}
	ValueTypeBase* eval(std::vector<Log>* log) override {
		if (result == nullptr)
		{
			result = BuiltInFuntionCalc(fcnPtr, realUnknown, pChild, log);
		}
		return result;

	}
	//这个主要用股userfunction的解析式的带入形参的过程
	bool setParameterPtr(std::vector<ExpNodeBase*> &real, std::vector<Log>* log) {
		int j;
		ExpNodeBase **pnode = &pChild;
		for (j = 0; j < real.size(); ++j)
		{
			*pnode = real[j];
			pnode = &((*pnode)->pBrother);
		}
		realUnknown = j;
		//动态解析参数的类型
		return true;
	}
	//这个主要用于解析表达式的时候，用的是栈
	bool setParameterPtr(std::stack<ExpNodeBase*> &sta, std::vector<Log>* log)
	{
		auto fptr = getFunctionInfo(fcnID);
		int unk = fptr->_unkcount;
		if (fptr)
		{
			ExpNodeBase **pnode = &pChild;
			int j;
			if (fptr->_combine == FcnCombine::Left)
			{
				j = 0;
				std::stack<ExpNodeBase*> rever;
				for (; !sta.empty() && j < unk; ++j)
				{
					rever.push(sta.top());
					sta.pop();
				}
				while (!rever.empty())
				{
					*pnode = rever.top();
					rever.pop();
					pnode = &((*pnode)->pBrother);
				}
			}
			else
			{
				for (j = 0; j < unk && sta.size(); ++j)
				{
					*pnode = sta.top();
					sta.pop();
					pnode = &((*pnode)->pBrother);
				}
			}
			realUnknown = j;//保存实际的参数
			return true;
		}
		else
		{
			if (log)
				log->emplace_back(LogType::ERROR, "无法解析函数信息：没有有效的函数信息指针");
			return false;
		}
	}
	virtual ~BuiltInFunction()
	{
		if (result)
		{
			delete result;
			result = nullptr;
		}
	}
	virtual void setResultPtr(ValueTypeBase *res) { result = res; }
}; 

FunctionInfo *getFunctionInfo(const std::string &id)
{
	auto i = functionlist.find(id);
	if (i != functionlist.end())
	{
		return &(*i).second;
	}
	else
	{
		return nullptr;
	}
}
//获取子表达式的结尾位置即右括号的位置   beg表示的是左括号位置 -1表示失败
int do_transfer_getLastKuohao(const std::string &exp, int beg, int u)
{
	int tra = 0;
	for (; beg <= u; ++beg)
	{
		if (exp[beg] == '(')
			++tra;
		if (exp[beg] == ')')
			--tra;
		if (tra == 0)
			break;
	}
	if (beg > u)
		return -1;
	return beg;
}
//拆分子表达式项（用于函数解析）如果失败：清空poss 例如 fcn(x+3,u+2) 函数返回 x+3   u+2的起点终点坐标 l是左括号+1 u是右括号的位置-1
void do_transfer_spiltFunctionSubExp(const std::string &exp, int l, int u, std::vector<std::pair<int, int>> &poss)
{
	//注意子表达式也可能包含函数调用
	int prePos = l;//前一个子表达式的终止位置的下一个的下一个 即下一个子表达式的起始位置
	for (; l <= u;)
	{
		int temp = l;
		if (exp[temp] == ',')
		{
			poss.emplace_back(prePos, temp - 1);
			l=prePos = temp + 1;
		}
		else
		{
			if (exp[temp] == '(')
			{
				//存在子表达式
				int lastkuohao = do_transfer_getLastKuohao(exp, temp, u);
				if (lastkuohao == -1)
				{
					poss.clear();
					return;
				}
				if (lastkuohao == u || (lastkuohao < u && exp[lastkuohao + 1] == ','))
				{
					//找到了一个子表达式
					poss.emplace_back(prePos, lastkuohao);
					l = prePos = lastkuohao + 2;
				}
				else
				{
					//并不式是完整的表达式 所以需要继续计算不要更新prepos
					l = lastkuohao + 1;
					continue;
				}
			}
			else
			{
				++l;
			}
		}

	}
	if(prePos<=u)
		poss.emplace_back(prePos, u);//避免出现只有一个参数，且参数是一个函数调用的表达式的而造成的添加多余参数的问题
}
Function* do_realUnkown_Function(const std::string &fcnname,std::vector<ExpNodeBase*> *real_exp,int totalunk,std::vector<Log> *log=nullptr)
{
	auto finfo = getFunctionInfo(fcnname);//用户自定义的函数不存在结合性的概念和优先级的概念 都是从左到右
	Function* fptr = nullptr;
	if (finfo)
	{
		//是否是用户自定义函数
		if (finfo->_isbuiltin == true)
		{
			fptr = new BuiltInFunction(fcnname);
		}
		else
		{
			fptr = new UserFunction(fcnname);
		}
		bool rs = fptr->setParameterPtr(*real_exp,log);
		if (rs)
		{
			return fptr;
		}
		else
		{
			if (log)
				log->emplace_back(LogType::ERROR, "无法解析函数的表达式",finfo->_FcnName);
			return nullptr;
		}
	}
	else
	{
		if (log)
			log->emplace_back(LogType::ERROR, "无法读取函数的信息");
		return nullptr;
	}
}
//解析函数和未知数的辅助函数 表达式，保存操作符的栈，保存数字、函数的栈，上界，下界，未知数列表
int do_trasfer_FcnandPresent(const std::string &exp, std::stack<Function*> &fcns, std::stack<ExpNodeBase*> &cab, int &l, int &u, std::map<std::string, Parameter> *unklist = nullptr, std::vector<Log> *log = nullptr)
{
	//确定函数的名称：同一个符号意义不同的有歧义函数可以没有括号但是参数数量或参数类型一定是不一样的 但也可以使用-(x)的调用形式 有歧义的符号 ： -
	//例如 -3-1 第一个-时取反一元函数；d第二个-是减法操作，二元
	Function *fptr = nullptr;
	if (exp[l] == '-' && cab.size() == 1)
	{
		//不是括号调用形式 后面可以是数字 或者是函数名
		//auto *fninfo = getFunctionInfo(std::string("-"));
		fptr = new BuiltInFunction("-");
		fcns.push(fptr);
		return l + 1;
	}
	else
	{

		if (exp[l] == '+' || exp[l] == '-' || exp[l] == '*' || exp[l] == '/')
		{
			fptr = new BuiltInFunction(exp[l]);
			FunctionInfo *curr = fptr->getFcnExpPtr(), *statop;
			if (curr == nullptr)
			{
				return -1;//error 
			}
			for (; !fcns.empty() && fcns.top() != nullptr; fcns.pop())
			{
				statop = fcns.top()->getFcnExpPtr();
				if (statop == nullptr)
					return -1;//unknown opts
				if (curr->_priority < statop->_priority)
					break;
				bool rs = fcns.top()->setParameterPtr(cab, log);
				if (rs)
					cab.push(fcns.top());
				else
					return -1;
			}
			fcns.push(fptr);
			return l + 1;
		}//其实可以添加一些更长的运算符 诸如逻辑等等 思路是作为函数处理就可以了
		else
		{
			//C函数是以可打印字符且不是数字、字母开头的
			if (isgraph(exp[l]) && !isalnum(exp[l]))
			{
				//是C语言开头的函数的类型，也不是+ - * /基本运算
				int endof = l + 1;
				for (; endof <= u && (isgraph(exp[endof]) && !isalnum(exp[endof])); ++endof)
					continue;
				fptr = new BuiltInFunction(exp.substr(l, endof - l));
				FunctionInfo *curr = fptr->getFcnExpPtr(), *statop;
				if (curr == nullptr)
				{
					return -1;//error 
				}
				for (; !fcns.empty() && fcns.top() != nullptr; fcns.pop())
				{
					statop = fcns.top()->getFcnExpPtr();
					if (statop == nullptr)
						return -1;//unknown opts
					if (curr->_priority < statop->_priority)
						break;
					bool rs = fcns.top()->setParameterPtr(cab, log);
					if (rs)
						cab.push(fcns.top());
					else
						return -1;
				}
				fcns.push(fptr);
				return endof;
			}
			else
			{
				int split = l + 1;//split是变量函数名的后一位 若exp[split]=='('表示函数 否则是变量名
				for (; isalnum(exp[split]) || exp[split] == '_'; ++split)
					;
				if (exp[split] == '(')
				{
					std::string fcnname = exp.substr(l, split - l);
					int right = do_transfer_getLastKuohao(exp, split, u);
					if (right == -1)
						return -1;//error 
					std::vector<std::pair<int, int>> poss;
					do_transfer_spiltFunctionSubExp(exp, split + 1, right - 1, poss);
					if (poss.size() < 1)
						return -1;//error
					std::vector<ExpNodeBase*> real_exp(poss.size());
					for (int i = 0; i < poss.size(); ++i)
					{
						real_exp[i] = Transfer_ExpToExpTree(exp, poss[i].first, poss[i].second, unklist, log);
						if (real_exp[i] == nullptr)
						{
							return -1;
						}
					}
					fptr = do_realUnkown_Function(fcnname, &real_exp, poss.size(), log);
					if (fptr)
					{
						cab.push(fptr);//放入值类型
						return right + 1;
					}
					else
					{
						return -1;
					}
				}
				else
				{
					//优先查找常量这个以后再加入
					//变量名字
					std::string name = exp.substr(l, split - l);
					auto cfind = getConst(name);
					if (cfind)
					{
						//常量
						Present *pre = new Present(PresentType::CONST, cfind->getValueExp());
						cab.push(pre);
						return split;
					}
					else
					{
						if (unklist)
						{
							auto id = unklist->find(name);
							if (id != unklist->end())
							{
								Present *pre = new Present(PresentType::UNK, nullptr, id->second._paraID);
								cab.push(pre);
								return split;
							}
						}
						else
						{
							if (log)
								log->emplace_back(LogType::UNKNOWN, "解析函数调用或者代数的时候发生了意外的状况，但计算过程已经终止。");
							return -1;
						}
					}
				}

			}
		}
	}

}

ExpNodeBase *Transfer_ExpToExpTree(std::string e, int lower, int upper, std::map<std::string, Parameter> *unklist, std::vector<Log> *log)
{
	std::stack<Function*> fcns;//这个是保存函数、运算符和括号的栈
	std::stack<ExpNodeBase*> cab;//计算单元
	std::string exp(e);
	if (e.size() < 1)
	{
		if (log)
			log->emplace_back(LogType::ERROR, "表达式太短");
		return nullptr;
	}
	//submark默认全是0 >=1 表示括号的层数 例如sin((9)) submark[3]=1 submark[4]=2 submark[6]=2 submark[7]=1;
	for (int beg = lower; beg <= upper;)
	{
		//对于-作为单元操作符，我们规定如果之前没有操作数即cab栈的大小为0，那么就是单元的。如果时出现在表达式中的 且是有歧义的操作符或者函数 是肯定会有括号的 而括号则会在递归中解决
		if ((exp[beg] >= '0' && exp[beg] <= '9'))
		{
			int endn = beg + 1;
			for (; exp[endn] == '.' || (exp[endn] >= '0' && exp[endn] <= '9'); ++endn)
				;
			ExpNodeBase *num = new Number(exp.substr(beg, endn - beg));
			if (!num)
			{
				//可能需要调用内存清理函数
				if (log)
					log->emplace_back(LogType::ERROR, "内存分配失败");
				return nullptr;
			}
			cab.push(num);
			beg = endn;
		}
		else
		{
			if (exp[beg] == '(')
			{
				if (beg < upper)
				{

					fcns.push(nullptr);//左括号
					++beg;
				}
			}
			else
			{
				if (exp[beg] == ')')
				{
					for (; !fcns.empty() && fcns.top() != nullptr; fcns.pop())
					{
						fcns.top()->setParameterPtr(cab);
						cab.push(fcns.top());
					}
					if (!fcns.empty())
					{
						fcns.pop();
					}
					else
					{
						//可能需要调用内存清理函数
						if (log)
							log->emplace_back(LogType::ERROR, "括号匹配失败");
						return nullptr;
					}
					++beg;
				}
				else
				{
					if ((isgraph(exp[beg]) && !isalnum(exp[beg])/*不能使数字字母开头*/)|| isalpha(exp[beg])/*一般函数和变量*/)
					{
						//函数的头一个字母 以及变量名字 可以包含开头必须是字母 其余是数字 字母 下划线
						//但是必须解析C语言 数学运算符
						int la = do_trasfer_FcnandPresent(exp, fcns, cab, beg, upper,unklist,log);
						if (la == -1)
						{
							//可能需要调用内存清理函数
							if (log)
								log->emplace_back(LogType::ERROR, "解析子表达式出现问题");
							return nullptr;
						}
						beg = la;
					}
					else
					{
						//可能需要调用内存清理函数
						if (log)
							log->emplace_back(LogType::ERROR, "表达式出现了未定义的字符");
						return nullptr;
					}
				}
			}
		}
	}
	for (; !fcns.empty(); fcns.pop())
	{
		//buildTreeNode(fcns.top(), cab);
		fcns.top()->setParameterPtr(cab);
		cab.push(fcns.top());
	}
	if (cab.size() == 1)
	{
		return cab.top();
	}
	else
	{
		//可能需要调用内存清理函数
		if (log)
			log->emplace_back(LogType::ERROR, "表达式存在错误");
		return nullptr;
	}
}


