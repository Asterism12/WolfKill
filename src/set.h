#pragma once
#include <cqcppsdk/cqcppsdk.h>

#include <iostream>
#include <map>
#include <set>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <string>

#include "player.h"
using namespace cq;
using namespace std;
enum class SetState { Uninitialized, Setting, Waiting, Day, Night, Analysing };

class Set {
public:
    int16_t playerMaxNum;
    int16_t playerNum;
    SetState setState;
    vector<int64_t> playersNo;
    vector<Player> players;
    int64_t owner;
    vector<PlayerRole> rolePool;
    

    void init(const GroupMessageEvent &event) {
        setState = SetState::Setting;

        string msg = "房间已创建！\n";
        msg += "输入.9快速创建预女猎9人局\n";
        msg += "输入.12快速创建预女猎守白狼王12人局\n";
        msg += "人满后房主可输入.go开始游戏\n";

        send_group_message(event.group_id, msg);
    }

    void control(const GroupMessageEvent &event) {
        if (event.message == ".9" && setState == SetState::Setting) {
            c9(event);
        } else if (event.message == ".12" && setState == SetState::Setting) {
            c12(event);
        } else if (event.message == ".join" && setState == SetState::Waiting) {
            addPlayer(event);
        } else if (event.message == ".add" && setState == SetState::Waiting) {
            addBot(event);
        }
    }

private:
    void c9(const GroupMessageEvent &event) {
        setState = SetState::Waiting;
        playerMaxNum = 9;
        playerNum = 0;
        owner = event.user_id;
        rolePool.push_back(PlayerRole::Prophet);
        rolePool.push_back(PlayerRole::Hunter);
        rolePool.push_back(PlayerRole::Witch);
        for (int i = 0; i < 3; i++) {
            rolePool.push_back(PlayerRole::Wolf);
            rolePool.push_back(PlayerRole::Human);
        }
            
        addPlayer(event);

        string msg = "本局为预女猎9人局，玩家输入.join加入游戏，房主输入.add添加bot，bot角色限制为民";
        send_group_message(event.group_id, msg);
    }

    void c12(const GroupMessageEvent &event) {
        setState = SetState::Waiting;
        playerMaxNum = 12;
        playerNum = 0;
        owner = event.user_id;
        rolePool.push_back(PlayerRole::Prophet);
        rolePool.push_back(PlayerRole::Hunter);
        rolePool.push_back(PlayerRole::Witch);
        rolePool.push_back(PlayerRole::Guard);
        rolePool.push_back(PlayerRole::WhiteWolf);
        rolePool.push_back(PlayerRole::Human);
        for (int i = 0; i < 3; i++) {
            rolePool.push_back(PlayerRole::Wolf);
            rolePool.push_back(PlayerRole::Human);
        }

        addPlayer(event);

        string msg = "本局为预女猎守白狼王12人局，玩家输入.join加入游戏，房主输入.add添加bot，bot角色限制为民";
        send_group_message(event.group_id, msg);
    }

    void addPlayer(const GroupMessageEvent &event) {
        if (playerNum == playerMaxNum) {
            string msg =
                "最大人数" + to_string(playerMaxNum) + "人，当前人数" + to_string(playerNum) + "人，已达到人数上线";
            send_group_message(event.group_id, msg);
        } else {
            playerNum++;
            playersNo.push_back(event.message_id);

            string msg = "最大人数" + to_string(playerMaxNum) + "人，当前人数" + to_string(playerNum) + "人。";
            send_group_message(event.group_id, msg);
        }
    }
    void addBot(const GroupMessageEvent &event) {
        if (playerNum == playerMaxNum) {
            string msg =
                "最大人数" + to_string(playerMaxNum) + "人，当前人数" + to_string(playerNum) + "人，已达到人数上线";
            send_group_message(event.group_id, msg);
        } else {
            for (auto it = rolePool.begin(); it != rolePool.end(); it++) {
                if (*it == PlayerRole::Human) {
                    players.push_back(Player(PlayerRole::Human, -1));
                    rolePool.erase(it);

                    playerNum++;
                    playersNo.push_back(-1);

                    string msg = "最大人数" + to_string(playerMaxNum) + "人，当前人数" + to_string(playerNum) + "人。";
                    send_group_message(event.group_id, msg);

                    break;
                }
            }
            string msg = "bot数量已达到平民数量上限";
            send_group_message(event.group_id, msg);
        }
    }

    void go(const GroupMessageEvent &event) {
        srand(time(0));
        int i = 0;
        while (rolePool.size() > 0) {
            int res = rand() % rolePool.size();
            players.push_back(Player(rolePool[res], playersNo[i]));
            rolePool.erase(rolePool.begin() + res);
        }
    }
};