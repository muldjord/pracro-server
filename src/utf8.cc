/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            utf8.cc
 *
 *  Tue Feb 27 19:18:23 CET 2007
 *  Copyright  2006 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Artefact.
 *
 *  Artefact is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Artefact is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Artefact; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "utf8.h"

#include <hugin.hpp>

#include <errno.h>

UTF8::UTF8(std::string encoding)
{
  this->encoding = encoding;
  if(encoding != "ISO-8859-1") throw UTF8CreateException("Encoding not implemented.");

  // ENCODE MAP
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = "";
  map_encode["�"] = " ";
  map_encode["�"] = "¡";
  map_encode["�"] = "¢";
  map_encode["�"] = "£";
  map_encode["�"] = "¤";
  map_encode["�"] = "¥";
  map_encode["�"] = "¦";
  map_encode["�"] = "§";
  map_encode["�"] = "¨";
  map_encode["�"] = "©";
  map_encode["�"] = "ª";
  map_encode["�"] = "«";
  map_encode["�"] = "¬";
  map_encode["�"] = "­";
  map_encode["�"] = "®";
  map_encode["�"] = "¯";
  map_encode["�"] = "°";
  map_encode["�"] = "±";
  map_encode["�"] = "²";
  map_encode["�"] = "³";
  map_encode["�"] = "´";
  map_encode["�"] = "µ";
  map_encode["�"] = "¶";
  map_encode["�"] = "·";
  map_encode["�"] = "¸";
  map_encode["�"] = "¹";
  map_encode["�"] = "º";
  map_encode["�"] = "»";
  map_encode["�"] = "¼";
  map_encode["�"] = "½";
  map_encode["�"] = "¾";
  map_encode["�"] = "¿";
  map_encode["�"] = "À";
  map_encode["�"] = "Á";
  map_encode["�"] = "Â";
  map_encode["�"] = "Ã";
  map_encode["�"] = "Ä";
  map_encode["�"] = "Å";
  map_encode["�"] = "Æ";
  map_encode["�"] = "Ç";
  map_encode["�"] = "È";
  map_encode["�"] = "É";
  map_encode["�"] = "Ê";
  map_encode["�"] = "Ë";
  map_encode["�"] = "Ì";
  map_encode["�"] = "Í";
  map_encode["�"] = "Î";
  map_encode["�"] = "Ï";
  map_encode["�"] = "Ð";
  map_encode["�"] = "Ñ";
  map_encode["�"] = "Ò";
  map_encode["�"] = "Ó";
  map_encode["�"] = "Ô";
  map_encode["�"] = "Õ";
  map_encode["�"] = "Ö";
  map_encode["�"] = "×";
  map_encode["�"] = "Ø";
  map_encode["�"] = "Ù";
  map_encode["�"] = "Ú";
  map_encode["�"] = "Û";
  map_encode["�"] = "Ü";
  map_encode["�"] = "Ý";
  map_encode["�"] = "Þ";
  map_encode["�"] = "ß";
  map_encode["�"] = "à";
  map_encode["�"] = "á";
  map_encode["�"] = "â";
  map_encode["�"] = "ã";
  map_encode["�"] = "ä";
  map_encode["�"] = "å";
  map_encode["�"] = "æ";
  map_encode["�"] = "ç";
  map_encode["�"] = "è";
  map_encode["�"] = "é";
  map_encode["�"] = "ê";
  map_encode["�"] = "ë";
  map_encode["�"] = "ì";
  map_encode["�"] = "í";
  map_encode["�"] = "î";
  map_encode["�"] = "ï";
  map_encode["�"] = "ð";
  map_encode["�"] = "ñ";
  map_encode["�"] = "ò";
  map_encode["�"] = "ó";
  map_encode["�"] = "ô";
  map_encode["�"] = "õ";
  map_encode["�"] = "ö";
  map_encode["�"] = "÷";
  map_encode["�"] = "ø";
  map_encode["�"] = "ù";
  map_encode["�"] = "ú";
  map_encode["�"] = "û";
  map_encode["�"] = "ü";
  map_encode["�"] = "ý";
  map_encode["�"] = "þ";
  map_encode["�"] = "ÿ";

  // DECODE MAP
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[""] = "�";
  map_decode[" "] = "�";
  map_decode["¡"] = "�";
  map_decode["¢"] = "�";
  map_decode["£"] = "�";
  map_decode["¤"] = "�";
  map_decode["¥"] = "�";
  map_decode["¦"] = "�";
  map_decode["§"] = "�";
  map_decode["¨"] = "�";
  map_decode["©"] = "�";
  map_decode["ª"] = "�";
  map_decode["«"] = "�";
  map_decode["¬"] = "�";
  map_decode["­"] = "�";
  map_decode["®"] = "�";
  map_decode["¯"] = "�";
  map_decode["°"] = "�";
  map_decode["±"] = "�";
  map_decode["²"] = "�";
  map_decode["³"] = "�";
  map_decode["´"] = "�";
  map_decode["µ"] = "�";
  map_decode["¶"] = "�";
  map_decode["·"] = "�";
  map_decode["¸"] = "�";
  map_decode["¹"] = "�";
  map_decode["º"] = "�";
  map_decode["»"] = "�";
  map_decode["¼"] = "�";
  map_decode["½"] = "�";
  map_decode["¾"] = "�";
  map_decode["¿"] = "�";
  map_decode["À"] = "�";
  map_decode["Á"] = "�";
  map_decode["Â"] = "�";
  map_decode["Ã"] = "�";
  map_decode["Ä"] = "�";
  map_decode["Å"] = "�";
  map_decode["Æ"] = "�";
  map_decode["Ç"] = "�";
  map_decode["È"] = "�";
  map_decode["É"] = "�";
  map_decode["Ê"] = "�";
  map_decode["Ë"] = "�";
  map_decode["Ì"] = "�";
  map_decode["Í"] = "�";
  map_decode["Î"] = "�";
  map_decode["Ï"] = "�";
  map_decode["Ð"] = "�";
  map_decode["Ñ"] = "�";
  map_decode["Ò"] = "�";
  map_decode["Ó"] = "�";
  map_decode["Ô"] = "�";
  map_decode["Õ"] = "�";
  map_decode["Ö"] = "�";
  map_decode["×"] = "�";
  map_decode["Ø"] = "�";
  map_decode["Ù"] = "�";
  map_decode["Ú"] = "�";
  map_decode["Û"] = "�";
  map_decode["Ü"] = "�";
  map_decode["Ý"] = "�";
  map_decode["Þ"] = "�";
  map_decode["ß"] = "�";
  map_decode["à"] = "�";
  map_decode["á"] = "�";
  map_decode["â"] = "�";
  map_decode["ã"] = "�";
  map_decode["ä"] = "�";
  map_decode["å"] = "�";
  map_decode["æ"] = "�";
  map_decode["ç"] = "�";
  map_decode["è"] = "�";
  map_decode["é"] = "�";
  map_decode["ê"] = "�";
  map_decode["ë"] = "�";
  map_decode["ì"] = "�";
  map_decode["í"] = "�";
  map_decode["î"] = "�";
  map_decode["ï"] = "�";
  map_decode["ð"] = "�";
  map_decode["ñ"] = "�";
  map_decode["ò"] = "�";
  map_decode["ó"] = "�";
  map_decode["ô"] = "�";
  map_decode["õ"] = "�";
  map_decode["ö"] = "�";
  map_decode["÷"] = "�";
  map_decode["ø"] = "�";
  map_decode["ù"] = "�";
  map_decode["ú"] = "�";
  map_decode["û"] = "�";
  map_decode["ü"] = "�";
  map_decode["ý"] = "�";
  map_decode["þ"] = "�";
  map_decode["ÿ"] = "�";
}

