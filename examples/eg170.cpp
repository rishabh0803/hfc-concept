#include <iostream>
#include <thread>

void do_something()
{
    // some code
}

int main()
{
    thread t(do_something);
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
    t.join();
    return 0;
}