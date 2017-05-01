#ifndef PSL_MY_SCOPE_EXIT_H
#define PSL_MY_SCOPE_EXIT_H

#include <functional>

class MyScopeExit
{
public:
    explicit MyScopeExit(std::function<void()> exitWork);
	MyScopeExit(const MyScopeExit &) = delete;
	MyScopeExit & operator = (const MyScopeExit &) = delete;
    ~MyScopeExit();
    void execWork(bool bexec);
private:
    std::function<void()> m_exitWork;
    bool m_isExec;
};


#endif
