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
    Second second(read.scope, read.preTokens);
    second.secondPass();
    backEnd b(second.outerScope, second.irSender);
    // b.Printer(second.outerScope);
    b.start();
    return 0;
}
