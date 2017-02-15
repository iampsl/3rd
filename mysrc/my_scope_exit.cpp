#include "my_scope_exit.h"

my_scope_exit::my_scope_exit(std::function<void()> on_exit_exec) :on_exit_exec_(on_exit_exec), bexec_(true)
{

}

my_scope_exit::~my_scope_exit()
{
    if (bexec_)
    {
        on_exit_exec_();
    }
}

void my_scope_exit::set_exec(bool bexec)
{
    bexec_ = bexec;
}

