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