std::string UTF8::encode(std::string s)
{
  std::string ret;

  for(int i = 0; i < (int)s.length(); i++) {
    std::string c;

    if((unsigned char)s[i] <= 0x7F) c = s.substr(i, 1);
    else c = map_encode[s.substr(i, 1)];

    if(c.length() == 0) throw UTF8EncodeException("Unknown character in string");

    ret.append(c);

  }

  return ret;
 
}

std::string UTF8::decode(std::string s)
{
  std::string ret;

  int width = 1;
  for(int i = 0; i < (int)s.length(); i+=width) {
    if(/*(unsigned char)s[i]>=0x00&&*/(unsigned char)s[i] <= 0x7F) width = 1; // 00-7F	1 byte
    if((unsigned char)s[i] >= 0xC2 && (unsigned char)s[i] <= 0xDF) width = 2; // C2-DF	2 bytes
    if((unsigned char)s[i] >= 0xE0 && (unsigned char)s[i] <= 0xEF) width = 3; // E0-EF	3 bytes
    if((unsigned char)s[i] >= 0xF0 && (unsigned char)s[i] <= 0xF4) width = 4; // F0-F4	4 bytes

    std::string c;

    if(width == 1) c = s.substr(i, 1);
    else c = map_decode[s.substr(i, width)];

    if(c.length() == 0) throw UTF8DecodeException("Unknown character in string");

    ret.append(c);
  }

  return ret;
}

#ifdef TEST_UTF8
//deps: exception.cc log.cc
//cflags: -I..
//libs:
#include <test.h>

TEST_BEGIN;

  try {
    UTF8 utf8("ISO-8859-1");

    std::string a = "AaBb������";
    printf("a [%s]\n", a.c_str());
    std::string b = utf8.encode(a);
    printf("b [%s]\n", b.c_str());
    b = utf8.encode(b);
    printf("b [%s]\n", b.c_str());
    std::string c = utf8.decode(b);
    printf("c [%s]\n", c.c_str());
    c = utf8.decode(c);
    printf("c [%s]\n", c.c_str());

    if(a == c) return 0;
    else return 1;
  } catch(Exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return 1;
  }

TEST_END;

#endif//TEST_UTF8
