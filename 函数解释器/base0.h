//��������������Ķ���
#include <queue>
#include <stack>
#include <string>
#include <utility>
#include <string>
#include <map>
#include <memory>
# include <iterator>
#include "json\json.h"
const long double PI = 3.1415926535898l;//PI����
const long double DEGTORAD = PI / 180;//�Ƕ���ת������
//��������Ե�ö�����ͣ�ע������Զ��庯����֧���Զ������ԣ���Ϊ���ڴ����Զ��庯��ʽ������Ϊһ������ڵ������
enum class FcnCombine { Left, Right };
//�������͵�presentfor������
enum class PresentType{UNK,CONST};
//�ڵ������ ����Ϊ�˱���dynamic_cast�Ŀ�����
/*
NUM���������ͣ�FCN���������ͣ���Ҷ�ӣ���PRE���������ͣ�������Ϊδ֪�������������ͺͳ�������MARTX����������
SET���������ͣ�
*/
//�����û�������Ĭ��ֵ
const int USERDEFINED_FUNC_PRIORITY = 0;//�û�����ĺ�����Ĭ�����ȼ�
const FcnCombine USERDEFINED_FUNC_COMBINATION = FcnCombine::Left;//Ĭ���û�����ĺ��������ȼ�
enum class ExpNodeBaseType {NUM,FCN,PRE,MATRIX,SET};
//��־����Ĵ�����Ϣ
enum class LogType {NONE,ERROR,WARNNING,UNKNOWN};
//�����Ĭ�ϵ���ֵ������
typedef long double FloatType;//Ĭ�ϵĸ�������
typedef int Int32;//Ĭ�ϵ���������
typedef int BoolType;//Ĭ�ϵĲ������� ��������0Ϊ�� ��0Ϊ��
struct ExpNodeBase;
struct ValueTypeBase;

//�����Ĳ�������������Ϣ
struct ParameterDetail {
	std::string _description;//���ǵĽ�����Ϣ
	ExpNodeBase *_condition;//��ȡ������ֵ��ʱ���ж��Ƿ��������������Ƿ���϶�����һ����������Ҫ����0����1����Ȼ������Ҫ�ҵĽ�����zici��
	std::string _errorMsg;//������_condition��ֵΪfalseʱ�ĳ�����Ϣ
	ParameterDetail() = default;
	ParameterDetail(const std::string &des, const std::string &cond, const std::string error) :\
		_description(des), _errorMsg(error), _condition(nullptr) {}
};
//�����Ļ�����Ҫ����������
struct Parameter {
	int _paraID;//����������ֵ ��0��ʼ
	std::string _name;//�����ı�ʶ��
	ParameterDetail _detail;//��ϸ�Ĳ�����Ϣ������϶����Ǽ̳�
	Parameter(int id, const std::string &name):_paraID(id), _name(name),_detail(ParameterDetail()){}
	Parameter(int id, const std::string &name, const std::string &des, const std::string &cond, const std::string error) :\
		_paraID(id), _name(name), _detail(ParameterDetail(des,cond,error)) {}
};

