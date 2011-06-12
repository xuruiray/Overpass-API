#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "../core/settings.h"
#include "../frontend/output.h"
#include "osm_updater.h"

using namespace std;

int main(int argc, char* argv[])
{
  // read command line arguments
  string db_dir, data_version;
  bool transactional = true;
  
  int argpos(1);
  while (argpos < argc)
  {
    if (!(strncmp(argv[argpos], "--db-dir=", 9)))
    {
      db_dir = ((string)argv[argpos]).substr(9);
      if ((db_dir.size() > 0) && (db_dir[db_dir.size()-1] != '/'))
	db_dir += '/';
      transactional = false;
    }
    if (!(strncmp(argv[argpos], "--version=", 10)))
      data_version = ((string)argv[argpos]).substr(10);
    ++argpos;
  }
  
  try
  {
    if (transactional)
    {
      Osm_Updater osm_updater(get_verbatim_callback(), data_version);
      //reading the main document
      osm_updater.parse_file_completely(stdin);
    }
    else
    {
      Osm_Updater osm_updater(get_verbatim_callback(), db_dir, data_version);
      //reading the main document
      osm_updater.parse_file_completely(stdin);
    }
  }
  catch (File_Error e)
  {
    report_file_error(e);
  }
  
  return 0;
}
