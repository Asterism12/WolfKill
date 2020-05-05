#include "set_new.h"
#include "player_new.hpp"

bool GameSet::init(vector<string> command, GroupMessageEvent event) {
    state = SetState::Setting;
    group = event.group_id;
    botNum = 0;
    debug = false;
    date = 0;
    host = event.user_id;
    if (command[2] == "9") {
        c9();
    } else if (command[2] == "白痴") {
        cIdiot();
    } else if (command[2] == "白狼王") {
        cWhite();
    } else if (command[2] == "5") {
        c5();
    } else {
        return false;
    }
    return true;
}

void GameSet::c5() {
    state = SetState::Waiting;
    rolePool.push_back(PlayerRole::Human);
    rolePool.push_back(PlayerRole::Wolf);
    rolePool.push_back(PlayerRole::WhiteWolf);
    rolePool.push_back(PlayerRole::Prophet);

    humanNum = 1;
    wolfNum = 2;
    godNum = 1;

    addPlayer(host);

    string msg = "本局为人狼白3人测试局，玩家输入.join加入游戏";
    send_group_message(group, msg);
}

void GameSet::c9() {
    state = SetState::Waiting;
    rolePool.push_back(PlayerRole::Prophet);
    rolePool.push_back(PlayerRole::Hunter);
    rolePool.push_back(PlayerRole::Witch);
    for (int i = 0; i < 3; i++) {
        rolePool.push_back(PlayerRole::Wolf);
        rolePool.push_back(PlayerRole::Human);
    }

    humanNum = 3;
    wolfNum = 3;
    godNum = 3;

    addPlayer(host);

    string msg = "本局为预女猎9人局，玩家输入.join加入游戏";
    send_group_message(group, msg);
}

void GameSet::cIdiot() {
    state = SetState::Waiting;
    rolePool.push_back(PlayerRole::Prophet);
    rolePool.push_back(PlayerRole::Hunter);
    rolePool.push_back(PlayerRole::Witch);
    rolePool.push_back(PlayerRole::Idiot);
    for (int i = 0; i < 4; i++) {
        rolePool.push_back(PlayerRole::Wolf);
        rolePool.push_back(PlayerRole::Human);
    }

    humanNum = 4;
    wolfNum = 4;
    godNum = 4;

    addPlayer(host);

    string msg = "本局为预女猎白12人局，玩家输入.join加入游戏";
    send_group_message(group, msg);
}

void GameSet::cWhite() {
    state = SetState::Waiting;
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

    addPlayer(host);

    string msg = "本局为预女猎守白狼王12人局，玩家输入.join加入游戏";
    send_group_message(group, msg);
}

void GameSet::addPlayer(int64_t player) {
    if (seat.size() == rolePool.size()) {
        send_group_message(group, "已达到人数上限。");
        return;
    }
    for (auto pl : seat) {
        if (pl == player) {
            send_group_message(group, "你已经参加过了。");
            return;
        }
    }
    seat.push_back(player);
    string msg = "房间人数：" + to_string(seat.size()) + "/" + to_string(rolePool.size());
    send_group_message(group, msg);
}

void GameSet::go() {
    if (seat.size() != rolePool.size()) {
        string msg = "人数不足。房间人数：" + to_string(seat.size()) + "/" + to_string(rolePool.size());
        send_group_message(group, msg);
        return;
    }

    //为人类玩家分配角色
    std::mt19937 gen(rd());
    for (int i = 0; i < seat.size(); i++) {
        std::uniform_int_distribution<> dis(0, rolePool.size() - 1);
        int index = dis(gen);
        switch (rolePool[index]) {
        case PlayerRole::Human:
            players[seat[i]] = new Human(seat[i], i);
            break;
        case PlayerRole::Wolf:
            players[seat[i]] = new Wolf(seat[i], i);
            wolfs[i] = -1;
            break;
        case PlayerRole::Prophet:
            players[seat[i]] = new Prophet(seat[i], i);
            break;
        case PlayerRole::Witch:
            players[seat[i]] = new Witch(seat[i], i);
            break;
        case PlayerRole::Hunter:
            players[seat[i]] = new Hunter(seat[i], i);
            break;
        case PlayerRole::Idiot:
            players[seat[i]] = new Idiot(seat[i], i);
            break;
        case PlayerRole::Guard:
            players[seat[i]] = new Guard(seat[i], i);
            break;
        case PlayerRole::WhiteWolf:
            players[seat[i]] = new WhiteWolf(seat[i], i);
            wolfs[i] = -1;
            break;
        default:
            break;
        }
        rolePool.erase(rolePool.begin() + index);
        if (host == seat[i]) {
            players[seat[i]]->isHost = true;
        }
    }

    seatShow();
    send_group_message(group, "随时使用.seat查看座位表\n");
    setPlayersState(PlayerState::Day, PlayerState::Ready);
    night();
}

