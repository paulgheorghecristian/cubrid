//
// Created by paul on 21.03.2017.
//

#include "StatisticsFile.h"

StatisticsFile::StatisticsFile (const std::string &filename, const std::string &alias) : filename (filename),
  alias (alias)
{
  FILE *binary_fp = NULL;
  struct tm *timestamp;
  char strTime[80];
  INT64 seconds, unpacked_seconds;
  time_t epochSeconds;

  binary_fp = fopen (filename.c_str(), "rb");
  if (binary_fp == NULL)
    {
      throw FileNotFoundException ("The provided file doesn't exist or I don't have permission to open it!");
    }
  else
    {
      fread (&seconds, sizeof (INT64), 1, binary_fp);
      OR_GET_INT64 (&seconds, &unpacked_seconds);
      relativeEpochSeconds = (time_t)unpacked_seconds;
      relativeTimestamp = *localtime (&relativeEpochSeconds);
    }

  strftime (strTime, 80, "%a %B %d %H:%M:%S %Y", &relativeTimestamp);
  printf ("Relative Timestamp = %s\n", strTime);

  while (fread (&seconds, sizeof (INT64), 1, binary_fp) > 0)
    {
      char *unpacked_stats = (char *)malloc (sizeof (UINT64) * (size_t)Utils::getNStatValues());
      OR_GET_INT64 (&seconds, &unpacked_seconds);
      epochSeconds = (time_t)unpacked_seconds;
      Snapshot *snapshot = new Snapshot (epochSeconds);
      fread (unpacked_stats, sizeof (UINT64), (size_t)Utils::getNStatValues(), binary_fp);
      perfmon_unpack_stats (unpacked_stats, snapshot->rawStats);
      snapshots.push_back (snapshot);
    }

  fclose (binary_fp);
}

StatisticsFile::Snapshot *StatisticsFile::getSnapshotByMinutes (unsigned int minutes)
{
  unsigned int seconds = minutes * 60;
  unsigned int i, j, mid;

  i = 0;
  j = (unsigned int)snapshots.size() - 1;

  if ((mktime (&snapshots[0]->timestamp) - relativeEpochSeconds) >= seconds)
    {
      return snapshots[0];
    }

  while (j-i > 1)
    {
      mid = (i+j) / 2;
      time_t sec = (mktime (&snapshots[mid]->timestamp) - relativeEpochSeconds);
      if (sec > seconds)
        {
          j = mid;
        }
      else if (sec < seconds)
        {
          i = mid;
        }
      else if (sec == seconds)
        {
          return snapshots[mid];
        }
    }

  return snapshots[j];
}

int StatisticsFile::getSnapshotIndexByMinutes (unsigned int minutes)
{
  unsigned int seconds = minutes * 60;
  unsigned int i, j, mid;

  i = 0;
  j = (unsigned int)snapshots.size() - 1;

  if ((mktime (&snapshots[0]->timestamp) - relativeEpochSeconds) >= seconds)
    {
      return 0;
    }

  while (j-i > 1)
    {
      mid = (i+j) / 2;
      time_t sec = (mktime (&snapshots[mid]->timestamp) - relativeEpochSeconds);
      if (sec > seconds)
        {
          j = mid;
        }
      else if (sec < seconds)
        {
          i = mid;
        }
      else if (sec == seconds)
        {
          return mid;
        }
    }

  return j;
}

void StatisticsFile::getIndicesOfSnapshotsByArgument (char *argument, int &index1, int &index2)
{
  char diffArgument[32];
  char alias[MAX_FILE_NAME_SIZE];
  unsigned int  minutes1, minutes2;
  int tmp;

  index1 = 0;
  index2 = (int)snapshots.size();

  sscanf (argument, "%[^(]%[^)]", alias, diffArgument);

  if (strcmp (alias, this->alias.c_str()) != 0)
    {
      index1 = -1;
      index2 = -1;
      return;
    }

  if (strchr (diffArgument, '-') != NULL)
    {
      sscanf (diffArgument, "(%d-%d", &minutes1, &minutes2);
      index1 = getSnapshotIndexByMinutes (minutes1);
      index2 = getSnapshotIndexByMinutes (minutes2);

      if (index1 > index2)
        {
          tmp = index2;
          index2 = index1;
          index1 = tmp;
        }
      return;
    }
}

