
#include "feature.h"


int main(int argc, const char * argv[]) 
{
    FeatureTool ft;
    if (argc < 4) {
        std::cerr << "Need 3 paras for this program!" << std::endl;
        return -1;
    }
    std::cout << "KAZE" << std::endl;
    auto r = ft.getMatchedPoints(argv[1], argv[2], FeatureTool::KAZE, FeatureTool::BOTH);
    ft.writeMatchedPointsIntoFile(r, argv[3]);
    return 0;
}
