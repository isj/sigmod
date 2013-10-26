#include "core.h"

using namespace std;

void splitStringVec(const char *s, const char delimiter, vector<string> &container)
{
	string str = s;
	container.clear();
	int start = 0;
	while (true)
	{
		auto pos = str.find(' ', start);
		if (pos != string::npos)
		{
			container.push_back(str.substr(start, pos - start));
			start = pos + 1;
		} else {
			container.push_back(str.substr(start));
			return;
		}
	}
}

void splitStringSet(const char *s, const char delimiter, google::dense_hash_set<string> &container)
{
	string str = s;
	container.clear();
	int start = 0;
	while (true)
	{
		auto pos = str.find(' ', start);
		if (pos != string::npos)
		{
			container.insert(str.substr(start, pos - start));
			start = pos + 1;
		} else {
			container.insert(str.substr(start));
			return;
		}
	}
}

