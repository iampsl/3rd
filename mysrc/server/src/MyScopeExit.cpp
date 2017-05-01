#include "MyScopeExit.h"

MyScopeExit::MyScopeExit(std::function<void()> exitWork) :m_exitWork(exitWork), m_isExec(true)
{

}

MyScopeExit::~MyScopeExit()
{
    if (m_isExec)
    {
		m_exitWork();
    }
}

void MyScopeExit::execWork(bool bexec)
{
    m_isExec = bexec;
}

