/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            hexify.cc
 *
 *  Mon Feb  7 08:42:59 CET 2011
 *  Copyright 2011 Bent Bisballe Nyeng
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
#include <stdio.h>

#define WIDTH 12

int main(int argc, char *argv[])
{
  if(argc < 2) {
    printf("Usage %s file\n", argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "r");
  if(!fp) {
    printf("Could not open %s\n", argv[1]);
    return 1;
  }

  int cnt = 0;
  printf("static const char val[] = {");
  while(!feof(fp)) {
    unsigned char c;
    int sz = fread(&c, 1, 1, fp);
    if(cnt) printf(", ");
    if(cnt % WIDTH == 0) printf("\n  ");
    printf("0x%02x", c);
    cnt++;
  }
  printf("\n};\n");
  printf("static int valsize = %d;\n", cnt);

  fclose(fp);

  return 0;
}

