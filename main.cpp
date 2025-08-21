#include <iostream>
#include <string>
#include <sstream>
#include "subs/firstPass/reading.h"
#include "subs/secondPass/second.h"
#include "subs/backEnd/bEnd.h"

int main(int argc, char const *argv[])
{
    reading read;
    read.setMemory("Manifesto.txt");
    read.reader();
    read.firstPass();
    BEnd backEnd;
    Second second(read.scope, read.preTokens, backEnd);
    
    second.secondPass();

    return 0;
}
