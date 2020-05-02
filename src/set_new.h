#pragma once
#include <string>
#include <map>
#include <set>

#include <cqcppsdk/cqcppsdk.h>

#include "player_new.hpp"
#include "control.h"

using namespace cq;
using namespace std;

enum class SetState { Uninitialized, Setting, Waiting, Day, Night, Analysing };

class GameSet {
public:
    int64_t group;//群号
    int64_t host;//主机玩家
    SetState state;
    int16_t code;//闭眼玩家的验证码，用于减少场外因素
    vector<int64_t> seat; //玩家列表，数组下标对应座位号
    map<int64_t, Player> players;
    int16_t day;
    map<int64_t, set<string>> deathNote;//每晚玩家与死亡有关的行动列表，<死亡玩家，行动列表>
    map<int16_t, int16_t> wolfs; //狼队座位号，狼刀目标

    void init(vector<string> command, GroupMessageEvent event);
    void c9();
    void cIdiot();
    void cWhite(); 
    void broadcast(string s);
    void wolfTalk(string s);
    void wolfKillCheck();
    void kill(int16_t seat);
}