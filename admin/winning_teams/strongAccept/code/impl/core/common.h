#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>
#include <sparsehash/dense_hash_set>

void splitStringVec(const char *s, const char delimiter, std::vector<std::string> &container);
void splitStringSet(const char *s, const char delimiter, google::dense_hash_set<std::string> &container);

#endif