//��־��Ϣ
struct Log {
	LogType _logtype;//��־��Ϣ������
	std::string _message;//��־����Ϣ
	std::string _exp;//��������ı��ʽ��Ϣ
	Log(LogType logtype, const std::string &info, const std::string &exp = std::string()) :_logtype(logtype), _message(info), _exp(exp) {}
	Log(LogType logtype, const char *msg, const char *exp = nullptr) :_logtype(logtype), _message(msg) { if (exp) _exp = exp; }
	//����൱��Ĭ�ϵĹ��캯�� Ĭ��Ϊû�д���
	Log(LogType logtype = LogType::NONE) :_logtype(logtype) {}
};
//������������Ϣ
struct ExpNodeBase {
	/*
	ExpNodeBase *pChild;//��һ������ָ��
	ExpNodeBase *pBrother;//ͬһ����ĵ�һ������
	~ExpNodeBase() {}
	ExpNodeBase():pChild(nullptr),pBrother(nullptr){}
	*/
	//���麯�������ر��ʽ���������ֵ��ָ��
	virtual ValueTypeBase* eval(std::vector<Log>* log = nullptr) = 0;
	//��̬ʶ�����ͣ�����RTTI�Ŀ���
	virtual ExpNodeBaseType getType()
	{
		return ExpNodeBaseType::FCN;
	}
	ExpNodeBase* pChild;//��һ������ָ��
	ExpNodeBase* pBrother;//ͬһ����ĵ�һ������
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
//���ʽ����Ҷ������
struct ValueTypeBase :public ExpNodeBase
{
	//���ʽҶ�ӽ�㣬��ô���Լ̳���Щ�ӿڣ��������Ҫ���̳�Ĭ�ϵİ汾�Ϳ�����
	virtual std::string getDisplayName() { return std::string("û�ж��������"); }

	virtual std::pair<FloatType, Log> asFloatType() const { return { FloatType(),Log(LogType::ERROR,"��֧�ֽ�������ת��ΪLong Double����") }; };//ת��Ϊ��ֵ����
	virtual std::pair<FloatType, Log> asInt32Type() const { return { FloatType(),Log(LogType::ERROR,"��֧�ֽ�������ת��Ϊ��������") }; };//ת��Ϊ��������
	virtual std::pair<FloatType, Log> asBoolType() const { return { FloatType(),Log(LogType::ERROR,"��֧�ֽ�������ת��Ϊ��������") }; };//ת��Ϊ��������

	virtual std::pair<ValueTypeBase*, Log> opt_add(ValueTypeBase* r) { return { nullptr,Log(LogType::ERROR,"���������Ͳ�֧�ֲ�����\'+\'") }; }//�ӷ�
	virtual std::pair<ValueTypeBase*, Log> opt_divide(ValueTypeBase* r) { return { nullptr,Log(LogType::ERROR,"���������Ͳ�֧�ֲ�����\'-\'") }; }//����
	virtual std::pair<ValueTypeBase*, Log> opt_subtract(ValueTypeBase* r) { return { nullptr,Log(LogType::ERROR,"���������Ͳ�֧�ֲ�����\'*\'") }; }//�˷�
	virtual std::pair<ValueTypeBase*, Log> opt_multiply(ValueTypeBase* r) { return { nullptr,Log(LogType::ERROR,"���������Ͳ�֧�ֲ�����\'/\'") }; }//����
	virtual std::pair<ValueTypeBase*, Log> opt_reverse() { return { nullptr,Log(LogType::ERROR,"���������Ͳ�֧��\'��Ԫ����\'") }; }//����Ԫ
	virtual ~ValueTypeBase() {}
	virtual ValueTypeBase *clone() = 0;//���ʱ��¡һ��ָ�� ����������һ�ν���ڴ��ͷŵ�����
	//�����ֵ������
	virtual std::ostream &display_value(std::ostream &os) { return os; }
};
//�������ʽ�ࣺ�������������ı��ʽ��Ҫ�󣬱������������Ҫ�󣬲���������
class FunctionInfo {
private:
	
public:
	std::vector<std::string> _signatures;//������ǩ�� �������ܻ����ظ���ǩ��
	std::string _expression;//�����Ľ���ʽ
	std::string _FcnName;//��������ʾ����
	std::string _displayname;//��ʾ������
	std::map<std::string, Parameter> _map_nameToInfo;//�������ֺ͵�������Ϣ��ӳ��
	bool _isbuiltin;//�Ƿ������õı��ʽ
	int _priority;//���������ȼ�
	FcnCombine _combine;//�����Ľ����
	int _unkcount;//δ֪���ĵ�����
	FunctionInfo(const std::string &fcnname,const std::string &disname,std::string &exp,
		std::initializer_list<Parameter> parainit,bool builtin,int prior,FcnCombine comb,int unkcount):
		_priority(prior), _unkcount(unkcount),_expression(exp),_FcnName(fcnname),_displayname(disname),
		_isbuiltin(builtin),_combine(comb)
	{
		for (auto i =parainit.begin(); i != parainit.end(); ++i)
		{
			_map_nameToInfo.emplace(i->_name, *i);//���ݲ��������ֽ���ӳ���
		}
	}
	//֧�ִ��Ѿ������Ĳ����б�(vector�洢����δ֪�������֣��������Ӧ����Ψһ��)�в��뺯������������ʹ����ͨ��Parameter(��������)�����
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
//��ȡ��������Ϣ����FunctionInfo��ָ�룬���id��������ȫ�ֺ������У���ô����nullptr
FunctionInfo *getFunctionInfo(const std::string &id);
//���ݱ��ʽ��e[w(lower),w[upper]��ת��Ϊ���ʽ��������һ��ָ�����־��Ϣ��������ʽ��һ�����в����ı��ʽ����ô��ָ��ʵ���б�
ExpNodeBase *Transfer_ExpToExpTree(std::string e, int lower, int upper, std::map<std::string, Parameter>* = nullptr, std::vector<Log>* = nullptr);
struct Const {
	std::string _description;//������������Ϣ
	std::string _exp;//�����ı��ʽ
	ValueTypeBase *_value = nullptr;//����õ��ı��ʽ��ֵ

