#ifndef DATATYPES_DEFINED
#define DATATYPES_DEFINED

#include <cstring>
#include <map>
#include <set>
#include <vector>

#include "basic_types.h"
#include "type_node.h"
#include "type_way.h"
#include "type_relation.h"

using namespace std;

struct String_Object
{
  String_Object(string s) : value(s) {}
  String_Object(void* data) : value()
  {
    value = string(((int8*)data + 2), *(uint16*)data);
  }
  
  uint32 size_of() const
  {
    return value.length() + 2;
  }
  
  static uint32 size_of(void* data)
  {
    return *(uint16*)data + 2;
  }
  
  void to_data(void* data) const
  {
    *(uint16*)data = value.length();
    memcpy(((uint8*)data + 2), value.data(), value.length());
  }
  
  bool operator<(const String_Object& index) const
  {
    return this->value < index.value;
  }
  
  bool operator==(const String_Object& index) const
  {
    return this->value == index.value;
  }
  
  string val() const
  {
    return value;
  }
  
  protected:
    string value;
};

struct Pair_Comparator_By_Id {
  bool operator() (const pair< uint32, bool >& a, const pair< uint32, bool >& b)
  {
    return (a.first < b.first);
  }

  bool operator() (const pair< uint32, uint32 >& a, const pair< uint32, uint32 >& b)
  {
    return (a.first < b.first);
  }
};

struct Pair_Equal_Id {
  bool operator() (const pair< uint32, bool >& a, const pair< uint32, bool >& b)
  {
    return (a.first == b.first);
  }
  
  bool operator() (const pair< uint32, uint32 >& a, const pair< uint32, uint32 >& b)
  {
    return (a.first == b.first);
  }
};

struct Tag_Entry
{
  uint32 index;
  string key;
  string value;
  vector< uint32 > ids;
};

struct Tag_Index_Local
{
  uint32 index;
  string key;
  string value;
  
  Tag_Index_Local() {}
  
  Tag_Index_Local(void* data)
  {
    index = (*((uint32*)data + 1))<<8;
    key = string(((int8*)data + 7), *(uint16*)data);
    value = string(((int8*)data + 7 + key.length()),
		   *((uint16*)data + 1));
  }
  
  uint32 size_of() const
  {
    return 7 + key.length() + value.length();
  }
  
  static uint32 size_of(void* data)
  {
    return (*((uint16*)data) + *((uint16*)data + 1) + 7);
  }
  
  void to_data(void* data) const
  {
    *(uint16*)data = key.length();
    *((uint16*)data + 1) = value.length();
    *((uint32*)data + 1) = index>>8;
    memcpy(((uint8*)data + 7), key.data(), key.length());
    memcpy(((uint8*)data + 7 + key.length()), value.data(),
	   value.length());
  }
  
  bool operator<(const Tag_Index_Local& a) const
  {
    if ((index & 0x7fffffff) != (a.index & 0x7fffffff))
      return ((index & 0x7fffffff) < (a.index & 0x7fffffff));
    if (index != a.index)
      return (index < a.index);
    if (key != a.key)
      return (key < a.key);
    return (value < a.value);
  }
  
  bool operator==(const Tag_Index_Local& a) const
  {
    if (index != a.index)
      return false;
    if (key != a.key)
      return false;
    return (value == a.value);
  }
};

struct Tag_Index_Global
{
  string key;
  string value;
  
  Tag_Index_Global() {}
  
  Tag_Index_Global(void* data)
  {
    key = string(((int8*)data + 4), *(uint16*)data);
    value = string(((int8*)data + 4 + key.length()),
		   *((uint16*)data + 1));
  }
  
  uint32 size_of() const
  {
    return 4 + key.length() + value.length();
  }
  
  static uint32 size_of(void* data)
  {
    return (*((uint16*)data) + *((uint16*)data + 1) + 4);
  }
  
  void to_data(void* data) const
  {
    *(uint16*)data = key.length();
    *((uint16*)data + 1) = value.length();
    memcpy(((uint8*)data + 4), key.data(), key.length());
    memcpy(((uint8*)data + 4 + key.length()), value.data(),
	   value.length());
  }
  
  bool operator<(const Tag_Index_Global& a) const
  {
    if (key != a.key)
      return (key < a.key);
    return (value < a.value);
  }
  
  bool operator==(const Tag_Index_Global& a) const
  {
    if (key != a.key)
      return false;
    return (value == a.value);
  }
};

/**
  * A dataset that is referred in the scripts by a variable.
  */
struct Set
{
  map< Uint32_Index, vector< Node_Skeleton > > nodes;
  map< Uint31_Index, vector< Way_Skeleton > > ways;
  map< Uint31_Index, vector< Relation_Skeleton > > relations;
};

#endif
