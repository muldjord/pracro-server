/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            widgetgenerator.h
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
#ifndef __PRACRO_WIDGETGENERATOR_H__
#define __PRACRO_WIDGETGENERATOR_H__

#include <string>

#include "tcpsocket.h"
#include "template.h"
#include "luaquerymapper.h"
#include "database.h"

/**
 * This prettyprints the Macro datastructure as XML.
 * Furthermore it fills out the values of all the widgets, according to their value,
 * and map attributes.
 * If the value exists in the database it will be used if recent enough.
 * If the map points to a value that is more recent than the one in the database (if
 * there is one) it will be used.
 * If no data is available from the database or the map (or they are too old), the
 * value of the 'value' attribute will be used. 
 * @param cpr An std::string containing the patient id to use with the database lookup.
 * @param macro The Macro to prettyprint.
 * @param mapper The LUAQueryMapper to look for mappings in.
 * @param db The Database to search for recent values.
 * @return An std::srting containing the prettyprinted version of the Macro.
 */
std::string widgetgenerator(std::string cpr,
                            std::string sessionid,
                            Macro &macro,
                            LUAQueryMapper &mapper,
                            Database &db, time_t oldest);

#endif/*__PRACRO_WIDGETGENERATOR_H__*/
