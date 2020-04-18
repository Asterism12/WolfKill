#pragma once
#include <cqcppsdk/cqcppsdk.h>

#include <iostream>
#include <map>
#include <set>

#include "player.h"
using namespace cq;
using namespace std;
enum class SetState { Uninitialized, Setting, Wait, Day, Night, Analyse };

class Set {
public:
    int16_t playerNum;
    SetState SetState;
    set<int64_t> PlayersNo;
    set<Player> Players;
    int64_t owner;

    void init(const GroupMessageEvent &event) {
        SetState = SetState::Setting;

        string msg = "房间已创建！\n";
        msg += "输入.9快速创建预女猎9人局\n";
        msg += "输入.12快速创建预女猎守白狼王12人局\n";
        
        send_group_message(event.group_id, msg);
    }

    void c9(const GroupMessageEvent &e) {
        SetState = SetState::Wait;
        playerNum = 9;
    }

    void c12(const GroupMessageEvent &e) {
        SetState = SetState::Wait;
        playerNum = 12;
    }

private:
    void set(const GroupMessageEvent &e) {
    
    }
};