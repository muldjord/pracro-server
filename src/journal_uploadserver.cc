/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            journal_uploadserver.cc
 *
 *  Mon Jun 21 12:55:38 CEST 2010
 *  Copyright 2010 Bent Bisballe Nyeng
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
#include "journal_uploadserver.h"

#include "journal_commit.h"

#define USE_MULTIPLE_USERS

static inline bool iswhitespace(char c)
{
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

/**
 * Remove all spaces, tabs and newline trailing the string.
 */
static std::string stripTrailingWhitepace(const std::string &str)
{
  if(str == "") return str;

  ssize_t end = str.size() - 1;

  while(end >= 0 && iswhitespace(str[end])) end--;
  end++;

  return str.substr(0, end);
}

static bool isInsideUTF8(const std::string &str, size_t idx)
{
  // Two byte character
  if(idx > 0 &&
     (str[idx] & 0xC0) == 0x80 &&
     (str[idx - 1] & 0xE0) == 0xC0)
    return true;

  // Three byte character
  if(idx > 1 &&
     (str[idx] & 0xC0) == 0x80 &&
     (str[idx - 1] & 0xC0) == 0x80 &&
     (str[idx - 2] & 0xF0) == 0xE0)
    return true;

  if(idx > 0 &&
     (str[idx] & 0xC0) == 0x80 &&
     (str[idx - 1] & 0xF0) == 0xE0)
    return true;

  // Four byte character
  if(idx > 2 &&
     (str[idx] & 0xC0) == 0x80 &&
     (str[idx - 1] & 0xC0) == 0x80 &&
     (str[idx - 2] & 0xC0) == 0x80 &&
     (str[idx - 3] & 0xF8) == 0xF0)
    return true;

  if(idx > 1 &&
     (str[idx] & 0xC0) == 0x80 &&
     (str[idx - 1] & 0xC0) == 0x80 &&
     (str[idx - 2] & 0xF8) == 0xF0)
    return true;

  if(idx > 0 &&
     (str[idx] & 0xC0) == 0x80 &&
     (str[idx - 1] & 0xF8) == 0xF0)
    return true;

  return false;
}

static size_t UTF8Length(const std::string &str)
{
  size_t size = 0;
  for(size_t i = 0; i < str.size(); i++) {
    if(!isInsideUTF8(str, i)) size++;
  }
  return size;
}

/**
 * Find all lines longer than 'width', and insert a newline in the
 * first backward occurring space. Force split any lines without a space.
 */
static std::string addNewlines(const std::string &str, size_t width)
{
  std::string output;
  size_t len = 0;
  for(size_t i = 0; i < str.size(); i++) {
    char c = str[i];

    /*
    fprintf(stderr, "i: %d, char: '%c', width: %d, len: %d, output: '%s'\n",
            i, c, width, len, output.c_str());
    */

    output += c;

    if(isInsideUTF8(str, i)) continue;

    len++;
    if(c == '\n') len = 0;

    // Try to split line at whitespace.
    if(len > width) {
      size_t p = 0;
      while(p < width) {
        p++;

        size_t pos = output.size() - p;

        if(isInsideUTF8(output, pos)) continue;

        if(iswhitespace(output[pos])) {
          output[pos] = '\n';
          len = UTF8Length(output.substr(pos+1));
          break;
        }
      }
    }

    // Force split line at current pos.
    if(len > width) {
      // replace last char with a newline, and append the character again, after the newline.
      output[output.size()-1] = '\n';
      output += c;
      len = 1;
    }
  }

  return output;
}

JournalUploadServer::JournalUploadServer(std::string host,
                                         unsigned short int port)
{
  this->host = host;
  this->port = port;
}

void JournalUploadServer::commit()
{
  int ret = 0;

#ifdef USE_MULTIPLE_USERS
  std::string resume;
  std::string olduser;
  
  // Iterate through all resumes, and create a string containing them all.
  std::map< int, ResumeEntry >::iterator i = entrylist.begin();
  while(i != entrylist.end()) {
    if(i->second.dirty) {
      i++;
      continue;
    }

    if(i->second.user != olduser && olduser != "" && resume != "") {
      ret = journal_commit(patientID().c_str(), olduser.c_str(),
                           host.c_str(), port,
                           resume.c_str(), resume.size());

      if(ret == -1) throw Journal::Exception("Journal Commit error.");

      // FIXME - UGLY HACK: Avoid upload server spooling in the wrong order.
      usleep(200000);
      resume = "";
    }

    olduser = i->second.user;

    if(resume != "") resume += "\n\n";
    //    resume += i->macro + "\n";
    resume += stripTrailingWhitepace(addNewlines(i->second.resume, 60));
    i++;
  }

  if(resume == "") return;

  ret = journal_commit(patientID().c_str(), olduser.c_str(),
                       host.c_str(), port,
                       resume.c_str(), resume.size());
#else
  std::string resume;
  std::string user;
  
  // Iterate through all resumes, and create a string containing them all.
  std::map< int, ResumeEntry >::iterator i = entrylist.begin();
  while(i != entrylist.end()) {
    if(i->second.dirty) {
      i++;
      continue;
    }

    if(user == "") {
      user = i->second.user;
    }

    if(resume != "") resume += "\n\n";
    resume += stripTrailingWhitepace(addNewlines(i->second.resume, 60));
    i++;
  }

  if(resume == "") return;

  // Connect to praxisuploadserver and commit all resumes in one bulk.
  ret = journal_commit(patientID().c_str(), user.c_str(),
                       host.c_str(), port,
                       resume.c_str(), resume.size());
#endif/*USE_MULTIPLE_USERS*/

  if(ret == -1) throw Journal::Exception("Journal Commit error.");
}


#ifdef TEST_JOURNAL_UPLOADSERVER
//deps: debug.cc log.cc journal.cc journal_commit.cc mutex.cc luascript.cc luautil.cc saxparser.cc configuration.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS) $(EXPAT_CFLAGS)
//libs: $(PTHREAD_LIBS) $(LUA_LIBS) $(CURL_LIBS) $(EXPAT_LIBS)
#include "test.h"

#define LONG "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do\neiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.           \n\n    \t";

TEST_BEGIN;

TEST_EQUAL_STR(stripTrailingWhitepace
               ("Lorem ipsum dolor sit amet.           \n\n    \t"),
                "Lorem ipsum dolor sit amet.", "Test wspace remover.");

TEST_EQUAL_STR(stripTrailingWhitepace(""), "", "Test wspace remover on empty string.");

TEST_EQUAL_STR(stripTrailingWhitepace("\n\t "), "", "Test wspace remover on wspace-only string.");

TEST_EQUAL_STR(stripTrailingWhitepace("\n"), "", "Test wspace remover on newline only.");
TEST_EQUAL_STR(stripTrailingWhitepace("\t"), "", "Test wspace remover on tab only.");
TEST_EQUAL_STR(stripTrailingWhitepace("\r"), "", "Test wspace remover on space only.");
TEST_EQUAL_STR(stripTrailingWhitepace(" "), "", "Test wspace remover on space only.");

TEST_EQUAL_STR(stripTrailingWhitepace("ø "), "ø", "Test wspace remover on utf-8 char.");
TEST_EQUAL_STR(stripTrailingWhitepace("ø"), "ø", "Test wspace remover on utf-8 char only.");

TEST_EQUAL_STR(stripTrailingWhitepace("a "), "a", "Test wspace remover on single char only.");
TEST_EQUAL_STR(stripTrailingWhitepace("a"), "a", "Test wspace remover on single char only.");

TEST_EQUAL_STR(addNewlines
               ("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do.", 60),
                "Lorem ipsum dolor sit amet, consectetur adipisicing elit,\nsed do.",
               "Test single linesplit.");

TEST_EQUAL_STR(addNewlines
               ("Lorem ipsum dolor sit amet, consectetur adipisicing elit, øsed do.", 60),
                "Lorem ipsum dolor sit amet, consectetur adipisicing elit,\nøsed do.",
               "Test single linesplit around utf-8 char.");

TEST_EQUAL_STR(addNewlines
               ("Lorem ipsum dolor sit amet, consectetur adipisicing elitø, sed do.", 60),
                "Lorem ipsum dolor sit amet, consectetur adipisicing elitø,\nsed do.",
               "Test single linesplit around utf-8 char.");

TEST_EQUAL_STR(addNewlines
               ("Lorem\nipsum dolor sit amet.", 12),
                "Lorem\nipsum dolor\nsit amet.",
               "Test single linesplit with contained newline.");

TEST_EQUAL_STR(addNewlines
               ("Lorem ipsum dolor sitan met.", 11),
                "Lorem ipsum\ndolor sitan\nmet.",
               "Test single linesplit on exact border.");

TEST_EQUAL_STR(addNewlines
               ("Loremipsum", 6),
                "Loremi\npsum",
               "Test single linesplit inside word.");

TEST_EQUAL_STR(addNewlines
               ("abc Loremipsum", 6),
                "abc\nLoremi\npsum",
               "Test single linesplit inside word.");

TEST_TRUE(isInsideUTF8("ø", 1), "Test positive utf8 match.");
TEST_TRUE(isInsideUTF8("aæb", 2), "Test positive utf8 match.");
TEST_TRUE(isInsideUTF8("aøb", 2), "Test positive utf8 match.");
TEST_TRUE(isInsideUTF8("aåb", 2), "Test positive utf8 match.");
TEST_TRUE(isInsideUTF8("aÆb", 2), "Test positive utf8 match.");
TEST_TRUE(isInsideUTF8("aØb", 2), "Test positive utf8 match.");
TEST_TRUE(isInsideUTF8("aÅb", 2), "Test positive utf8 match.");
TEST_FALSE(isInsideUTF8("ø", 0), "Test negative utf8 match.");
TEST_FALSE(isInsideUTF8("aæøb", 3), "Test negative utf8 match (between two utf8 chars).");
TEST_FALSE(isInsideUTF8("aøb", 0), "Test negative utf8 match (before utf8 char).");

TEST_FALSE(isInsideUTF8("𤭢", 0), "Test positive utf8 match, len 4.");
TEST_TRUE(isInsideUTF8("𤭢", 1), "Test positive utf8 match, len 4.");
TEST_TRUE(isInsideUTF8("𤭢", 2), "Test positive utf8 match, len 4.");
TEST_TRUE(isInsideUTF8("𤭢", 3), "Test positive utf8 match, len 4.");

TEST_FALSE(isInsideUTF8("€", 0), "Test positive utf8 match, len 3.");
TEST_TRUE(isInsideUTF8("€", 1), "Test positive utf8 match, len 3.");
TEST_TRUE(isInsideUTF8("€", 2), "Test positive utf8 match, len 3.");

TEST_FALSE(isInsideUTF8("¢", 0), "Test positive utf8 match, len 2.");
TEST_TRUE(isInsideUTF8("¢", 1), "Test positive utf8 match, len 2.");

TEST_EQUAL_INT(UTF8Length("ø"), 1, "Test utf8 string length.");
TEST_EQUAL_INT(UTF8Length("æø"), 2, "Test utf8 string length.");
TEST_EQUAL_INT(UTF8Length(""), 0, "Test utf8 string length.");
TEST_EQUAL_INT(UTF8Length("a"), 1, "Test utf8 string length.");
TEST_EQUAL_INT(UTF8Length("aø"), 2, "Test utf8 string length.");
TEST_EQUAL_INT(UTF8Length("aøb"), 3, "Test utf8 string length.");

TEST_EQUAL_INT(UTF8Length("a𤭢€¢ø𤭢€¢øa"), 10, "Test utf8 string length, combi.");

TEST_EQUAL_STR(stripTrailingWhitepace(addNewlines("", 60)), "", "Test on empty input.");

TEST_END;

#endif/*TEST_JOURNAL_UPLOADSERVER*/
