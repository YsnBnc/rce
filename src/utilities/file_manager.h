
#ifndef RCE_FILE_MANAGER_H
#define RCE_FILE_MANAGER_H
#pragma once
#include <bits/stdc++.h>
#include <fstream>

std::string read_file(const std::string & file_name);
void catch_file(const char *buffer);
std::string compile_file(const std::string & command);

#endif
