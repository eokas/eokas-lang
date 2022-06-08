
#ifndef  _EOKAS_BASE_EVENT_H_
#define  _EOKAS_BASE_EVENT_H_

#include "header.h"

_BeginNamespace(eokas)

enum class SignalResult
{
	Break,
	Continue
};

template<typename SignalMessage>
class AbstractSignalHandler :public Interface
{
public:
	virtual SignalResult doHandle(SignalMessage message) = 0;
};

template<typename SignalReceiver, typename SignalMessage>
class SignalHandler_Method :public AbstractSignalHandler<SignalMessage>
{
	typedef SignalResult (SignalReceiver::*HandleFunc)(SignalMessage message);

public:
	SignalHandler_Method(SignalReceiver* receiver, HandleFunc handle)
		:mReceiver(receiver)
		,mHandle(handle)
	{}

	SignalResult doHandle(SignalMessage message)
	{
		SignalResult result = SignalResult::Continue;
		if(mReceiver != nullptr)
		{
			result = (mReceiver->*mHandle)(message);
		}
		return result;
	}

public:
	SignalReceiver* mReceiver;
	HandleFunc mHandle;
};

template<typename HandleFunc, typename SignalMessage>
class SignalHandler_Functor :public AbstractSignalHandler<SignalMessage>
{
public:
	SignalHandler_Functor(HandleFunc handle)
		:mHandle(handle)
	{}

	SignalResult doHandle(SignalMessage message)
	{
		SignalResult result = SignalResult::Continue;
		if(mHandle != nullptr)
		{
			result = (*mHandle)(message);
		}
		return result;
	}

public:
	HandleFunc mHandle;
};

template<typename SignalMessage>
class Signal
{
	using HandlerList = std::list<AbstractSignalHandler<SignalMessage>*>;

public:
	Signal()
		:mHandlers()
	{}

	virtual ~Signal()
	{
		this->clearHandlers();
	}

	void operator()(SignalMessage message)
	{
		if(!mHandlers.empty())
		{
			auto handlerIter = mHandlers.begin();
			while(handlerIter != mHandlers.end())
			{
				if(*handlerIter == nullptr)
				{
					handlerIter = mHandlers.erase(handlerIter);
					continue;
				}
				SignalResult result = (*handlerIter)->doHandle(message);
				if(result == SignalResult::Break)
					break;
			
				// SignalResult::Continue
				++ handlerIter;
			}
		}
	}

	bool hasHandler()
	{
		return !mHandlers.empty();
	}

	template<typename SignalReceiver>
	void attachHandler(SignalReceiver* receiver, SignalResult (SignalReceiver::*handle)(SignalMessage))
	{
		AbstractSignalHandler<SignalMessage>* handler 
			= new SignalHandler_Method<SignalReceiver, SignalMessage>(receiver, handle);
		mHandlers.push_back(handler);
	}

	template<typename HandleFunc>
	void attachHandler(HandleFunc handle)
	{
		AbstractSignalHandler<SignalMessage>* handler
			= new SignalHandler_Functor<HandleFunc, SignalMessage>(handle);
		mHandlers.push_back(handler);
	}

	template<typename SignalReceiver>
	void detachHandler(SignalReceiver* receiver, SignalResult (SignalReceiver::*handle)(SignalMessage))
	{
		auto handlerIter = mHandlers.begin();
		while(handlerIter != mHandlers.end())
		{
			if(*handlerIter == nullptr)
			{
				handlerIter = mHandlers.erase(handlerIter);
				continue;
			}
			SignalHandler_Method<SignalReceiver, SignalMessage>* handler =
				dynamic_cast<SignalHandler_Method<SignalReceiver, SignalMessage>*>(*handlerIter);
			if( (handler != nullptr) &&
				(handler->mReceiver == receiver) &&
				(handler->mHandle == handle) )
			{
				delete (*handlerIter);
				handlerIter = mHandlers.erase(handlerIter);
				continue;
			}

			++ handlerIter;
		}
	}

	template<typename HandleFunc>
	void detachHandler(HandleFunc handle)
	{
		auto handlerIter = mHandlers.begin();
		while(handlerIter != mHandlers.end())
		{
			if(*handlerIter == nullptr)
			{
				handlerIter = mHandlers.erase(handlerIter);
				continue;
			}
			SignalHandler_Functor<HandleFunc, SignalMessage>* handler =
				dynamic_cast<SignalHandler_Functor<HandleFunc, SignalMessage>*>(*handlerIter);
			if( (handler != nullptr) &&
				(handler->mHandle == handle) )
			{
				delete (*handlerIter);
				handlerIter = mHandlers.erase(handlerIter);
				continue;
			}

			++ handlerIter;
		}
	}

	void clearHandlers()
	{
		auto handlerIter = mHandlers.begin();
		while(handlerIter != mHandlers.end())
		{
			if(*handlerIter != nullptr)
			{
				delete (*handlerIter);
				*handlerIter = nullptr;
			}
			++ handlerIter;
		}
		mHandlers.clear();
	}

private:
	HandlerList mHandlers;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_EVENT_H_
