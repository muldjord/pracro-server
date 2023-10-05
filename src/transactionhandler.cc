/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            transactionhandler.cc
 *
 *  Fri Dec 18 10:00:50 CET 2009
 *  Copyright 2009 Bent Bisballe Nyeng
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
#include "transactionhandler.h"

#include "macroparser.h"
#include "templateparser.h"
#include "templateheaderparser.h"
#include "courseparser.h"
#include "configuration.h"
#include "luaquerymapper.h"
#include "luaresume.h"
#include "luaoncommit.h"
#include "queryhandlerpentominos.h"
#include "queryhandlerpracro.h"
#include "xml_encode_decode.h"
#include "widgetgenerator.h"
#include "journal.h"

#include "exception.h"

class NotFoundException : public Exception {
public:
  NotFoundException(Request &r)
  : Exception("Macro " + r.macro + " not found in template " + r.templ) {}
};

static std::string error_box(std::string message)
{
  std::string errorbox =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<pracro version=\"1.0\">\n"
    "  <error>" + message + "</error>\n"
    "</pracro>\n";
  return errorbox;
}

static std::string handleCommits(Transaction &transaction, Environment &env,
                                 Session &session)
{
  std::string answer;

  Commits::iterator i = transaction.commits.begin();
  while(i != transaction.commits.end()) {
    Commit &commit = *i;
    
    MacroParser mp(env.macrolist.getLatestVersion(commit.macro));
    mp.parse();
    Macro *macro = mp.getMacro();
    
    std::string resume;
    try {
      LUAResume luaresume(transaction, commit);
      luaresume.addScripts(macro->resume_scripts);
      luaresume.run();
      resume = luaresume.resultString();
      commit.fields["journal.resume"] = resume;
      session.commitMacro(transaction, commit, *macro);
    } catch(LUAScript::Exception &e) {
      throw e;
    }

    LUAOnCommit *oncommit = nullptr;
    if(macro->commit_scripts.size() != 0) {
      oncommit = new LUAOnCommit(transaction, commit);
      oncommit->addScripts(macro->commit_scripts);
    }

    if(resume != "" || oncommit != nullptr) {
      TemplateParser tp(env.templatelist.getLatestVersion(commit.templ));
      tp.parse();
      Template *templ = tp.getTemplate();
      
      session.journal()->addEntry(transaction, commit, resume, templ, oncommit);
    }

    i++;
  }

  return answer;
}

