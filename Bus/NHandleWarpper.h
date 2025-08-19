#pragma once
#include <QVariant>

/*
// 使用用例
class P
{
public:
	int print(const int & num)
	{
		qDebug() << num;
		return 90;
	}
};

P  pobj;
NHandleWrapper<P, int, int> wrapper(&pobj, &P::print);
NCoreBusControl::getInstance().RegRecvBin(1, wrapper);

QVariant outContext;
NCoreBusControl::getInstance().ExecBusBin(1, 2, outContext);
if (outContext.canConvert<int>())
{
	int num = outContext.value<int>();
	qDebug() << num;
}
*/

template<typename ClassType, typename InputType, typename ReturnType>
class NHandleWrapper {
public:
	using MemberFunction = ReturnType(ClassType::*)(const InputType&);

	NHandleWrapper(ClassType* obj, MemberFunction func)
		: m_obj(obj), m_func(func) {}

	void operator()(QVariant inputContext, QVariant& outContext) {
		if (inputContext.canConvert<InputType>()) {
			InputType input = inputContext.value<InputType>();
			ReturnType result = (m_obj->*m_func)(input);
			outContext = QVariant::fromValue(result);
		}
	}

private:
	ClassType* m_obj;
	MemberFunction m_func;
};

