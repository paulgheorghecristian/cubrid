//
// Created by paul on 28.03.2017.
//

#ifndef CUBRID_COMMANDEXECUTOR_H
#define CUBRID_COMMANDEXECUTOR_H

#include <string>
#include <vector>
#include <sstream>
#include "STAT_TOOL_StatisticsFile.hpp"
#include "STAT_TOOL_ErrorManager.hpp"

extern "C" {
#include "../../base/perfmon_base.h"
}

class CommandExecutor
{
  public:
    CommandExecutor (std::string &wholeCommand, std::vector<StatisticsFile *> &files);
    virtual ErrorManager::ErrorCode parseCommandAndInit() = 0;
    virtual ErrorManager::ErrorCode execute() = 0;
    virtual void printUsage() = 0;
    virtual ~CommandExecutor ();
  protected:
    std::vector<std::string> arguments;
    std::vector<std::string> possibleOptions;
    std::vector<StatisticsFile *> &files;
};

#endif //CUBRID_COMMANDEXECUTOR_H
