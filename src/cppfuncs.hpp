/*-------------------------------------------------------------------------------

	BARONY
	File: cppfuncs.hpp
	Desc: contains functions for random, generic, recycled code that gets used in every project under the sun for menial tasks.

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/


#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "../odin/containers/dynamic_array.hpp"
#include "prng.hpp"


// DynamicArrayStr overload (for the name-list pickers)
inline DynamicString randomEntryFromVector(DynamicArrayStr& vector)
{
	if ( !vector.size() )
	{
		throw "Empty vector!";
	}

    static BaronyRNG rng;
	return vector.at(rng.rand() % vector.size());
}
