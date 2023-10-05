/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            xml_encode_decode.cc
 *
 *  Mon Jun  9 10:19:33 CEST 2008
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
#include "xml_encode_decode.h"
#include <string.h>
/*
char xml_map[][2][16] =
  {
    { "&", "&amp;" }, // & must be first
    { "\'", "&apos;" },
    { "\"", "&qout;" },
    { ">", "&gt;" },
    { "<", "&lt;" },
    { "", "" } // End marker
  };
*/

char xml_map[][2][16] =
  {
    { "&", "&#38;" }, // & must be first
    { "\'", "&#39;" },
    { "\"", "&#34;" },
    { ">", "&#62;" },
    { "<", "&#60;" },
    { "", "" } // End marker
  };

#define MAX_MAPS 5

std::string xml_encode(std::string str)
{
  size_t pos;

  for( int map = 0; map < MAX_MAPS; map++ ) {
    pos = 0;
    while( ( pos = str.find(xml_map[map][0], pos) ) != std::string::npos) {
      str.replace(pos, strlen(xml_map[map][0]), xml_map[map][1]);
      pos += strlen(xml_map[map][1]);
    }
  }

  return str;
}

std::string xml_decode(std::string str)
{
  size_t pos;

  // Traverse backwards, to handle '&' last.
  for( int map = MAX_MAPS - 1; map > -1; map-- ) {
    pos = 0;
    while( ( pos = str.find(xml_map[map][1], pos) ) != std::string::npos) {
      str.replace(pos, strlen(xml_map[map][1]), xml_map[map][0]);
      pos += strlen(xml_map[map][0]);
    }
  }

  return str;
}

#ifdef TEST_XML_ENCODE_DECODE
//deps:
//cflags:
//libs:
#include <test.h>

TEST_BEGIN;

std::string in  = "&A<B>C\"D\'<>\"&amp;E<>";
std::string enc = xml_encode(in);
std::string denc = xml_encode(enc);
std::string dec = xml_decode(denc);
std::string ddec = xml_decode(dec);

TEST_EQUAL_STR(in, ddec, "compare");
TEST_EQUAL_STR(enc, dec, "compare");

TEST_END;

#endif/*TEST_XML_ENCODE_DECODE*/
