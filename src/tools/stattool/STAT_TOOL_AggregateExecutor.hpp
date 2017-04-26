//
// Created by paul on 04.04.2017.
//

#ifndef CUBRID_STAT_TOOL_AGGREGATEEXECUTOR_H
#define CUBRID_STAT_TOOL_AGGREGATEEXECUTOR_H

#include "STAT_TOOL_CommandExecutor.hpp"
#include <vector>
#include <sstream>

extern "C" {
#include <stdlib.h>
}

class AggregateExecutor : public CommandExecutor
{
  public:
    AggregateExecutor (std::string &wholeCommand, std::vector<StatToolSnapshotSet *> &files);
    ErrorManager::ErrorCode parseCommandAndInit ();
    ErrorManager::ErrorCode execute ();
    void printUsage ();
    ~AggregateExecutor ();
  private:
    int fixedDimension;
    std::string fixedDimensionStr;
    int statIndex;
    std::string statName, aggregateName, plotFilename;
    StatToolSnapshotSet *file;
    FILE *gnuplotPipe;
};
#endif //CUBRID_STAT_TOOL_AGGREGATEEXECUTOR_H
