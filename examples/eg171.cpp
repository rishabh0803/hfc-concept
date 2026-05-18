#include <iostream>
#include <thread>

void do_something()
{
    // some code
}

int main()
{
    try
    {
        thread t(do_something);
        some_class obj(t); // parameterized constructor with (thread &t):t(t){}
                           // some class property -- thread &t;
                           // destructor with t.join();
                           // some_class should/will be called thread_guard
        // some code
        //
        // some code, may be return statement
        //
        //
        // some code, may be a return statement
        //
        // 	// some code, may be a function call that may raise exception
        //
        //
        // some code
    }
    catch (...)
    {
    }
    return 0;
}