#pragma once

#include <dirent.h>
#include "main.hpp"

class Directory {
public:
	Directory(const char* name) :
		path(name)
	{
		//TODO: Use datadir. rom:/ needs to get prepended...
		DIR* dir;
		struct dirent* ent;
		if ((dir = opendir(name)) == NULL)
		{
			printlog("failed to open directory '%s'", name);
			return;
		}
		while ((ent = readdir(dir)) != NULL)
		{
			std::string entry(ent->d_name);
			if (ent->d_name[0] != '.')
			{
				list.push_back(ent->d_name);
			}
		}
		closedir(dir);
		{
			std::vector<DynamicString> _sorted;
			list.snapshot(_sorted);
			std::sort(_sorted.begin(), _sorted.end());
			for ( size_t _si = 0; _si < _sorted.size(); ++_si ) { list.set((int64_t)_si, _sorted[_si]); }
		}
	}
	DynamicArrayStr list;
	const char* path;
};