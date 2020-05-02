#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <cqcppsdk/cqcppsdk.h>

#include "set_new.h"

using namespace cq;
using namespace std;

enum class PlayerRole { Human, Wolf, Prophet, Witch, Hunter, Guard, WhiteWolf, Idiot };
enum class PlayerState { Ready, Day, Action, Wait, Die, AFK };

class Player {
public:
    PlayerRole role;
    PlayerState state = PlayerState::Ready;
    int64_t playerQQ;
    bool isHost = false;
    int16_t seat;

    Player(PlayerRole pr, int64_t QQ, int16_t seatNumber) {
        role = pr;
        playerQQ = QQ;
        seat = seatNumber;
    }

    void act(vector<string> command, GameSet &set){};//各角色需要在私人聊天中处理的行动
    void lossGun(){};
};

class Human : public Player {
public:
    void act(vector<string> command, GameSet &set) {
        string failmsg = "验证失败，请检查验证码是否正确，使用.code查看验证码";
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() >= 2) {
            return;
        }
        if (command[1] == "code") {
            send_private_message(playerQQ, "当前验证码为" + to_string(set.code));
        }
        try {
            int code = stoi(command[1]);
            if (code = set.code) {
                state = PlayerState::Wait;
                send_private_message(playerQQ, "验证成功，请等待白天开始");
            } else {
                send_private_message(playerQQ, failmsg);
            }
        } catch (invalid_argument) {
            send_private_message(playerQQ, failmsg);
        } catch (out_of_range) {
            send_private_message(playerQQ, failmsg);
        }
    }
};

class Wolf : public Player {
public:
    void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        string msg;
        if (command[1] == "target") {
            msg = "当前狼团队中各玩家目标为：";
            for (auto p : set.wolfs) {
                msg += "\n" + to_string(p.first) + "号玩家想要刀" + to_string(p.second) + "号玩家";
            }
            send_private_message(playerQQ, msg);
            return;
        }
        if (command.size() == 2) {
            try {
                int NO = stoi(command[1]);
                if (NO < 0 || NO >= set.seat.size()) {
                    send_private_message(playerQQ, "你选择的座位号超出范围");
                    return;
                }
                auto target = &set.players.at(set.seat[NO]);
                if (target->state == PlayerState::Die) {
                    send_private_message(playerQQ, "该玩家已死亡，请重新选择目标");
                    return;
                }
                set.wolfs.at(seat) = NO; //设置狼刀
                set.wolfKillCheck();
                return;
            } catch (invalid_argument) {
            } catch (out_of_range) {
            }
        }

        //狼人讨论组
        for (int i = 1; i < command.size(); i++) {
            msg += command[i] + " ";
        }
        set.wolfTalk(msg);
    }
};

class Prophet {
public:
    void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() >= 2) {
            return;
        }
        try {
            int NO = stoi(command[1]);
            if (NO < 0 || NO >= set.seat.size()) {
                send_private_message(playerQQ, "你选择的座位号超出范围");
                return;
            }
            auto target = &set.players.at(set.seat[NO]);
            if (target->state == PlayerState::Die) {
                send_private_message(playerQQ, "该玩家已死亡，请重新选择目标");
                return;
            }
            string msg = to_string(NO) + "号玩家的身份为";
            switch (target->role) {
            case PlayerRole::WhiteWolf:
                msg += "狼人";
                break;
            case PlayerRole::WhiteWolf:
                msg += "狼人";
                break;
            default:
                msg += "好人";
                break;
            }
            state = PlayerState::Wait;
            msg += "，行动结束，请等待白天开始";
            send_private_message(playerQQ, msg);
        } catch (invalid_argument) {
        } catch (out_of_range) {
        }
    }
};

class Witch {
public:
    bool havePoison = true; //第几天使用了使用毒药，0为没有使用
    int haveAntidote = true; //第几天使用了使用解药，0为没有使用

