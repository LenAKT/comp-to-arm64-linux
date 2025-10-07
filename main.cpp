#include <iostream>
#include <string>
#include <sstream>
#include "subs/firstPass/reading.h"
#include "subs/secondPass/second.h"
#include "subs/backEnd/backEnd.h"

int main(int argc, char const *argv[])
{
    reading read;
    read.setMemory("Manifesto.txt");
    read.reader();
    read.firstPass();

    backEnd b(read.scope);

    Second second(read.scope, read.preTokens, b);
    second.secondPass();
    return 0;
}
