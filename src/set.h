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
    SetState SetState;
    set<int64_t> PlayersNo;
    set<Player> Players;
    int64_t owner;

    void init(const GroupMessageEvent &e) {
        SetState = SetState::Setting;

        string msg = "房间已创建！\n";
        msg += "输入.9ren快速创建预女猎9人局\n";
        msg += "输入.12ren快速创建预女猎白12人局\n";
        msg += "输入.num+[数字]确定本局人数，如.num9";
        
        send_group_message(e.group_id, msg);
    }

    void control(const GroupMessageEvent &e) {
    
    }

private:
    void set(const GroupMessageEvent &e) {
    
    }
};