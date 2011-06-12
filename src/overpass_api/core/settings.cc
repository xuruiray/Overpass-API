#include "datatypes.h"
#include "settings.h"

#include "../../template_db/file_blocks.h"
#include "../../template_db/file_blocks_index.h"
#include "../../template_db/random_file_index.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

template < typename TVal >
struct OSM_File_Properties : public File_Properties
{
  OSM_File_Properties(string file_base_name_, uint32 block_size_,
		      uint32 map_block_size_)
    : file_base_name(file_base_name_), block_size(block_size_),
      map_block_size(map_block_size_ > 0 ? map_block_size_*TVal::max_size_of() : 0) {}
  
  string get_file_name_trunk() const { return file_base_name; }
  
  string get_index_suffix() const { return basic_settings().INDEX_SUFFIX; }
  string get_data_suffix() const { return basic_settings().DATA_SUFFIX; }
  string get_id_suffix() const { return basic_settings().ID_SUFFIX; }  
  string get_shadow_suffix() const { return basic_settings().SHADOW_SUFFIX; }
  
  uint32 get_block_size() const { return block_size; }
  uint32 get_map_block_size() const { return map_block_size; }
  
  vector< bool > get_data_footprint(const string& db_dir) const
  {
    return get_data_index_footprint< TVal >(*this, db_dir);
  }
  
  vector< bool > get_map_footprint(const string& db_dir) const
  {
    return get_map_index_footprint(*this, db_dir);
  }
  
  uint32 id_max_size_of() const
  {
    return TVal::max_size_of();
  }
  
  File_Blocks_Index_Base* new_data_index
      (bool writeable, bool use_shadow, string db_dir, string file_name_extension)
      const
  {
    return new File_Blocks_Index< TVal >
        (*this, writeable, use_shadow, db_dir, file_name_extension);
  }

  string file_base_name;
  uint32 block_size;
  uint32 map_block_size;
};

//-----------------------------------------------------------------------------

Basic_Settings::Basic_Settings()
:
  max_allowed_space(512*1024*1024),
  max_allowed_time(3600*24),

  DATA_SUFFIX(".bin"),
  INDEX_SUFFIX(".idx"),
  ID_SUFFIX(".map"),
  SHADOW_SUFFIX(".shadow"),

  base_directory("./"),
  logfile_name("transactions.log"),
  shared_name_base("/osm3s_v0.6.90")
{}

Basic_Settings& basic_settings()
{
  static Basic_Settings obj;
  return obj;
}

//-----------------------------------------------------------------------------

Osm_Base_Settings::Osm_Base_Settings()
:
  NODES(new OSM_File_Properties< Uint32_Index >("nodes", 512*1024, 64*1024)),
  NODE_TAGS_LOCAL(new OSM_File_Properties< Tag_Index_Local >
      ("node_tags_local", 512*1024, 0)),
  NODE_TAGS_GLOBAL(new OSM_File_Properties< Tag_Index_Global >
      ("node_tags_global", 2*1024*1024, 0)),
      
  WAYS(new OSM_File_Properties< Uint31_Index >("ways", 512*1024, 64*1024)),
  WAY_TAGS_LOCAL(new OSM_File_Properties< Tag_Index_Local >
      ("way_tags_local", 512*1024, 0)),
  WAY_TAGS_GLOBAL(new OSM_File_Properties< Tag_Index_Global >
      ("way_tags_global", 2*1024*1024, 0)),
      
  RELATIONS(new OSM_File_Properties< Uint31_Index >("relations", 1024*1024, 64*1024)),
  RELATION_ROLES(new OSM_File_Properties< Uint32_Index >
      ("relation_roles", 512*1024, 0)),
  RELATION_TAGS_LOCAL(new OSM_File_Properties< Tag_Index_Local >
      ("relation_tags_local", 512*1024, 0)),
  RELATION_TAGS_GLOBAL(new OSM_File_Properties< Tag_Index_Global >
      ("relation_tags_global", 2*1024*1024, 0)),
      
  shared_name(basic_settings().shared_name_base + "_osm_base")
{}

const Osm_Base_Settings& osm_base_settings()
{
  static Osm_Base_Settings obj;
  return obj;
}

//-----------------------------------------------------------------------------

Area_Settings::Area_Settings()
:
  AREA_BLOCKS(new OSM_File_Properties< Uint31_Index >
      ("area_blocks", 512*1024, 64*1024)),
  AREAS(new OSM_File_Properties< Uint31_Index >("areas", 512*1024, 64*1024)),
  AREA_TAGS_LOCAL(new OSM_File_Properties< Tag_Index_Local >
      ("area_tags_local", 256*1024, 0)),
  AREA_TAGS_GLOBAL(new OSM_File_Properties< Tag_Index_Global >
      ("area_tags_global", 512*1024, 0)),
      
  shared_name(basic_settings().shared_name_base + "_areas")
{}

const Area_Settings& area_settings()
{
  static Area_Settings obj;
  return obj;
}

//-----------------------------------------------------------------------------

void show_mem_status()
{
  ostringstream proc_file_name_("");
  proc_file_name_<<"/proc/"<<getpid()<<"/stat";
  ifstream stat(proc_file_name_.str().c_str());
  while (stat.good())
  {
    string line;
    getline(stat, line);
    cerr<<line;
  }
  cerr<<'\n';
}

//-----------------------------------------------------------------------------

Logger::Logger(const string& db_dir)
  : logfile_full_name(db_dir + basic_settings().logfile_name) {}

void Logger::annotated_log(const string& message)
{
  // Collect current time in a user-readable form.
  time_t time_t_ = time(0);
  struct tm* tm_ = gmtime(&time_t_);
  char strftime_buf[21];
  strftime_buf[0] = 0;
  if (tm_)
    strftime(strftime_buf, 21, "%F %H:%M:%S ", tm_);
  
  ofstream out(logfile_full_name.c_str(), ios_base::app);
  out<<strftime_buf<<'['<<getpid()<<"] "<<message<<'\n';
}

void Logger::raw_log(const string& message)
{
  ofstream out(logfile_full_name.c_str(), ios_base::app);
  out<<message<<'\n';
}

string get_logfile_name()
{
  return basic_settings().logfile_name;
}
