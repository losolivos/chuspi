/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DATABASEENV_H
#define DATABASEENV_H

#include "DatabaseWorkerPool.h"
#include "Implementation/CharacterDatabase.h"
#include "Implementation/LoginDatabase.h"
#include "Implementation/WorldDatabase.h"
#include "ResultSet.h"
#include "PreparedResultSet.h"
#include "PreparedStatement.h"
#include "QueryHolder.h"
#include "Transaction.h"
#include "Field.h"
#include "MySQLHelper.h"

extern DatabaseWorkerPool WorldDatabase;
extern DatabaseWorkerPool CharacterDatabase;
extern DatabaseWorkerPool LoginDatabase;

#endif