void GameSet::night() {
    if (state == SetState::Analysing) {
        return;
    }
    date++;
    state = SetState::Night;
    send_group_message(group, "进入第" + to_string(date) + "天黑夜，在私聊中使用.me查看自己的身份信息和提示");
    setPlayersState(PlayerState::Action, PlayerState::Day);
    //初始化狼刀目标
    for (auto it = wolfs.begin(); it != wolfs.end(); it++) {
        it->second = -1;
    }
    generateCode();
}

void GameSet::morning() {
    if (state == SetState::Analysing) {
        return;
    }
    state = SetState::Morning;
    send_group_message(group, "夜晚所有玩家行动完成，房主使用.morning命令进入第二天白天");
}

void GameSet::day() {
    if (state == SetState::Analysing) {
        return;
    }
    handleDeathEvent();
    state = SetState::Day;
    setPlayersState(PlayerState::Day, PlayerState::Wait);
    send_group_message(
        group, "进入第" + to_string(date) + "天白天，房主使用.kill [-n]处死n号座位号的玩家，使用.night直接进入黑夜");
}

void GameSet::handleDeathEvent() {
    for (auto death = deathNote.begin(); death != deathNote.end(); death++) {
        int vote = 0; //狼刀-1，毒-2，女巫救+1，守卫守+1，得分不为0判定为死亡
        for (auto reason = death->second.begin(); reason != death->second.end(); reason++) {
            if (*reason == "wolfkill") {
                vote -= 1;
                continue;
            }
            if (*reason == "witchpoison") {
                vote -= 2;
                continue;
            }
            if (*reason == "witchsave") {
                vote += 1;
                continue;
            }
            if (*reason == "guard") {
                vote += 1;
                continue;
            }
        }
        //判定死亡
        if (vote != 0) {
            kill(players[death->first]->seat);
            deathNote.erase(death);
        }
    }
}

void GameSet::setPlayersState(PlayerState state, PlayerState lastState) {
    for (auto pl = players.begin(); pl != players.end(); pl++) {
        if (pl->second->state == lastState) {
            pl->second->state = state;
        }
    }
}

void GameSet::seatShow() {
    string msg = "座位表：\n";
    for (int i = 0; i < seat.size(); i++) {
        if (players[seat[i]]->state == PlayerState::Die) {
            msg += "[" + to_string(i) + "] " + to_string(seat[i]) + "(Die)" + '\n';
        } else {
            msg += "[" + to_string(i) + "] " + to_string(seat[i]) + '\n';
        }
    }
    send_group_message(group, msg);
}

void GameSet::daybreakCheck() {
    for (auto pl : players) {
        if (pl.second->state != PlayerState::Die && pl.second->state != PlayerState::Wait) {
            return;
        }
    }

    morning();
}

void GameSet::sendPrivateMessage(int64_t QQ, string msg) {
    if (debug) {
        msg += " send to " + to_string(QQ);
        send_private_message(host, msg);
    } else {
        send_private_message(QQ, msg);
    }
}

void GameSet::receiveGroupMessage(int64_t QQ, vector<string> command) {
    if (debug) {
        // debug模式，第二个参数为模拟发送者的QQ号
        if (command.size() <= 2) {
            return;
        }
        if (command[1] == "host") {
            QQ = host;
            command.erase(command.begin() + 1);
        } else {
            try {
                QQ = stoi(command[1]);
                command.erase(command.begin() + 1);
            } catch (invalid_argument) {
                return;
            } catch (out_of_range) {
                return;
            }
        }
    }

    if (players.count(QQ) == 0 && state != SetState::Waiting) {
        return;
    }

    if (command.size() <= 1) {
        return;
    }

    if (command[1] == "join" && state == SetState::Waiting) {
        addPlayer(QQ);
    } else if (command[1] == "go" && state == SetState::Waiting) {
        if (QQ == host) {
            go();
        }
    } else if (command[1] == "seat") {
        seatShow();
    } else if (command[1] == "role" && debug) {
        roleMessage();
    } else if (command[1] == "morning" && state == SetState::Night) {
        send_group_message(group, "夜晚尚未结束，仍有玩家没有结束行动或输入验证码");
    } else if (command[1] == "morning" && state == SetState::Morning) {
        day();
    } else if (command[1] == "night"&& state==SetState::Day) {
        night();
    } else if (command[1] == "kill" && state == SetState::Day) {
        //白天抗推
        if (QQ == host && command.size() == 3) {
            try {
                int NO = stoi(command[2]);
                if (NO >= 0 && NO < players.size()) {
                    if (players[seat[NO]]->state == PlayerState::Die) {
                        send_group_message(group, "该玩家之前已经死亡，请重新选择另一名玩家");
                    } else if (players[seat[NO]]->role == PlayerRole::Idiot) {
                        send_group_message(group, "该玩家的身份为白痴，玩家存活，游戏继续");
                        night();
                    } else {
                        kill(NO);
                        night();
                    }
                }
            } catch (invalid_argument) {
                return;
            } catch (out_of_range) {
                return;
            }
        }
    } else if (command[1] == "shoot" && state == SetState::Night) {
        //猎人开枪
        if (players[QQ]->role == PlayerRole::Hunter && players[QQ]->state == PlayerState::Die) {
            players[QQ]->act2(command, *this);
        }
    } else if (command[1] == "boom" && state == SetState::Day) {
        //白狼自爆
        if (players[QQ]->role == PlayerRole::WhiteWolf && players[QQ]->state == PlayerState::Day) {
            players[QQ]->act2(command, *this);
        }
    }
}

