#!/usr/bin/env bash

# Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Roland Olbricht et al.
#
# This file is part of Overpass_API.
#
# Overpass_API is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# Overpass_API is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with Overpass_API. If not, see <https://www.gnu.org/licenses/>.

if [[ -z $1  ]]; then
{
  echo Usage: $0 Replicate_id Source_dir Local_dir [Sleep]
  exit 0
};
fi

REPLICATE_ID=$1
SOURCE_DIR=$2
LOCAL_DIR=$3
SLEEP_BETWEEN_DLS=15 # How long to sleep between download attempts (sec). Default: 15. See also FAIL_COUNTER_ALERT
FAIL_COUNTER_ALERT=20 # After how many sleep cycles do we get nervous
FAIL_COUNTER=0
FILE_PANIC=

if [[ ! -d $LOCAL_DIR ]];
  then {
    mkdir $LOCAL_DIR
};
fi

EXEC_DIR="$(dirname $0)/"
if [[ ! "${EXEC_DIR:0:1}" == "/" ]]; then
{
  EXEC_DIR="$(pwd)/$EXEC_DIR"
}; fi

DB_DIR="$($EXEC_DIR/dispatcher --show-dir)"
if [[ "$REPLICATE_ID" == "auto" ]]; then
  if [[ ! -s "$DB_DIR/replicate_id" ]]; then
    echo "$DB_DIR/replicate_id does not exist and start set to auto"
    exit 1
  fi
  REPLICATE_ID=$(($(cat "$DB_DIR/replicate_id") + 0))
fi

# $1 - remote source
# $2 - local destination
fetch_file()
{
  wget -nv -O "$2" "$1"
};

retry_fetch_file()
{
  FILE_PANIC=
  FAIL_COUNTER=0
  if [[ ! -s "$2" ]]; then {
    fetch_file "$1" "$2"
    if [[ "$3" == "gzip" ]]; then {
      gunzip -t <"$2"
      if [[ $? -ne 0 ]]; then {
        rm "$2"
      }; fi
    }; fi
  }; fi
  until [[ -s "$2" || $FAIL_COUNTER -ge $FAIL_COUNTER_ALERT ]]; do {
    FAIL_COUNTER=$(($FAIL_COUNTER+1))
    sleep $SLEEP_BETWEEN_DLS
    fetch_file "$1" "$2"
    if [[ "$3" == "gzip" ]]; then {
      gunzip -t <"$2"
      if [[ $? -ne 0 ]]; then {
        rm "$2"
      }; fi
    }; fi
  }; done
  if [[ $FAIL_COUNTER -ge $FAIL_COUNTER_ALERT ]]; then
    FILE_PANIC="true"
  fi
};

file_panic()
{
  echo "fetch_osc()@"`date -u "+%F %T"`": upstream_delay $REPLICATE_ID" >>$LOCAL_DIR/fetch_osc.log
  REPLICATE_ID=$(($REPLICATE_ID - 1))

  printf -v TDIGIT3 %03u $(($REPLICATE_ID % 1000))
  ARG=$(($REPLICATE_ID / 1000))
  printf -v TDIGIT2 %03u $(($ARG % 1000))
  ARG=$(($ARG / 1000))
  printf -v TDIGIT1 %03u $ARG

  FILE_PANIC="true"
  until [[ ! -n $FILE_PANIC ]]; do {
    retry_fetch_file "$REMOTE_PATH/$TDIGIT3.state.txt" "$LOCAL_PATH/$TDIGIT3.new.state.txt" "text"
  }; done
  FILE_PANIC="true"
  until [[ ! -n $FILE_PANIC ]]; do {
    retry_fetch_file "$REMOTE_PATH/$TDIGIT3.osc.gz" "$LOCAL_PATH/$TDIGIT3.new.osc.gz" "gzip"
  }; done

  RES_GZIP=`diff -q "$LOCAL_PATH/$TDIGIT3.osc.gz" "$LOCAL_PATH/$TDIGIT3.new.osc.gz"`
  RES_TEXT=`diff -q "$LOCAL_PATH/$TDIGIT3.state.txt" "$LOCAL_PATH/$TDIGIT3.new.state.txt"`
  if [[ -n $RES_GZIP || -n $RES_TEXT ]]; then
    echo "fetch_osc()@"`date -u "+%F %T"`": file_panic $REPLICATE_ID" >>$LOCAL_DIR/fetch_osc.log
    echo "fetch_osc()@"`date -u "+%F %T"`": $RES_GZIP" >>$LOCAL_DIR/fetch_osc.log
    echo "fetch_osc()@"`date -u "+%F %T"`": $RES_TEXT" >>$LOCAL_DIR/fetch_osc.log
    exit 1
  fi

  rm "$LOCAL_PATH/$TDIGIT3.new.osc.gz"
  rm "$LOCAL_PATH/$TDIGIT3.new.state.txt"
};