    void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() >= 2) {
            return;
        }
        if (command[1] == "save") {
            if (!haveAntidote) {
                send_private_message(playerQQ, "你已经没有解药了");
                return;
            }
            for (auto death = set.deathNote.begin(); death != set.deathNote.end(); death++) {
                if (death->second.count("wolfkill") != 0) {
                    death->second.insert("witchsave");
                    haveAntidote = false;
                    state = PlayerState::Wait;
                    send_private_message(playerQQ, "已成功使用解药，行动结束，请等待白天开始");
                    return;
                }
            }
            send_private_message(playerQQ, "还没有玩家因为狼刀死亡");
            return;
        }
        try {
            int NO = stoi(command[1]);
            if (!haveAntidote) {
                send_private_message(playerQQ, "你已经没有毒药了");
                return;
            }
            if (NO < 0 || NO >= set.seat.size()) {
                send_private_message(playerQQ, "你选择的座位号超出范围");
                return;
            }
            auto target = &set.players.at(set.seat[NO]);
            if (target->state == PlayerState::Die) {
                send_private_message(playerQQ, "该玩家已死亡，请重新选择目标");
                return;
            }
            set.deathNote[set.seat[NO]].insert("witchpoison");
            havePoison = false;
            if (set.players[set.seat[NO]].role == PlayerRole::Hunter) {
                set.players[set.seat[NO]].lossGun();
            }
            state = PlayerState::Wait;
            send_private_message(playerQQ, "已成功使用毒药，行动结束，请等待白天开始");
            return;
        } catch (invalid_argument) {
        } catch (out_of_range) {
        }
    }
};

class Hunter {
public:
    bool enableShoot = true;
    void lossGun() {
        enableShoot = false;
        send_private_message(playerQQ, "女巫毒到了你，你死亡后将不能开枪");
    }

    void act(vector<string> command, GameSet &set) {
        string failmsg = "验证失败，请检查验证码是否正确，使用.code查看验证码";
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() >= 2) {
            return;
        }
        if (command[1] == "code") {
            send_private_message(playerQQ, "当前验证码为" + to_string(set.code));
        }
        try {
            int code = stoi(command[1]);
            if (code = set.code) {
                state = PlayerState::Wait;
                send_private_message(playerQQ, "验证成功，请等待白天开始");
            } else {
                send_private_message(playerQQ, failmsg);
            }
        } catch (invalid_argument) {
            send_private_message(playerQQ, failmsg);
        } catch (out_of_range) {
            send_private_message(playerQQ, failmsg);
        }
    }
};

class Guard {
public:
    int16_t lastTarget = -1;

    void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() >= 2) {
            return;
        }
        try {
            int NO = stoi(command[1]);
            if (NO == lastTarget) {
                send_private_message(playerQQ, "两夜不能守同一个人");
                return;
            }
            if (NO < 0 || NO >= set.seat.size()) {
                send_private_message(playerQQ, "你选择的座位号超出范围");
                return;
            }
            auto target = &set.players.at(set.seat[NO]);
            if (target->state == PlayerState::Die) {
                send_private_message(playerQQ, "该玩家已死亡，请重新选择目标");
                return;
            }
            set.deathNote[set.seat[NO]].insert("guard");
            lastTarget = NO;
            state = PlayerState::Wait;
            send_private_message(playerQQ, "守卫"+to_string(NO)+"号玩家，行动结束，请等待白天开始");
            return;
        } catch (invalid_argument) {
        } catch (out_of_range) {
        }
    }
};

class WhiteWolf {
public:
    void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        string msg;
        if (command[1] == "target") {
            msg = "当前狼团队中各玩家目标为：";
            for (auto p : set.wolfs) {
                msg += "\n" + to_string(p.first) + "号玩家想要刀" + to_string(p.second) + "号玩家";
            }
            send_private_message(playerQQ, msg);
            return;
        }
        if (command.size() == 2) {
            try {
                int NO = stoi(command[1]);
                if (NO < 0 || NO >= set.seat.size()) {
                    send_private_message(playerQQ, "你选择的座位号超出范围");
                    return;
                }
                auto target = &set.players.at(set.seat[NO]);
                if (target->state == PlayerState::Die) {
                    send_private_message(playerQQ, "该玩家已死亡，请重新选择目标");
                    return;
                }
                set.wolfs.at(seat) = NO; //设置狼刀
                set.wolfKillCheck();
                return;
            } catch (invalid_argument) {
            } catch (out_of_range) {
            }
        }

        //狼人讨论组
        for (int i = 1; i < command.size(); i++) {
            msg += command[i] + " ";
        }
        set.wolfTalk(msg);
    }
};

class Idiot {
public:
    void act(vector<string> command, GameSet &set) {
        string failmsg = "验证失败，请检查验证码是否正确，使用.code查看验证码";
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() >= 2) {
            return;
        }
        if (command[1] == "code") {
            send_private_message(playerQQ, "当前验证码为" + to_string(set.code));
        }
        try {
            int code = stoi(command[1]);
            if (code = set.code) {
                state = PlayerState::Wait;
                send_private_message(playerQQ, "验证成功，请等待白天开始");
            } else {
                send_private_message(playerQQ, failmsg);
            }
        } catch (invalid_argument) {
            send_private_message(playerQQ, failmsg);
        } catch (out_of_range) {
            send_private_message(playerQQ, failmsg);
        }
    }
};