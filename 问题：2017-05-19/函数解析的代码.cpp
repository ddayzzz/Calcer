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