#include "EN_SocketOptions.h"

namespace EN
{
    PredefinedSocketOptions::PredefinedSocketOptions(std::vector<int> levels, std::vector<int> optionNames, std::vector<int> optionValues)
    {
        Levels = levels;
        OptionNames = optionNames;
        OptionValues = optionValues;
    }
}