void GameSet::receivePrivateMessage(int64_t QQ, vector<string> command) {
    if (debug) {
        // debug模式，第二个参数为模拟发送者的QQ号
        if (command.size() <= 2) {
            return;
        }
        if (command[1] == "host") {
            QQ = host;
            command.erase(command.begin() + 1);
        } else {
            try {
                QQ = stoi(command[1]);
                command.erase(command.begin() + 1);
            } catch (invalid_argument) {
                return;
            } catch (out_of_range) {
                return;
            }
        }
    }
    if (players.count(QQ) == 0) {
        return;
    }

    if (command.size() <= 1) {
        return;
    }

    if (command[1] == "code" && state == SetState::Night) {
        sendPrivateMessage(QQ, "今晚的验证码为." + to_string(code) + " 回复验证码即可验证（注意数字前需要加.）");
    } else if (command[1] == "me" && state == SetState::Night) {
        players[QQ]->me(*this);
    } else if (state == SetState::Night) {
        //各玩家的晚间行动
        players[QQ]->act(command, *this);
        daybreakCheck();
    }
}

void GameSet::wolfTalk(string s) {
    for (auto wolf : wolfs) {
        sendPrivateMessage(wolf.first, s);
    }
}

void GameSet::wolfKillCheck() {
    //检测狼刀是否一致，一致时锁定狼刀目标
    bool locked = true;
    for (auto wolf = wolfs.begin(); wolf != wolfs.end(); wolf++) {
        if (wolf->second != wolfs.begin()->second) {
            locked = false;
        }
    }
    if (locked) {
        int16_t target = wolfs.begin()->second;
        if (target == 100) {
            for (auto pl = players.begin(); pl != players.end(); pl++) {
                if (pl->second->state == PlayerState::Die) {
                    continue;
                }
                if (pl->second->role == PlayerRole::Wolf || pl->second->role == PlayerRole::WhiteWolf) {
                    sendPrivateMessage(pl->second->playerQQ, "今晚锁定狼刀为：空刀，行动结束，请等待白天开始");
                }
                if (pl->second->role == PlayerRole::Witch) {
                    pl->second->react("今晚狼人空刀使用.nop结束行动或可以使用毒药", *this);
                    break;
                }
            }
        } else {
            deathNote[seat[target]].insert("wolfkill");
            for (auto pl = players.begin(); pl != players.end(); pl++) {
                if (pl->second->state == PlayerState::Die) {
                    continue;
                }
                if (pl->second->role == PlayerRole::Wolf || pl->second->role == PlayerRole::WhiteWolf) {
                    sendPrivateMessage(pl->second->playerQQ,
                                       "今晚锁定狼刀为：" + to_string(target) + "号玩家，行动结束，请等待白天开始");
                }
                if (pl->second->role == PlayerRole::Witch) {
                    pl->second->react("今晚" + to_string(target) + "号玩家中刀，可以使用.save命令使用解药", *this);
                    break;
                }
            }
        }
        for (auto wolf = wolfs.begin(); wolf != wolfs.end(); wolf++) {
            players[seat[wolf->first]]->state = PlayerState::Wait;
        }
    }
}

void GameSet::kill(int16_t seatNumber) {
    if (state == SetState::Analysing) {
        return;
    }
    players[seat[seatNumber]]->state = PlayerState::Die;
    send_group_message(group, to_string(seatNumber) + "号玩家死亡");

    if (players[seat[seatNumber]]->role == PlayerRole::Wolf
        || players[seat[seatNumber]]->role == PlayerRole::WhiteWolf) {
        wolfs.erase(seatNumber);
        wolfNum--;
    } else if (players[seat[seatNumber]]->role == PlayerRole::Human) {
        humanNum--;
    } else {
        godNum--;
    }

    winCheck();
}

void GameSet::winCheck() {
    if (humanNum == 0 || godNum == 0) {
        send_group_message(group, "游戏结束，狼人胜利\n");
        analyse();
    } else if (wolfNum == 0) {
        send_group_message(group, "游戏结束，好人胜利\n");
        analyse();
    }
}

void GameSet::analyse() {
    state = SetState::Analysing;
    roleMessage();
}

void GameSet::roleMessage() {
    string msg = "身份表：\n";
    for (int i = 0; i < seat.size();i++) {
        switch (players[seat[i]]->role) {
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
        case PlayerRole::Idiot:
            msg += "[" + to_string(i) + "] 白痴" + '\n';
            break;
        case PlayerRole::WhiteWolf:
            msg += "[" + to_string(i) + "] 白狼王" + '\n';
            break;
        case PlayerRole::Guard:
            msg += "[" + to_string(i) + "] 守卫" + '\n';
            break;
        default:
            break;
        }
    }
    send_group_message(group, msg);
}

void GameSet::generateCode() {
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, players.size() - 1);
    code = dis(gen);
}