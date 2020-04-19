#pragma once
#include <cqcppsdk/cqcppsdk.h>

#include <ctype.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <set>
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
    int64_t group;
    vector<PlayerRole> rolePool;
    int32_t code;
    vector<int16_t> diePeople;

    vector<pair<int16_t, int16_t>> wolfs;

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
        } else if (event.message == ".go" && setState == SetState::Waiting) {
            go(event);
        } else if (event.message == ".seat" && setState == SetState::Day) {
            seat(event);
        } else if (event.message == ".seat" && setState == SetState::Night) {
            seat(event);
        } else if (event.message == ".seat" && setState == SetState::Analysing) {
            seat(event);
        } else if (event.message == ".role" && setState == SetState::Analysing) {
            analyse();
        }
    }

    void humanAct(const PrivateMessageEvent &event) {
        if (to_string(code) == event.message) {
            for (Player pl : players) {
                if (pl.playerNo == event.user_id) {
                    pl.playerState = PlayerState::Wait;

                    string msg = "验证成功";
                    send_private_message(event.user_id, msg);
                    return;
                }
            }
            string msg = "你不在局里";
            send_private_message(event.user_id, msg);
        } else {
            string msg = "验证失败，当前验证码为：" + to_string(code);
            send_private_message(event.user_id, msg);
        }
    }

    void wolfAct(const PrivateMessageEvent &event) {
        if (event.message[0] != '.' || event.message.size() < 2) {
            return;
        }
        if (event.message.size() <= 3) {
            int no = 0;
            int limit = event.message.size();
            int i = 0;
            for (i = 1; i < limit; i++) {
                if (isdigit(event.message[i])) {
                    no = no * 10 + (event.message[i] - '0');
                } else {
                    break;
                }
            }
            if (i == limit) {
                if (no < playerMaxNum) {
                    
                }
            }
        }
        string msg = "你不在局里";
        send_private_message(event.user_id, msg);
    }

    void prophetAct(const PrivateMessageEvent &event) {
    }

    void witchAct(const PrivateMessageEvent &event) {
    }

    void hunterAct(const PrivateMessageEvent &event) {
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
        for (int64_t pl : playersNo) {
            if (pl == event.message_id) {
                string msg = "你已经参加过了。";
                send_group_message(event.group_id, msg);
                return;
            }
        }
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
        if (playerNum != playerMaxNum) {
            string msg = "人数不足";
            send_group_message(event.group_id, msg);
            return;
        }
        setState = SetState::Night;
        srand(time(0));
        int i = 0;
        while (rolePool.size() > 0) {
            int res = rand() % rolePool.size();
            players.push_back(Player(rolePool[res], playersNo[i]));
            if (rolePool[res] == PlayerRole::Wolf || rolePool[res] == PlayerRole::WhiteWolf) {
                wolfs.push_back(pair<int16_t, int16_t>(i, 0));
            }
            rolePool.erase(rolePool.begin() + res);
        }

        setState = SetState::Night;

        seat(event);
        string msg = "随时使用.seat查看座位表\n";
        msg += "第一天黑夜，请检查自己的私聊信息";
        send_group_message(event.group_id, msg);
    }

    void seat(const GroupMessageEvent &event) {
        string msg = "座位表：（QQ号码为-1为bot）\n";
        for (int i = 0; i < players.size(); i++) {
            if (players[i].playerState == PlayerState::Die) {
                msg += "[" + to_string(i) + "] " + to_string(players[i].playerNo) + '\n';
            } else {
                msg += "[" + to_string(i) + "] " + to_string(players[i].playerNo) + "(Die)" + '\n';
            }
        }
        send_group_message(event.group_id, msg);
    }

    void night(const GroupMessageEvent &event) {
        string msg;
        for (Player pl : players) {
            if (pl.playerState == PlayerState::Day) {
                pl.playerState = PlayerState::Action;
            }
            switch (pl.playerRole) {
            case PlayerRole::Prophet:
                msg = "你的身份是预言家，请选择今晚的验人";
                msg += "为减少场外，夜间狼人行动会向你发送验证码";
                break;
            case PlayerRole::Witch:
                msg = "你的身份是女巫，等待狼人行动\n";
                msg += "为减少场外，夜间狼人行动会向你发送验证码";
                break;
            case PlayerRole::Hunter:
                msg = "你的身份是猎人，等待女巫行动\n";
                msg += "为减少场外，夜间狼人行动会向你发送验证码";
                break;
            case PlayerRole::Wolf:
                msg = "你的身份是狼人，请选择今晚的刀口，使用.+n刀座位号为n的人，如.1\n";
                msg += ".66为空刀\n";
                msg += "在消息前+.可以把信息传给狼队友，如.我倒钩。仅限晚上生效\n";
                msg += "狼团队:";
                for (auto wolf : wolfs) {
                    msg += to_string(wolf.first) + ".";
                }
                break;
            case PlayerRole::Human:
                msg = "你的身份是平民，为减少场外，夜间狼人行动会向你发送验证码";
                break;
            default:
                break;
            }
            send_private_message(pl.playerNo, msg);
        }
    }

    void day() {
        for (int16_t person : diePeople) {
            if (person != 66) {
                die(person);
            }
        }
        winCheck();

        for (auto pl : players) {
            if (pl.playerState == PlayerState::Wait) {
                pl.playerState = PlayerState::Day;
            }
        }

        string msg = "天亮了\n";
        msg += "剩余玩家数：" + to_string(playerNum);
        msg += "房主可使用.+数字选择相应座位号的抗推的玩家，如.1";
        send_group_message(group, msg);
    }

    void winCheck() {
        int humanNum = 0, spiritNum = 0, wolfNum = 0;
        for (int i = 0; i < players.size(); i++) {
            if (players[i].playerRole == PlayerRole::Wolf || players[i].playerRole == PlayerRole::WhiteWolf) {
                wolfNum++;
            } else if (players[i].playerRole == PlayerRole::Human) {
                humanNum++;
            } else {
                spiritNum++;
            }
        }
        if (wolfNum == 0) {
            string msg = "狼人全灭，好人胜利\n";
            send_group_message(group, msg);
            setState = SetState::Analysing;
        }
        if (humanNum == 0 || spiritNum == 0) {
            string msg = "狼人屠边胜\n";
            send_group_message(group, msg);
            setState = SetState::Analysing;
        }

        setState = SetState::Analysing;
        analyse();
    }

    void analyse() {
        string msg = "使用.exit退出游戏状态，使用.role查看身份信息\n";
        send_group_message(group, msg);
    }

    void roleMessage(const GroupMessageEvent &event) {
        string msg = "身份表：\n";
        for (int i = 0; i < players.size(); i++) {
            switch (players[i].playerRole) {
            case PlayerRole::Human:
                msg += "[" + to_string(i) + "] 平民" + '\n';
                break;
            case PlayerRole::Hunter:
                msg += "[" + to_string(i) + "] 猎人" + '\n';
                break;
            case PlayerRole::Prophet:
                msg += "[" + to_string(i) + "] 预言家" + '\n';
                break;
            case PlayerRole::Witch:
                msg += "[" + to_string(i) + "] 女巫" + '\n';
                break;
            case PlayerRole::Wolf:
                msg += "[" + to_string(i) + "] 狼人" + '\n';
                break;
            default:
                break;
            }
        }
        send_group_message(event.group_id, msg);
    }

    void wakeCheck() {
        for (Player pl : players) {
            if (pl.playerState == PlayerState::Action) {
                return;
            }
        }
        setState = SetState::Day;
        day();
    }

    void wolfTeamTalk(string msg) {
        for (auto pl : players) {
            if (pl.playerRole == PlayerRole::Wolf || pl.playerRole == PlayerRole::WhiteWolf) {
                send_private_message(pl.playerNo, msg);
            }
        }
    }

    void wolfFocusCheck() {
        bool flag = false;
        if (wolfs.size() == 1) {
            flag = true;
        } else {
            for (int i = 1; i < wolfs.size(); i++) {
                if (wolfs[i].second != wolfs[i - 1].second) {
                    return;
                }
            }
            flag = true;
        }
        if (flag) {
            diePeople.push_back(wolfs[0].second);
            code = rand() % 10000;
            string msgHuman = "请在私聊中复读验证码：" + to_string(code);
            string msgWolf = "狼人已锁定目标为wolfs[0].second，无法继续更改";
            string msgWitch = "今天刀口为" + to_string(wolfs[0].second);
            for (auto pl : players) {
                if (pl.playerState == PlayerState::Die) {
                    continue;
                }
                switch (pl.playerRole) {
                case PlayerRole::Human:
                    send_private_message(pl.playerNo, msgHuman);
                    break;
                case PlayerRole::Hunter:
                    send_private_message(pl.playerNo, msgHuman);
                    break;
                case PlayerRole::Wolf:
                    pl.playerState = PlayerState::Wait;
                    send_private_message(pl.playerNo, msgWolf);
                    break;
                case PlayerRole::Witch:
                    if (pl.hasAntidote) {
                        send_private_message(pl.playerNo, msgWitch);
                    }
                    break;
                default:
                    break;
                }
            }
            wakeCheck();
        }
    }
    void die(int16_t no) {
        players[no].playerState = PlayerState::Die;
        if (players[no].playerRole == PlayerRole::Wolf) {
            for (auto it = wolfs.begin(); it != wolfs.end();it++) {
                if (it->first == no) {
                    wolfs.erase(it);
                    break;
                }
            }
        }

        string msg = to_string(no) + "号玩家倒牌";
        send_group_message(group, msg);

        winCheck();
    }
};