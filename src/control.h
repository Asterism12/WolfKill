#pragma once
#include <cqcppsdk/cqcppsdk.h>

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <random>

#include "set.h"

using namespace cq;
using namespace std;

vector<string> commandAnalyse(string message);
void groupControl(const GroupMessageEvent &event);
void privateControl(const PrivateMessageEvent &event);
void destorySet(int64_t set);