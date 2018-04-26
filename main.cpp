
#include "feature.h"

int main(int argc, const char * argv[]) 
{
    FeatureTool ft;
    if (argc < 4) {
        std::cerr << "Need 3 paras for this program!" << std::endl;
        return -1;
    }
    auto r = ft.getMatchedPoints(argv[1], argv[2]);
    ft.writeMatchedPointsIntoFile(r, argv[3]);
    return 0;
}
