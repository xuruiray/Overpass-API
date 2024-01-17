/** Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Roland Olbricht et al.
 *
 * This file is part of Overpass_API.
 *
 * Overpass_API is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Overpass_API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Overpass_API.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DE__OSM3S___TEMPLATE_DB__FILE_TOOLS_H
#define DE__OSM3S___TEMPLATE_DB__FILE_TOOLS_H

#include "types.h"

#include <map>
#include <set>
#include <vector>


/* Represents the connection to a client that is blocking after it has sent a command until
 * it gets an answer about the command execution state. This class ensures that the software cannot
 * fail due to a broken pipe. */
struct Blocking_Client_Socket
{
  Blocking_Client_Socket(int socket_descriptor_);
  uint32 get_command();
  std::vector< uint32 > get_arguments(int num_arguments); // the number of arguments is defined by the opcode
  void clear_state();
  void send_data(uint32 result);
  void send_result(uint32 result);
  ~Blocking_Client_Socket();
private:
  int socket_descriptor;
  enum { waiting, processing_command, disconnected } state;
  std::vector< uint32 > buffer;
  int bytes_in_buffer;
  int bytes_expected;
  int counter;
};


class Connection_Per_Pid_Map
{
public:
  typedef uint pid_t;
  typedef std::map< pid_t, Blocking_Client_Socket* >::const_iterator const_iterator;

  Connection_Per_Pid_Map() : last_pid(0) {}
  ~Connection_Per_Pid_Map();

  Blocking_Client_Socket* get(pid_t pid);
  void insert(pid_t pid, int socket_fd);
  void erase(pid_t pid);

  std::map< pid_t, Blocking_Client_Socket* >::size_type size() const { return data.size(); }
  const_iterator begin() const { return data.begin(); }
  const_iterator end() const { return data.end(); }

  void poll_command_round_robin(uint32& command, uint32& client_pid);

private:
  std::map< pid_t, Blocking_Client_Socket* > data;
  uint32 last_pid;
};


std::string getcwd();


#endif
