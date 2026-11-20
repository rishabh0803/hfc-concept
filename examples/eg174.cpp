#include <iostream>
using namespace std;
class Collection
{
    int x[4];

public:
    Collection()
    {
        x[0] = 10;
        x[1] = 20;
        x[2] = 30;
        x[3] = 40;
    }
    class iterator
    {
    public:
        int *ptr;
        int index;
        iterator(int *ptr, int index) : ptr(ptr), index(index) {}
        int operator!=(const iterator &other)
        {
            return this->index != other.index;
        } // ideally other relational operators should be overloaded
        int operator*()
        {
            return this->ptr[this->index];
        }
        iterator operator++() // for prefix
        {
            ++(this->index);
            return *this;
        }
        iterator operator++(int) // for postfix
        {
            iterator k = *this;
            this->index++;
            return k;
        }
    };
    iterator begin()
    {
        iterator t(x, 0);
        return t;
    }
    iterator end()
    {
        iterator t(NULL, 4);
        return t;
    }
};

int main()
{
    Collection c;
    for (Collection::iterator iter = c.begin(); iter != c.end(); ++iter)
    {
        cout << *iter << endl;
    }
    cout << "------------------------------" << endl;
    for (auto iter = c.begin(); iter != c.end(); ++iter)
    {
        cout << *iter << endl;
    }
    cout << "------------------------------" << endl;
    for (auto x : c)
    {
        cout << x << endl;
    }
    return 0;
}