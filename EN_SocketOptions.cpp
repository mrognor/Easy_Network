#include "EN_SocketOptions.h"

namespace EN
{
    SocketOption::SocketOption() {}
    
    SocketOption::SocketOption(int level, int optionName, int optionValue)
    {
        Level = level;
        OptionName = optionName;
        OptionValue = optionValue;
    }

    PredefinedSocketOptions::PredefinedSocketOptions(std::vector<int> levels, std::vector<int> optionNames, std::vector<int> optionValues)
    {
        Levels = levels;
        OptionNames = optionNames;
        OptionValues = optionValues;
    }
}