	//����ǿ��Դӱ��ʽ������ĳ�������ʽ �����Ļ�����Ҫ��������
	Const(const std::string &exp, const std::string &des = std::string()) :_description(des), _exp(exp) {
		if (_exp.size() > 0)
		{
			auto res = Transfer_ExpToExpTree(_exp, 0, _exp.size() - 1);//�����ı��ʽ���ܰ�������
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
	//һ��Ҫ���
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
//ȫ�ֺ�����
std::map<std::string, FunctionInfo> functionlist;
std::map<std::string, Const> global_constslist;//�������ȫ�ֵĳ����б�
//����ǳ�ʼ����ȫ�ֺ����б�
void init_functionlist()
{
	Parameter p1(0,"x","�����", "none", "error");
	Parameter p2(1,"y","�Ҽ���", "none", "error");
	//FunctionExpression fe(std::string("�ӷ�����"),"22", true,"none", std::initializer_list<Parameter>{ p1,p2 });
	FunctionInfo finfo(std::string("+"), std::string("�ӷ����㡢���ϲ��������"), std::string("x+y"), { p1,p2 },true, 4, FcnCombine::Left, 2);
	finfo.insertSignature("Number(Number,Number)");
	finfo.insertSignature("Set(Set,Set)");
	functionlist.emplace("+", finfo);

	Parameter p3(0,"x","������", "none", "error");
	Parameter p4(1,"y","����", "none", "error");
	FunctionInfo finfo1(std::string("-"), std::string("�������㡢���ϲ�����"), std::string("x-y"), { p3,p4 }, true, 4, FcnCombine::Left, 2);
	finfo1.insertSignature("Number(Number,Number)");
	finfo1.insertSignature("Set(Set,Set)");
	functionlist.emplace("-", finfo1);

	Parameter p5(0,"x", "����1", "none", "error");
	Parameter p6(1,"y", "����2", "none", "error");
	//Parameter p7(2,"z", "����3", "none", "error");
	FunctionInfo finfo2(std::string("kk"), std::string("kk�����"), std::string("x+y") ,{ p5,p6 },false, USERDEFINED_FUNC_PRIORITY, USERDEFINED_FUNC_COMBINATION, 2);
	finfo2.insertSignature("Number(Number,Number)");
	//functionlist.emplace("kk", finfo2);

	Parameter p7(0, "x", "�Ƕȣ������ƣ�", "none", "error");
	FunctionInfo finfo3(std::string("sin"), std::string("���Һ���"), std::string("sin(x)"), { p7 }, true, 4, FcnCombine::Left, 1);
	finfo3.insertSignature("Number(Number,Number)");
	functionlist.emplace("sin", finfo3);

	Parameter p8(0, "x", "���ʽ1", "none","error");
	Parameter p9(1, "y", "���ʽ2", "none", "error");
	//FunctionExpression fe(std::string("�ӷ�����"),"22", true,"none", std::initializer_list<Parameter>{ p1,p2 });
	FunctionInfo finfo4(std::string("!="), std::string("�����������"), std::string("x!=y"), { p1,p2 }, true, 7, FcnCombine::Left, 2);
	finfo4.insertSignature("Bool(Number,Number)");
	functionlist.emplace("!=", finfo4);
}
//��ʼ�����ļ��������ȫ�ֳ����б�
void init_constslist()
{
	//Const c1("3.1415926", "PI");//ע�⣺�����ôд�ͻᵼ���ڴ���ͷ�
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
//���ʽ���Ļ������ͣ�ӵ��һ�����Ӷ��ӽڵ�͵�һ��ͬ���Ľڵ�����ָ���򣺼�����˫�ױ�ʾ��



//��ValueTypeBase������Ҷ������
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
	std::string getDisplayName() override { return std::string("��������"); }
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
				return { nullptr,Log(LogType::ERROR,"�Ҳ�������֧��ת��ΪLong Double����") };
			}
		}
		return { nullptr,Log(LogType::ERROR,"�Ҳ���������Ϊ��") };
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
			return { nullptr,Log(LogType::ERROR,"�Ҳ�������֧��ת��ΪLong Double����") };
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
			return { nullptr,Log(LogType::ERROR,"�Ҳ�������֧��ת��ΪLong Double����") };
		}
	}
	std::pair<ValueTypeBase*, Log> opt_divide(ValueTypeBase* r) override
	{
		auto p = r->eval()->asFloatType();
		if (p.second._logtype == LogType::NONE)
		{
			if (p.first == 0.0l)
			{
				return {nullptr,Log(LogType::ERROR,"��������Ϊ0") };
			}
			return { new Number(_val / p.first),Log() };
		}
		else
		{
			return { nullptr,Log(LogType::ERROR,"�Ҳ�������֧��ת��ΪLong Double����") };
		}
	}
	std::pair<ValueTypeBase*, Log> opt_reverse() override
	{
			return { new Number(0.0l-_val),Log() };
	}
	std::ostream &display_value(std::ostream &os) override { return os<<_val; }
	//�������� ����û��ʲô�����ͷŵ�
	virtual ~Number(){}
	virtual Number *clone()
	{
		return new Number(*this);
	}
};

//��ValueTypeBase������Ҷ�����ͣ��洢�����ļ�������
class Set :public ValueTypeBase
{
private:
public:
	Set(std::initializer_list<ValueTypeBase*> init) 
	{ 
		//��һ��������ÿһ��Ԫ�ص�ֵ ת��ΪINT32����
		//�ڶ�����ʹ��λ��������ʽ�洢
	}
	ValueTypeBase* eval(std::vector<Log>* log) override { return this; }
	ExpNodeBaseType getType() override
	{
		return ExpNodeBaseType::SET;
	}
	std::string getDisplayName() override { return std::string("��������"); }
	std::pair<ValueTypeBase*, Log> opt_add(ValueTypeBase* r) override
	{
		return { nullptr,Log(LogType::ERROR,"�Ҳ�������֧��ת��ΪLong Double����") };
	}
	std::pair<ValueTypeBase*, Log> opt_subtract(ValueTypeBase* r) override
	{

		return { nullptr,Log(LogType::ERROR,"�Ҳ�������֧��ת��ΪLong Double����") };

	}
	std::pair<ValueTypeBase*, Log> opt_multiply(ValueTypeBase* r) override
	{
		return { nullptr,Log(LogType::ERROR,"�Ҳ�������֧��ת��ΪLong Double����") };

	}
	//�������� ����û��ʲô�����ͷŵ�
	virtual ~Set() {}
	virtual Set *clone()
	{
		return new Set(*this);
	}
	virtual std::ostream &display_value(std::ostream &os) { return os; }
};
//����ǡ��������ͣ����������Ϳ��Դ������֡����ʽ���ĸ��ڵ���������Դ��������ϵ�����
class Present :public ExpNodeBase {

public:
	int _id;//���ʽ��type=UNK��ʱ��ʹ�ã���ʾ����δ֪���ں����ı�ţ���0��ʼ��
	PresentType _type;
	ExpNodeBase* _presentfor;//ָ����ָ��
	Present(PresentType type, ExpNodeBase *presentfor = nullptr, int id = 0) :_type(type), _id(id), _presentfor(presentfor) {}
	ExpNodeBaseType getType() override { return ExpNodeBaseType::PRE; }
	ValueTypeBase* eval(std::vector<Log>* log) override {
		if (_presentfor)
		{
			return _presentfor->eval();//���ر������ָ��
		}
		//error
		return nullptr;
	}
	virtual ~Present() override
	{
		if (_type == PresentType::CONST)
			return;//���������ͷ�
		if (_presentfor)
		{
			delete _presentfor;
			_presentfor = nullptr;
		}
	}
};
//���ú�������
//�������õĺ��� ��Ҫָ����������Ϣ��δ֪����ʵ���������������Գ��ӽڵ㿪ʼ��һ�η��������ֵܣ�Ĭ�ϴ������Ҽ��㣩
ValueTypeBase* BuiltInFuntionCalc(const FunctionInfo *fptr, int realunk, ExpNodeBase *paras, std::vector<Log> *log = nullptr)
{
	//����Ҫ�Ż�һ��
	if (!fptr)
	{
		if (log)
			log->emplace_back(LogType::ERROR, "û��ָ������ָ������");
		return nullptr;
	}
	if (realunk == 2)
	{
		//����Ƿ����㹻������
		if (!(paras && paras->pBrother))
		{
			if (log)
				log->emplace_back(LogType::ERROR, "�����Ĳ�����ʵ�ʵĲ��������", fptr->_FcnName);
			return nullptr;
		}
		//��Ԫ����
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
				log->emplace_back(LogType::ERROR, std::string("���������\'").append(fptr->_FcnName).append("\'��ʵ�ʲ��������������������Ͳ�����"));
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
					log->emplace_back(LogType::ERROR, "�����Ĳ�����ʵ�ʵĲ��������", fptr->_FcnName);
				return nullptr;
			}
			//һԪ����ע��ȡ�������� ʹ��number����Ԫ����
			std::pair<ValueTypeBase*, Log> calles;
			if (fptr->_FcnName == "-")
				calles = paras->eval()->opt_reverse();
			else if (fptr->_FcnName == "sin")
			{
				auto fir = paras->eval(log);
				if (fir == nullptr)
				{
					if (log)
						log->emplace_back(LogType::ERROR, "�����Ĳ�����ʵ�ʵĲ��������", fptr->_FcnName);
					return nullptr;
				}
				auto fcncalls = fir->asFloatType();
				if (fcncalls.second._logtype == LogType::NONE)
					return new Number(sinl(fcncalls.first));
				//error
				if (log) log->push_back(fcncalls.second);
				return nullptr;
				//����Ҫ�����´ε��ж�
			}
			else
			{
				//unknown opts
				if (log)
					log->emplace_back(LogType::ERROR, std::string("���������\'").append(fptr->_FcnName).append("\'��ʵ�ʲ��������������������Ͳ�����"));
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
//����������������
class Function:public ExpNodeBase {
private:
	//���ú�������Ϣָ��
	void do_setFcnPtr()
	{
		if (fcnID.size()>0)
		{
			auto i = getFunctionInfo(fcnID);
			fcnPtr = i;
		}
	}
protected:
	FunctionInfo* fcnPtr;//��������Ϣָ��
	int realUnknown;//�������������е�ʵ�ʵĲ��� ������ʵ�ʵĲ������������;����ں����ļ�����Ϊ
	std::string fcnID;//������Ψһ�ı�ʶ���Ƶ�ID
	ValueTypeBase* result = nullptr;//�����������Ľ��������һ�����ϵ����ݣ�ע��Ҫ�ͷš�֮������ô������Ϊ��Ҫ���ֶ�̬������
public:
	ExpNodeBaseType getType() override { return ExpNodeBaseType::FCN; }
	FunctionInfo *getFcnExpPtr() { return fcnPtr; }
	Function(const std::string &fn):fcnID(fn){ do_setFcnPtr(); }
	Function(const char c) { fcnID.push_back(c); do_setFcnPtr();}
	virtual bool setParameterPtr(std::vector<ExpNodeBase*> &real,std::vector<Log>* =nullptr) = 0;//��������Ƿ����Զ��庯������Ҫ���
	virtual bool setParameterPtr(std::stack<ExpNodeBase*> &sta, std::vector<Log>* = nullptr) = 0;//��������Ҫ��� ʹ��ջģʽ��
	virtual ~Function() {}//��ΪUserFuntion��resultָ������ú������ǹ��õ� ���Ծ���Ҫ�·��ͷŲ�����result��ָ��ֻ��builtinfunction�����ͷš���Ϊuserfunction��builtfunctionͨ��fcnexpr����
	
};
//�û�����ĺ���
/*
һ����ʽΪfcn(x,y,x+z)�ⶼ�ǿ��Եģ��㷨��ִ�еĹ����У����Ὣ�����Ľ���ʽ���м���Ȼ�󱣴���fcnExpr�У�������ʵ��
*/
class UserFunction :public Function
{
private:
	ExpNodeBase *fcnExpr;//ʵ�ʵĺ����Ľ���ʽ ���������ú�����ʱ����Ҫ�����������򲻹�
	//�������ĸ������������뺯���Ľ���ʽ�ı��ʽ�� �޸�present��δ֪�����͵�present
	bool do_trasfer_presentRealParameter(std::vector<ExpNodeBase*> &real, ExpNodeBase *exptree,std::vector<Log> *log=nullptr)
	{
		ExpNodeBase* pchild = exptree->pChild;
		bool res=true;
		while (pchild)
		{
			if (pchild->getType() == ExpNodeBaseType::FCN)
			{
				//��Ȼʽ����ʽ
				res=res & do_trasfer_presentRealParameter(real, pchild,log);//��ô������һ��ĵݹ�
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
									if (log) log->emplace_back(LogType::ERROR, "ָ���Ĳ�������");
									return false;
								}
							}
							else
							{
								if (pre->_type == PresentType::CONST)
									;//�����Ѿ��õ���ֵ ����ҾͲ�����
							}
						}
					}
					else
					{
						if (log) log->emplace_back(LogType::ERROR, "δ���������");
						return false;
					}
				}
			}
			pchild = pchild->pBrother;
		}
		return res;
	}
	bool setParameterPtr(std::stack<ExpNodeBase*> &sta, std::vector<Log>*) { return false; }//�������� ����Ҫ
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
			//û��ָ������
			if (log)
				log->emplace_back(LogType::ERROR, "����û��ָ���ı��ʽ");
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
					delete result;//����Ѿ����������Ϣ
				realUnknown = real.size();
				return do_trasfer_presentRealParameter(real, fcnExpr,log);
			}
			else
			{
				if (log)
					log->emplace_back(LogType::ERROR, "�޷��������ʽ��", fcnPtr->_expression);
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
//���õĺ����Ķ��壻���û��������ǲ���Ҫ����ʽ��ֱ��ʹ��ָ�뱣�������Ϣ
class BuiltInFunction :public Function {
private:
	//ValueTypeBase* result = nullptr;//�����������Ľ��������һ�����ϵ����ݣ�ע��Ҫ�ͷš�֮������ô������Ϊ��Ҫ���ֶ�̬������
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
	//�����Ҫ�ù�userfunction�Ľ���ʽ�Ĵ����βεĹ���
	bool setParameterPtr(std::vector<ExpNodeBase*> &real, std::vector<Log>* log) {
		int j;
		ExpNodeBase **pnode = &pChild;
		for (j = 0; j < real.size(); ++j)
		{
			*pnode = real[j];
			pnode = &((*pnode)->pBrother);
		}
		realUnknown = j;
		//��̬��������������
		return true;
	}
	//�����Ҫ���ڽ������ʽ��ʱ���õ���ջ
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
			realUnknown = j;//����ʵ�ʵĲ���
			return true;
		}
		else
		{
			if (log)
				log->emplace_back(LogType::ERROR, "�޷�����������Ϣ��û����Ч�ĺ�����Ϣָ��");
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
//��ȡ�ӱ��ʽ�Ľ�βλ�ü������ŵ�λ��   beg��ʾ����������λ�� -1��ʾʧ��
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
//����ӱ��ʽ����ں������������ʧ�ܣ����poss ���� fcn(x+3,u+2) �������� x+3   u+2������յ����� l��������+1 u�������ŵ�λ��-1
void do_transfer_spiltFunctionSubExp(const std::string &exp, int l, int u, std::vector<std::pair<int, int>> &poss)
{
	//ע���ӱ��ʽҲ���ܰ�����������
	int prePos = l;//ǰһ���ӱ��ʽ����ֹλ�õ���һ������һ�� ����һ���ӱ��ʽ����ʼλ��
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
				//�����ӱ��ʽ
				int lastkuohao = do_transfer_getLastKuohao(exp, temp, u);
				if (lastkuohao == -1)
				{
					poss.clear();
					return;
				}
				if (lastkuohao == u || (lastkuohao < u && exp[lastkuohao + 1] == ','))
				{
					//�ҵ���һ���ӱ��ʽ
					poss.emplace_back(prePos, lastkuohao);
					l = prePos = lastkuohao + 2;
				}
				else
				{
					//����ʽ�������ı��ʽ ������Ҫ�������㲻Ҫ����prepos
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
		poss.emplace_back(prePos, u);//�������ֻ��һ���������Ҳ�����һ���������õı��ʽ�Ķ���ɵ���Ӷ������������
}
Function* do_realUnkown_Function(const std::string &fcnname,std::vector<ExpNodeBase*> *real_exp,int totalunk,std::vector<Log> *log=nullptr)
{
	auto finfo = getFunctionInfo(fcnname);//�û��Զ���ĺ��������ڽ���Եĸ�������ȼ��ĸ��� ���Ǵ�����
	Function* fptr = nullptr;
	if (finfo)
	{
		//�Ƿ����û��Զ��庯��
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
				log->emplace_back(LogType::ERROR, "�޷����������ı��ʽ",finfo->_FcnName);
			return nullptr;
		}
	}
	else
	{
		if (log)
			log->emplace_back(LogType::ERROR, "�޷���ȡ��������Ϣ");
		return nullptr;
	}
}
//����������δ֪���ĸ������� ���ʽ�������������ջ���������֡�������ջ���Ͻ磬�½磬δ֪���б�
int do_trasfer_FcnandPresent(const std::string &exp, std::stack<Function*> &fcns, std::stack<ExpNodeBase*> &cab, int &l, int &u, std::map<std::string, Parameter> *unklist = nullptr, std::vector<Log> *log = nullptr)
{
	//ȷ�����������ƣ�ͬһ���������岻ͬ�������庯������û�����ŵ��ǲ����������������һ���ǲ�һ���� ��Ҳ����ʹ��-(x)�ĵ�����ʽ ������ķ��� �� -
	//���� -3-1 ��һ��-ʱȡ��һԪ������d�ڶ���-�Ǽ�����������Ԫ
	Function *fptr = nullptr;
	if (exp[l] == '-' && cab.size() == 1)
	{
		//�������ŵ�����ʽ ������������� �����Ǻ�����
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
		}//��ʵ�������һЩ����������� �����߼��ȵ� ˼·����Ϊ��������Ϳ�����
		else
		{
			//C�������Կɴ�ӡ�ַ��Ҳ������֡���ĸ��ͷ��
			if (isgraph(exp[l]) && !isalnum(exp[l]))
			{
				//��C���Կ�ͷ�ĺ��������ͣ�Ҳ����+ - * /��������
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
				int split = l + 1;//split�Ǳ����������ĺ�һλ ��exp[split]=='('��ʾ���� �����Ǳ�����
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
						cab.push(fptr);//����ֵ����
						return right + 1;
					}
					else
					{
						return -1;
					}
				}
				else
				{
					//���Ȳ��ҳ�������Ժ��ټ���
					//��������
					std::string name = exp.substr(l, split - l);
					auto cfind = getConst(name);
					if (cfind)
					{
						//����
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
								log->emplace_back(LogType::UNKNOWN, "�����������û��ߴ�����ʱ�����������״��������������Ѿ���ֹ��");
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
	std::stack<Function*> fcns;//����Ǳ��溯��������������ŵ�ջ
	std::stack<ExpNodeBase*> cab;//���㵥Ԫ
	std::string exp(e);
	if (e.size() < 1)
	{
		if (log)
			log->emplace_back(LogType::ERROR, "���ʽ̫��");
		return nullptr;
	}
	//submarkĬ��ȫ��0 >=1 ��ʾ���ŵĲ��� ����sin((9)) submark[3]=1 submark[4]=2 submark[6]=2 submark[7]=1;
	for (int beg = lower; beg <= upper;)
	{
		//����-��Ϊ��Ԫ�����������ǹ涨���֮ǰû�в�������cabջ�Ĵ�СΪ0����ô���ǵ�Ԫ�ġ����ʱ�����ڱ��ʽ�е� ����������Ĳ��������ߺ��� �ǿ϶��������ŵ� ����������ڵݹ��н��
		if ((exp[beg] >= '0' && exp[beg] <= '9'))
		{
			int endn = beg + 1;
			for (; exp[endn] == '.' || (exp[endn] >= '0' && exp[endn] <= '9'); ++endn)
				;
			ExpNodeBase *num = new Number(exp.substr(beg, endn - beg));
			if (!num)
			{
				//������Ҫ�����ڴ�������
				if (log)
					log->emplace_back(LogType::ERROR, "�ڴ����ʧ��");
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

					fcns.push(nullptr);//������
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
						//������Ҫ�����ڴ�������
						if (log)
							log->emplace_back(LogType::ERROR, "����ƥ��ʧ��");
						return nullptr;
					}
					++beg;
				}
				else
				{
					if ((isgraph(exp[beg]) && !isalnum(exp[beg])/*����ʹ������ĸ��ͷ*/)|| isalpha(exp[beg])/*һ�㺯���ͱ���*/)
					{
						//������ͷһ����ĸ �Լ��������� ���԰�����ͷ��������ĸ ���������� ��ĸ �»���
						//���Ǳ������C���� ��ѧ�����
						int la = do_trasfer_FcnandPresent(exp, fcns, cab, beg, upper,unklist,log);
						if (la == -1)
						{
							//������Ҫ�����ڴ�������
							if (log)
								log->emplace_back(LogType::ERROR, "�����ӱ��ʽ��������");
							return nullptr;
						}
						beg = la;
					}
					else
					{
						//������Ҫ�����ڴ�������
						if (log)
							log->emplace_back(LogType::ERROR, "���ʽ������δ������ַ�");
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
		//������Ҫ�����ڴ�������
		if (log)
			log->emplace_back(LogType::ERROR, "���ʽ���ڴ���");
		return nullptr;
	}
}