fetch_minute_diff()
{
  printf -v TDIGIT3 %03u $(($REPLICATE_ID % 1000))
  ARG=$(($REPLICATE_ID / 1000))
  printf -v TDIGIT2 %03u $(($ARG % 1000))
  ARG=$(($ARG / 1000))
  printf -v TDIGIT1 %03u $ARG

  LOCAL_PATH="$LOCAL_DIR/$TDIGIT1/$TDIGIT2"
  REMOTE_PATH="$SOURCE_DIR/$TDIGIT1/$TDIGIT2"
  mkdir -p "$LOCAL_DIR/$TDIGIT1/$TDIGIT2"

  retry_fetch_file "$REMOTE_PATH/$TDIGIT3.state.txt" "$LOCAL_PATH/$TDIGIT3.state.txt" "text"
  if [[ -n $FILE_PANIC ]]; then
    file_panic
  fi
  retry_fetch_file "$REMOTE_PATH/$TDIGIT3.osc.gz" "$LOCAL_PATH/$TDIGIT3.osc.gz" "gzip"
  if [[ -n $FILE_PANIC ]]; then
    file_panic
  fi
  echo $REPLICATE_ID >"$LOCAL_DIR/replicate_id"

  TIMESTAMP_LINE=`grep timestamp $LOCAL_DIR/$TDIGIT1/$TDIGIT2/$TDIGIT3.state.txt`
  TIMESTAMP=${TIMESTAMP_LINE:10}
};

if [[ -s "$LOCAL_DIR/state.txt" ]]; then
  MAX_SEQ_NR=$(cat "$LOCAL_DIR/state.txt" | grep -aE '^sequenceNumber')
  MAX_AVAILABLE_REPLICATE_ID=$((${MAX_SEQ_NR:15} + 0))
  if [[ "$REPLICATE_ID" == "auto" && -s "$LOCAL_DIR/state.txt" ]]; then
    REPLICATE_ID=$MAX_AVAILABLE_REPLICATE_ID
  fi
fi

while [[ true ]];
do
{
  if [[ $REPLICATE_ID -gt $MAX_AVAILABLE_REPLICATE_ID ]]; then
    sleep 15
    rm -f "$LOCAL_DIR/state.txt"
    retry_fetch_file "$SOURCE_DIR/state.txt" "$LOCAL_DIR/state.txt" "text"
    if [[ -n $FILE_PANIC ]]; then
      file_panic
    fi
    MAX_SEQ_NR=$(cat "$LOCAL_DIR/state.txt" | grep -aE '^sequenceNumber')
    MAX_AVAILABLE_REPLICATE_ID=$((${MAX_SEQ_NR:15} + 0))
  fi

  if [[ $REPLICATE_ID -le $MAX_AVAILABLE_REPLICATE_ID ]]; then
    fetch_minute_diff
    echo "fetch_osc()@"`date -u "+%F %T"`": new_replicate_diff $REPLICATE_ID $TIMESTAMP" >>$LOCAL_DIR/fetch_osc.log
    REPLICATE_ID=$(($REPLICATE_ID + 1))
  fi
};
done
