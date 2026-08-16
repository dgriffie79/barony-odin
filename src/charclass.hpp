/*-------------------------------------------------------------------------------

BARONY
File: charclass.hpp
Desc: defines character classes

Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once
#include "main.hpp"
#include "stat.hpp"
#include "json.hpp"
#include "files.hpp"

class PlayerCharacterClassManager
{
	Stat* classStats = nullptr;
	int characterClass = CLASS_BARBARIAN;
public:
	PlayerCharacterClassManager(Stat* const myStats, const int charClass)
	{
		classStats = myStats;
		characterClass = charClass;
	};
	bool serialize(FileInterface* const file);

	void writeToFile();

	void readFromFile();

	void setCharacterStatsAfterSerialization() const;
};
