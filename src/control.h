#pragma once
#include <cqcppsdk/cqcppsdk.h>

#include <iostream>
#include <set>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <string>

#include "set.h"

using namespace cq;
using namespace std;

vector<string> commandAnalyse(string message);
void groupControl(const GroupMessageEvent &event);
void privateControl(const PrivateMessageEvent &event);