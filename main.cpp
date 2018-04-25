
#include "feature.h"

int main(int argc, const char * argv[]) 
{
    auto r = getMatchedPoints(argv[1], argv[2]);
    writeIntoFile(r, argv[3]);
    return 0;
}
