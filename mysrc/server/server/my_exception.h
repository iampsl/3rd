#ifndef PSL_MY_EXCEPTION_H_
#define PSL_MY_EXCEPTION_H_

#include <stdexcept>
#include <string>
#include <sstream>
#include <type_traits>

#ifdef MY_EXCEPTION_A
#error "macro have already defined"
#endif

#ifdef MY_EXCEPTION_B
#error "macro have already defined"
#endif

#ifdef MY_EXCEPTION_OP
#error "macro have already defined"
#endif

class my_exception : public std::exception
{
public:
    my_exception() :MY_EXCEPTION_A(*this), MY_EXCEPTION_B(*this)
    {

    }

    my_exception(const char * what) :m_what(what), MY_EXCEPTION_A(*this), MY_EXCEPTION_B(*this)
    {

    }

    my_exception(const my_exception & other) : m_what(other.m_what), MY_EXCEPTION_A(*this), MY_EXCEPTION_B(*this)
    {

    }

    my_exception(my_exception && other) :MY_EXCEPTION_A(*this), MY_EXCEPTION_B(*this)
    {
        m_what.swap(other.m_what);
    }

    my_exception & operator = (const my_exception & other)
    {
        m_what = other.m_what;
        return *this;
    }

    my_exception & operator = (my_exception && other)
    {
        m_what.swap(other.m_what);
        return *this;
    }

    virtual ~my_exception() override
    {

    }

    virtual const char * what() const noexcept override
    {
        return m_what.c_str();
    }

    my_exception & capture(const char * file, int line, const char * expression)
    {
        std::ostringstream ostrstream;
        ostrstream << "failed expression:" << expression << std::endl;
        ostrstream << "position:" << file << "(" << line << ")" << std::endl;
        m_what += ostrstream.str();
        return *this;
    }

    template<typename T>
    my_exception & capture(const char * name, const T & value)
    {
        std::ostringstream ostrstream;
        ostrstream << name << ":" << value << std::endl;
        m_what += ostrstream.str();
        return *this;
    }

public:
    my_exception & MY_EXCEPTION_A;
    my_exception & MY_EXCEPTION_B;
private:
    std::string m_what;
};

#define MY_EXCEPTION_A(x) MY_EXCEPTION_OP(x,B)
#define MY_EXCEPTION_B(x) MY_EXCEPTION_OP(x,A)
#define MY_EXCEPTION_OP(x,next) MY_EXCEPTION_A.capture((#x),(x)).MY_EXCEPTION_##next
#define MY_ENSURE(expr) static_assert(std::is_same<decltype((expr)), bool>::value, #expr" is not bool expression"); if((expr));else throw my_exception().capture(__FILE__, __LINE__, (#expr)).MY_EXCEPTION_A



#endif

