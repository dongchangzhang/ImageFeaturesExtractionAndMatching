
#include "feature.h"


int main(int argc, char * argv[]) 
{
    FeatureTool ft;
    if (argc < 4) {
        std::cerr << "Need 3 paras for this program!" << std::endl;
        return -1;
    }
    std::cout << "SIFT" << std::endl;
    auto r = ft.getMatchedPoints(argv[1], argv[2], FeatureTool::SIFT, FeatureTool::BOTH);
    std::string name1 = "sift" + std::string(argv[3]);
    ft.writeMatchedPointsIntoFile(r, name1);
    
    std::cout << "SURF" << std::endl;
    r = ft.getMatchedPoints(argv[1], argv[2], FeatureTool::SURF, FeatureTool::BOTH);
    std::string name4 = "surf" + std::string(argv[3]);
    ft.writeMatchedPointsIntoFile(r, name4);
    
    std::cout << "KAZE" << std::endl;
    r = ft.getMatchedPoints(argv[1], argv[2], FeatureTool::KAZE, FeatureTool::BOTH);
    std::string name2 = "kaze" + std::string(argv[3]);
    ft.writeMatchedPointsIntoFile(r, name2);
    
    std::cout << "AKAZE" << std::endl;
    r = ft.getMatchedPoints(argv[1], argv[2], FeatureTool::AKAZE, FeatureTool::BOTH);
    std::string name3 = "akaze" + std::string(argv[3]);
    ft.writeMatchedPointsIntoFile(r, name3);

    return 0;
}
