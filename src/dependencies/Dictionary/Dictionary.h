
/*Arduino Dictionary
  This is a simple key-value dictionary for Arduino.
  It uses LinkedList (It has its own version within it to ensure compatibility).
  This library has had very little testing, so use at your own risk.

  For instructions go to: https://github.com/KeeganMartin/Dictionary/tree/master

*/

#ifndef Dictionary_h
#define Dictionary_h

#include "dependencies/LinkedList/LinkedList.h"

template <class T, class U>
class Dictionary
{
private:
    LinkedList<T> KeyList = LinkedList<T>();
    LinkedList<U> ValList = LinkedList<U>();

public:
    void set(T key, U val)
    {
        for (int i = 0; i < KeyList.size(); i++)
        {
            if (KeyList.get(i) == key)
            {
                ValList.set(i, val);
                return;
            }
        }
        KeyList.add(key);
        ValList.add(val);
    }

    U get(T key)
    {
        for (int i = 0; i < KeyList.size(); i++)
        {
            if (KeyList.get(i) == key)
            {
                return ValList.get(i);
            }
        }

        return "";
    }

    U get(int i)
    {
        if (i < KeyList.size())
        {
            return ValList.get(i);
        }
        else
        {
            return "";
        }
    }

    T getKey(U val)
    {
        for (int i = 0; i < ValList.size(); i++)
        {
            if (ValList.get(i) == val)
            {
                return KeyList.get(i);
            }
        }
        
        return "";
    }

    T getKey(int i)
    {
        if (i < KeyList.size())
        {
            return KeyList.get(i);
        }
        else
        {
            return "";
        }
    }

    int length()
    {
        return KeyList.size();
    }
};

#endif