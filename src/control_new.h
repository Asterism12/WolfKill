#pragma once
#include <cqcppsdk/cqcppsdk.h>

#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <map>

#include <cqcppsdk/cqcppsdk.h>

#include "set_new.h"

using namespace cq;
using namespace std;


vector<string> commandAnalyse(string message);
bool groupControl(const GroupMessageEvent &event);
bool privateControl(const PrivateMessageEvent &event);
void destorySet(int64_t set);