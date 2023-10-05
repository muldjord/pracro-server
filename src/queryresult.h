/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryresult.h
 *
 *  Mon May  5 15:47:41 CEST 2008
 *  Copyright 2008 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Pracro.
 *
 *  Pracro is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Pracro is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Pracro; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#ifndef __PRACRO_QUERYRESULT_H__
#define __PRACRO_QUERYRESULT_H__

#include <time.h>
#include <string>
#include <map>

#include <hugin.hpp>

class QueryResult {
public:
  time_t timestamp;
  std::string source;
  std::map< std::string, std::string > values;
  std::map< std::string, QueryResult > groups;

  void print(std::string tabs = "") {
    DEBUG(queryhandler,"%sTimestamp: %d\n", tabs.c_str(), (int)timestamp);
    DEBUG(queryhandler,"%sSource: %s\n", tabs.c_str(), source.c_str());
    DEBUG(queryhandler,"%sValues:\n", tabs.c_str());
    for(std::map< std::string, std::string >::iterator i = values.begin();
        i != values.end(); i++) {
      DEBUG(queryhandler,"%s[%s] => [%s]\n",
            tabs.c_str(), i->first.c_str(), i->second.c_str());
    }
    DEBUG(queryhandler,"%s{\n", tabs.c_str());
    for(std::map< std::string, QueryResult >::iterator i = groups.begin();
        i != groups.end(); i++) {
      DEBUG(queryhandler,"%s[%s] =>:\n", tabs.c_str(), i->first.c_str());
      i->second.print(tabs +"  ");
    }
    DEBUG(queryhandler,"%s}\n", tabs.c_str());
    
  }
};

#endif/*__PRACRO_QUERYRESULT_H__*/