StatisticsFile::Snapshot *StatisticsFile::getSnapshotByArgument (char *argument)
{
  char diffArgument[32];
  char alias[MAX_FILE_NAME_SIZE];
  unsigned int minutes1, minutes2;
  minutes1 = minutes2 = 0;

  sscanf (argument, "%[^(]%[^)]", alias, diffArgument);

  if (strcmp (alias, this->alias.c_str()) != 0)
    {
      return NULL;
    }

  if (strchr (diffArgument, '-') != NULL)
    {
      sscanf (diffArgument, "(%d-%d", &minutes1, &minutes2);
      Snapshot *s1 = getSnapshotByMinutes (minutes1);
      Snapshot *s2 = getSnapshotByMinutes (minutes2);
      return s1->difference (s2);
    }
  else
    {
      sscanf (diffArgument, "(%d", &minutes1);
      return getSnapshotByMinutes (minutes1);
    }
}

void StatisticsFile::printInTableForm (StatisticsFile::Snapshot *s1, StatisticsFile::Snapshot *s2, FILE *stream)
{
  int i;
  char timestamp1[20], timestamp2[20];

  if (s1 == NULL && s2 == NULL)
    {
      printf ("You must provide two existing aliases!\n");
      return;
    }

  UINT64 *stats1 = s1->rawStats;
  UINT64 *stats2 = s2->rawStats;

  strftime (timestamp1, 80, "%H:%M:%S", &s1->timestamp);
  strftime (timestamp2, 80, "%H:%M:%S", &s2->timestamp);
  printf ("\t\t\t\t\t\t\t      %s \t   %s \t  diff\n", timestamp1, timestamp2);

  for (i = 0; i < PSTAT_COUNT; i++)
    {
      if (pstat_Metadata[i].valtype == PSTAT_COMPLEX_VALUE)
        {
          break;
        }

      int offset = pstat_Metadata[i].start_offset;

      if (pstat_Metadata[i].valtype != PSTAT_COMPUTED_RATIO_VALUE)
        {
          if (pstat_Metadata[i].valtype != PSTAT_COUNTER_TIMER_VALUE)
            {
              if ( stats1[offset] == 0 && stats2[offset] == 0)
                {
                  continue;
                }

              fprintf (stream, "%-58s | %10lld | %10lld | %10lld\n", pstat_Metadata[i].stat_name,
                       (long long) stats1[offset],
                       (long long) stats2[offset],
                       difference ((long long)stats1[offset], (long long)stats2[offset]));
            }
          else
            {
              perfmon_compare_timer (stream, i, stats1, stats2);
            }
        }
      else
        {
          if (stats1[offset] == 0 && stats2[offset] == 0)
            {
              continue;
            }

          fprintf (stream, "%-58s | %10.2f | %10.2f | %10.2f\n", pstat_Metadata[i].stat_name,
                   (long long) stats1[offset] / 100.0f,
                   (long long) stats2[offset] / 100.0f,
                   difference ((long long)stats1[offset], (long long)stats2[offset])/100.0f);
        }
    }

  for (; i < PSTAT_COUNT; i++)
    {
      fprintf (stream, "\n%s:\n", pstat_Metadata[i].stat_name);
      pstat_Metadata[i].f_dump_diff_in_file (stream, & (stats1[pstat_Metadata[i].start_offset]),
                                             & (stats2[pstat_Metadata[i].start_offset]));
    }
}

StatisticsFile::~StatisticsFile()
{
  for (unsigned int i = 0; i < snapshots.size(); i++)
    {
      delete snapshots[i];
    }
}