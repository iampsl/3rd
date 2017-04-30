#ifndef PSL_MY_SCOPE_EXIT_H_
#define PSL_MY_SCOPE_EXIT_H_

#include <functional>

class my_scope_exit
{
public:
    explicit my_scope_exit(std::function<void()> on_exit_exec);
    ~my_scope_exit();
    void set_exec(bool bexec);
private:
    my_scope_exit(const my_scope_exit & other) = delete;
    my_scope_exit & operator = (const my_scope_exit & other) = delete;
private:
    std::function<void()> on_exit_exec_;
    bool bexec_;
};


#endif