static std::string handleRequest(Request &request, Environment &env,
                                 Session &session)
{
  std::string answer;

  if(request.course == "" && request.templ == "" && request.macro == "")
    return "";

  if(request.course != "" && request.templ == "" && request.macro == "") {
    CourseParser cp(env.courselist.getLatestVersion(request.course));
    cp.parse();
    Course *course = cp.getCourse();
    answer += "  <course name=\"" + course->name + "\" title=\"" +
      course->title + "\">\n";
    
    std::vector< Template >::iterator ti = course->templates.begin();
    while(ti != course->templates.end()) {
      std::string tname = ti->name;
    
      TemplateHeaderParser tp(env.templatelist.getLatestVersion(tname));
      tp.parse();
      Template *templ = tp.getTemplate();

      answer += "    <template name=\"" + templ->name + "\" title=\"" +
        templ->title + "\"/>\n";

      ti++;
    }
  
    answer += "  </course>\n";

    return answer;
  }

  AutoBorrower<Database*> borrower(env.dbpool);
  Database *db = borrower.get();
  //    Database *db = session.database();
    
  DEBUG(server, "Handling request - macro: %s, template: %s\n",
        request.macro.c_str(), request.templ.c_str());
  
  // Read and parse the template file.
  TemplateParser tp(env.templatelist.getLatestVersion(request.templ));
  tp.parse();
  
  Template *templ = tp.getTemplate();
  
  answer += "  <template name=\"" + templ->name + "\" title=\"" +
    templ->title + "\">\n";
  
  bool foundmacro = false;
  
  // Generate the macro and return it to the client
  std::vector< Macro >::iterator mi2 = templ->macros.begin();
  while(mi2 != templ->macros.end()) {
    Macro &macro = (*mi2);
    
    if(macro.isHeader) {
      answer += "    <header caption=\"" + macro.caption + "\"/>\n";
      mi2++;
      continue;
    }
    
    size_t oldest = time(nullptr) - Conf::db_max_ttl;
    if(macro.ttl != "") oldest = time(nullptr) - atoi(macro.ttl.c_str());
    
    bool completed =
      db->checkMacro(request.patientid, macro.name, session.id(),
                     oldest);
    
    answer += "    <macro uid=\"42\" completed=";
    if(completed) answer += "\"true\"";
    else answer += "\"false\"";
    
    attributes_t attr;
    attr["name"] = macro.name;
    attr["version"] = macro.version;
    if(macro.caption != "") attr["caption"] = macro.caption;
    if(macro.requires != "") attr["requires"] = macro.requires;
    attr["static"] = macro.isStatic?"true":"false";
    attr["compact"] = macro.isCompact?"true":"false";
    attr["important"] = macro.isImportant?"true":"false";
    
    attributes_t::iterator ai = attr.begin();
    while(ai != attr.end()) {
      std::string name = ai->first;
      std::string value = ai->second;
      answer += " "+name+"=\"" + value + "\"";
      ai++;
    }
    
    if(macro.name == request.macro || macro.isStatic) {
      foundmacro = true;
      
      MacroParser mp(env.macrolist.getLatestVersion(macro.name));
      mp.parse();
      Macro *m = mp.getMacro();
      answer += " caption=\"" + m->widgets.attributes["caption"] + "\"";
      answer += ">\n";
      
      AutoBorrower<Artefact*> borrower(env.atfpool);
      Artefact *atf = borrower.get();
      
      LUAQueryMapper lqm;
      
      ////////////////////////
      std::vector< Query >::iterator qi = m->queries.begin();
      while(qi != m->queries.end()) {
        
        Query &query = *qi;
        std::string service = query.attributes["service"];
        
        if(service == "pentominos") {
          // Send the queries to Pentominos (if any)
          QueryHandlerPentominos qh(*atf, request.patientid,
                                    "pracrod"/*user*/);
          
          QueryResult queryresult = qh.exec(*qi);
          lqm.addQueryResult(queryresult);
        }
        
        if(service == "pracro") {
          // Send the queries to Pentominos (if any)
          QueryHandlerPracro qh(*db, request.patientid, session.id());
          
          QueryResult queryresult = qh.exec(*qi);
          lqm.addQueryResult(queryresult);
        }
        
        qi++;
      }
      
      // Handle scripts
      if(m->scripts.size()) {
        answer += "      <scripts>\n";
        
        std::vector< Script >::iterator spi = m->scripts.begin();
        while(spi != m->scripts.end()) {
          //              answer += "        <script language=\"" +
          //                spi->attributes["language"] + "\">";
          answer += "        <script>";
          
          if(spi->attributes.find("src") != spi->attributes.end()) {
            std::string file =
                  Conf::xml_basedir + "/include/" + spi->attributes["src"];
            FILE *fp = fopen(file.c_str(), "r");
            if(fp) {
              char buf[64];
              size_t sz;
              std::string inc;
              while((sz = fread(buf, 1, sizeof(buf), fp)) != 0) {
                inc.append(buf, sz);
              }
              fclose(fp);
              answer +="\n-- BEGIN INCLUDE: '"+spi->attributes["src"]+"'\n";
              answer += xml_encode(inc);
              answer +="\n-- END INCLUDE: '"+spi->attributes["src"]+"'\n";
            }
          } else {
            answer += xml_encode(spi->code);
          }
          answer += "</script>\n";
          spi++;
        }
        
        answer += "      </scripts>\n";
      }
      
      answer += widgetgenerator(request.patientid, session.id(), *m,
                                lqm, *db, oldest);
    } else {
      // only find macro title
      MacroParser mp(env.macrolist.getLatestVersion(macro.name));
      mp.parse();
      Macro *m = mp.getMacro();
      answer += " caption=\"" + m->widgets.attributes["caption"] + "\"";
      answer += ">\n";
      
    }
    
    if(completed) {
      std::string jresume = session.journal()->getEntry(macro.name);
      
      std::string state = "old";
      std::string resume = db->getResume(request.patientid, macro,
                                         oldest, session.id());
      if(session.journal()->dirty(macro.name)) {
        state = "dirty";
      } else {
        if(resume == jresume) {
          state = "new";
        } else {
          if(jresume != "") {
            state = "dirty";
            session.journal()->setDirty(macro.name);
          } else {
            state = "old";
          }
        }
      }
      
      answer += "      <resume state=\""+state+"\">";
      answer += xml_encode(resume);
      answer += "</resume>\n";
    }
    
    answer += "    </macro>\n";
    mi2++;
    
  }
  
  if(foundmacro == false && request.macro != "")
    throw NotFoundException(request);
  
  answer += "  </template>\n";

  return answer;
}

std::string handleTransaction(Request &request,
                              Transaction &transaction,
                              Environment &env,
                              Session &session)
{
  std::string answer;

  answer += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  answer += "<pracro version=\"1.0\">\n";

  try {
    answer += handleCommits(transaction, env, session);
  } catch( LUAScript::Exception &e ) {
    ERR(server, "Commit error: %s\n", e.msg.c_str());
    return error_box(xml_encode(e.msg));
  }

  try {
    answer += handleRequest(request, env, session);
  } catch( std::exception &e ) {
    ERR(server, "Request error: %s\n", e.what());
    return error_box(xml_encode(e.what()));
  }

  answer += "</pracro>\n";

  DEBUG(server, "Done handling transaction\n");
  DEBUG(serverxml, "%s\n", answer.c_str());

  return answer;
}

#ifdef TEST_TRANSACTIONHANDLER
//deps: session.cc configuration.cc journal.cc journal_uploadserver.cc journal_commit.cc mutex.cc debug.cc sessionserialiser.cc sessionparser.cc saxparser.cc environment.cc semaphore.cc artefact.cc xml_encode_decode.cc database.cc pracrodaopgsql.cc pracrodaotest.cc pracrodao.cc entitylist.cc macrolist.cc templatelist.cc macroheaderparser.cc templateheaderparser.cc versionstr.cc exception.cc log.cc inotify.cc client_connection.cc admin_connection.cc admin_rc.cc admin_export.cc transactionparser.cc templateparser.cc macroparser.cc queryhandlerpracro.cc luaquerymapper.cc queryhandlerpentominos.cc luaresume.cc luautil.cc widgetgenerator.cc widgetvalue.cc luascript.cc sessionheaderparser.cc courselist.cc luaoncommit.cc courseparser.cc
//cflags: -I.. -DWITHOUT_ARTEFACT -DWITHOUT_DB $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS) $(HTTPD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS) $(HTTPD_LIBS) $(LUA_LIBS) $(CURL_LIBS)
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_TRANSACTIONHANDLER*/
