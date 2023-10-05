/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            widgetgenerator.cc
 *
 *  Mon May 19 09:58:41 CEST 2008
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
#include "widgetgenerator.h"

#include "widgetvalue.h"

#include "xml_encode_decode.h"

static std::string getWidgetString(Macro &macro,
                                   Widget &widget,
                                   std::string tabs,
                                   LUAQueryMapper &mapper,
                                   Values &values)
{
  std::string result;

  result = tabs + "<" + widget.attributes["tagname"];
  attr_t::iterator p = widget.attributes.begin();

  DEBUG(prefill, "TAG: %s - NAME: %s\n",
        widget.attributes["tagname"].c_str(),
        widget.attributes["name"].c_str());

  Value value;
  if(getValue(value, widget.attributes, macro.maps, mapper, values)) {
    widget.attributes["value"] = value.value;
    if(value.source != "") widget.attributes["prefilled"] = value.source;
  }

  while(p != widget.attributes.end()) {
    if(p->first != "tagname" && p->first != "map") {
      if( ! (p->first == "name" && p->second == "") )
        result += " " + p->first + "=\"" + xml_encode(p->second) + "\"";
    }
    p++;
  }

  if(widget.widgets.size() == 0) { // If node is empty, use short tag form
    result += "/>\n";
    return result;
  }

  result += ">\n";

  std::vector< Widget >::iterator w = widget.widgets.begin();
  while(w != widget.widgets.end()) {
    result += getWidgetString(macro, *w, tabs + "  ", mapper, values);
    w++;
  }
  result += tabs + "</" + widget.attributes["tagname"] + ">\n";

  return result;
}

static void get_fields(Widget &widget, Fieldnames &fields)
{
  if(widget.attributes.find("value") != widget.attributes.end()) {
    if(widget.attributes["name"] != "")
      fields.push_back(widget.attributes["name"]);
  }
  
  std::vector< Widget >::iterator w = widget.widgets.begin();
  while(w != widget.widgets.end()) {
    get_fields(*w, fields);
    w++;
  }
}

std::string widgetgenerator(std::string cpr, std::string sessionid,
                            Macro &macro, LUAQueryMapper &mapper, Database &db,
                            time_t oldest)
{
  Fieldnames fields;
  get_fields(macro.widgets, fields);

  Values values = db.getValues(cpr, fields, sessionid, oldest);
  
  return getWidgetString(macro, macro.widgets, "      ", mapper, values);
}

#ifdef TEST_WIDGETGENERATOR
//deps: log.cc mutex.cc debug.cc xml_encode_decode.cc widgetvalue.cc luaquerymapper.cc configuration.cc exception.cc luautil.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(LUA_CFLAGS)
//libs: $(PTHREAD_LIBS) $(LUA_LIBS)
#include <test.h>

#include <time.h>

#define PATIENTID "1234567890"

TEST_BEGIN;

TEST_TRUE(false, "No tests yet!");
/*
  time_t now = time(nullptr);

  printf("Test pretty printer:\n");
  {
    Macro macro;
    macro.widgets.attributes["tagname"] = "lineedit";
    macro.widgets.attributes["name"] = "mywidget";
    macro.widgets.attributes["value"] = "myvalue";
    macro.widgets.attributes["map"] = "mapvalue";
    macro.widgets.widgets.push_back(macro.widgets);

    Database db("testdb", "", "", "", "", "");
    LUAQueryMapper mapper;
    std::string result;
    Fields fields;
    QueryResult queryresult;

    // Test simple
    result = widgetgenerator(PATIENTID, macro, mapper, db);
    printf("[%s]\n", result.c_str());
    if(result != "      <lineedit name=\"mywidget\" value=\"myvalue\">\n"
       "        <lineedit name=\"mywidget\" value=\"myvalue\"/>\n"
       "      </lineedit>\n") return 1;
  }

  printf("Positive tests:\n");
  {
    Macro macro;
    macro.widgets.attributes["tagname"] = "lineedit";
    macro.widgets.attributes["name"] = "mywidget";
    macro.widgets.attributes["value"] = "myvalue";
    macro.widgets.attributes["map"] = "mapvalue";

    Database db("testdb", "", "", "", "", "");
    LUAQueryMapper mapper;
    std::string result;
    Fields fields;
    QueryResult queryresult;

    // Test simple
    result = widgetgenerator(PATIENTID, macro, mapper, db);
    printf("[%s]\n", result.c_str());
    if(result != "      <lineedit name=\"mywidget\" value=\"myvalue\"/>\n") return 1;

    // Make a database commit and test if the value shows up
    fields["mywidget"] = "testval";
    db.commitTransaction("testuser", PATIENTID, macro, fields, now);
    
    result = widgetgenerator(PATIENTID, macro, mapper, db);
    printf("[%s]\n", result.c_str());
    if(result != "      <lineedit name=\"mywidget\" value=\"testval\" prefilled=\"pracro\"/>\n") return 1;
    
    // Make a query result (newer than the db value) and see if it shows up
    queryresult.timestamp = now + 1;
    queryresult.source = "testsource";
    queryresult.values["mapvalue"] = "mymapvalue";
    mapper.addQueryResult(queryresult);
    
    result = widgetgenerator(PATIENTID, macro, mapper, db);
    printf("[%s]\n", result.c_str());
    if(result != "      <lineedit name=\"mywidget\" value=\"mymapvalue\" prefilled=\"testsource\"/>\n") return 1;
    
    // Make another db value (newer than the query result) and see if it shows up.
    fields["mywidget"] = "testval2";
    db.commitTransaction("testuser", PATIENTID, macro, fields, now + 2);
    
    result = widgetgenerator(PATIENTID, macro, mapper, db);
    printf("[%s]\n", result.c_str());
    if(result != "      <lineedit name=\"mywidget\" value=\"testval2\" prefilled=\"pracro\"/>\n") return 1;
  }

  printf("Negative tests:\n");
  {
    Macro macro;
    macro.widgets.attributes["tagname"] = "lineedit";
    macro.widgets.attributes["name"] = "mywidget";
    macro.widgets.attributes["value"] = "myvalue";
    macro.widgets.attributes["map"] = "mapvalue";

    Database db("testdb", "", "", "", "", "");
    LUAQueryMapper mapper;
    std::string result;
    Fields fields;
    QueryResult queryresult;

    // Test simple
    result = widgetgenerator(PATIENTID, macro, mapper, db);
    printf("[%s]\n", result.c_str());
    if(result != "      <lineedit name=\"mywidget\" value=\"myvalue\"/>\n") return 1;

    // Make a database commit too old, and test if the value shows up
    fields["mywidget"] = "testval";
    db.commitTransaction("testuser", PATIENTID, macro, fields, now - Conf::db_max_ttl - 1);
    
    result = widgetgenerator(PATIENTID, macro, mapper, db);
    printf("[%s]\n", result.c_str());
    if(result == "      <lineedit name=\"mywidget\" value=\"testval\" prefilled=\"pracro\"/>\n") return 1;
    
    // Make a too old query result (newer than the db value) and see if it shows up
    queryresult.timestamp = now - Conf::pentominos_max_ttl - 1;
    queryresult.source = "testsource";
    queryresult.values["mapvalue"] = "mymapvalue";
    mapper.addQueryResult(queryresult);
    
    result = widgetgenerator(PATIENTID, macro, mapper, db);
    printf("[%s]\n", result.c_str());
    if(result == "      <lineedit name=\"mywidget\" value=\"mymapvalue\" prefilled=\"testsource\"/>\n") return 1;
  }
*/
TEST_END;

#endif/*TEST_WIDGETGENERATOR*/
