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

    virtual void act(vector<string> command, GameSet &set){}; //各角色需要在私人聊天中处理的行动
    virtual void act2(vector<string> command, GameSet &set){}; //特殊角色的行动
    virtual void react(string msg, GameSet &set){}; //角色接受信息，如猎人被毒或告知女巫狼刀信息
    virtual void me(GameSet &set){}; //自身身份信息
};

class Human : public Player {
public:
    virtual void act(vector<string> command, GameSet &set) {
        string failmsg = "验证失败，请检查验证码是否正确，使用.code查看验证码";
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() > 2) {
            return;
        }
        try {
            int code = stoi(command[1]);
            if (code == set.code) {
                state = PlayerState::Wait;
                set.sendPrivateMessage(playerQQ, "验证成功，请等待白天开始");
            } else {
                set.sendPrivateMessage(playerQQ, failmsg);
            }
        } catch (invalid_argument) {
            set.sendPrivateMessage(playerQQ, failmsg);
        } catch (out_of_range) {
            set.sendPrivateMessage(playerQQ, failmsg);
        }
    }

    virtual void me(GameSet &set) {
        string msg = "你的身份是平民，为减少场外，夜间需要输入验证码，使用.code查看今晚的验证码";
        set.sendPrivateMessage(playerQQ, msg);
    }

    Human(int64_t QQ, int16_t seatNumber) {
        state = PlayerState::Ready;
        role = PlayerRole::Human;
        playerQQ = QQ;
        seat = seatNumber;
    }
};

class Wolf : public Player {
public:
    virtual void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        string msg;
        if (command[1] == "target") {
            msg = "当前狼团队中各玩家目标为：";
            for (auto p : set.wolfs) {
                if (p.second == -1) {
                    msg += "\n" + to_string(p.first) + "号玩家还没有指定自己的目标";
                } else if (p.second == 100) {
                    msg += "\n" + to_string(p.first) + "号玩家想要空刀";
                } else {
                    msg += "\n" + to_string(p.first) + "号玩家想要刀" + to_string(p.second) + "号玩家";
                }
            }
            set.sendPrivateMessage(playerQQ, msg);
            return;
        }
        if (command.size() == 2) {
            try {
                int NO = stoi(command[1]);
                if (NO == 100) {
                    set.wolfs.at(seat) = NO; //设置狼刀
                    set.wolfTalk(to_string(seat) + "号玩家想要空刀");
                    set.wolfKillCheck();
                    return;
                }
                if (NO < 0 || NO >= set.seat.size()) {
                    set.sendPrivateMessage(playerQQ, "你选择的座位号超出范围");
                    return;
                }
                auto target = &set.players.at(set.seat[NO]);
                if ((*target)->state == PlayerState::Die) {
                    set.sendPrivateMessage(playerQQ, "该玩家已死亡，请重新选择目标");
                    return;
                }
                set.wolfs.at(seat) = NO; //设置狼刀
                set.wolfTalk(to_string(seat) + "号玩家想要刀" + to_string(NO) + "号玩家");
                set.wolfKillCheck();
                return;
            } catch (invalid_argument) {
            } catch (out_of_range) {
            }
        }

        //狼人讨论组
        msg = to_string(seat) + "号玩家说:";
        for (int i = 1; i < command.size(); i++) {
            msg += command[i] + " ";
        }
        set.wolfTalk(msg);
    }

    virtual void me(GameSet &set) {
        string msg = "你的身份是狼人，请选择今晚的刀口，使用.[-n]刀座位号为n的人，如.1，\n";
        msg += ".100为空刀，\n";
        msg += "在消息前+.可以把信息传给狼队友，如.我倒钩。仅晚上可以使用，\n";
        msg += "使用.target查看当前狼团队的狼刀情况\n";
        msg += "狼团队:";
        for (auto wolf = set.wolfs.begin(); wolf != set.wolfs.end(); wolf++) {
            msg += to_string(wolf->first) + ",";
        }
        set.sendPrivateMessage(playerQQ, msg);
    }

    Wolf(int64_t QQ, int16_t seatNumber) {
        state = PlayerState::Ready;
        role = PlayerRole::Wolf;
        playerQQ = QQ;
        seat = seatNumber;
    }
};

class Prophet : public Player {
public:
    virtual void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() > 2) {
            return;
        }
        try {
            int NO = stoi(command[1]);
            if (NO < 0 || NO >= set.seat.size()) {
                set.sendPrivateMessage(playerQQ, "你选择的座位号超出范围");
                return;
            }
            auto target = &set.players.at(set.seat[NO]);
            if ((*target)->state == PlayerState::Die) {
                set.sendPrivateMessage(playerQQ, "该玩家已死亡，请重新选择目标");
                return;
            }
            string msg = to_string(NO) + "号玩家的身份为";
            switch ((*target)->role) {
            case PlayerRole::Wolf:
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
            set.sendPrivateMessage(playerQQ, msg);
        } catch (invalid_argument) {
        } catch (out_of_range) {
        }
    }

    virtual void me(GameSet &set) {
        string msg = "你的身份是预言家，.[-n]选择今晚的验人，如.1\n";
        set.sendPrivateMessage(playerQQ, msg);
    }

    Prophet(int64_t QQ, int16_t seatNumber) {
        state = PlayerState::Ready;
        role = PlayerRole::Prophet;
        playerQQ = QQ;
        seat = seatNumber;
    }
};

class Witch : public Player {
public:
    bool havePoison = true; //第几天使用了使用毒药，0为没有使用
    int haveAntidote = true; //第几天使用了使用解药，0为没有使用

    virtual void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() > 2) {
            return;
        }
        if (command[1] == "nop") {
            state = PlayerState::Wait;
            set.sendPrivateMessage(playerQQ, "你决定什么也不做，行动结束，请等待白天开始");
            return;
        }
        if (command[1] == "save") {
            if (!haveAntidote) {
                set.sendPrivateMessage(playerQQ, "你已经没有解药了");
                return;
            }
            for (auto death = set.deathNote.begin(); death != set.deathNote.end(); death++) {
                if (death->second.count("wolfkill") != 0) {
                    if (death->first == playerQQ && set.date > 1) {
                        set.sendPrivateMessage(playerQQ, "从第二晚开始，女巫不能自救");
                        return;
                    }
                    death->second.insert("witchsave");
                    haveAntidote = false;
                    state = PlayerState::Wait;
                    set.sendPrivateMessage(playerQQ, "已成功使用解药，行动结束，请等待白天开始");
                    return;
                }
            }
            set.sendPrivateMessage(playerQQ, "还没有玩家因为狼刀死亡");
            return;
        }
        try {
            int NO = stoi(command[1]);
            if (!havePoison) {
                set.sendPrivateMessage(playerQQ, "你已经没有毒药了");
                return;
            }
            if (NO < 0 || NO >= set.seat.size()) {
                set.sendPrivateMessage(playerQQ, "你选择的座位号超出范围");
                return;
            }
            auto target = &set.players.at(set.seat[NO]);
            if ((*target)->state == PlayerState::Die) {
                set.sendPrivateMessage(playerQQ, "该玩家已死亡，请重新选择目标");
                return;
            }
            set.deathNote[set.seat[NO]].insert("witchpoison");
            havePoison = false;
            if (set.players[set.seat[NO]]->role == PlayerRole::Hunter) {
                set.players[set.seat[NO]]->react("", set);
            }
            state = PlayerState::Wait;
            set.sendPrivateMessage(playerQQ, "已成功使用毒药，行动结束，请等待白天开始");
            return;
        } catch (invalid_argument) {
        } catch (out_of_range) {
        }
    }

    virtual void react(string msg, GameSet &set) {
        if (haveAntidote) {
            set.sendPrivateMessage(playerQQ, msg);
        }
    }

    virtual void me(GameSet &set) {
        string msg = "你的身份是女巫，等待狼人行动，\n";
        msg += ".[-n]向指定目标使用毒药，如.1，如果你还有解药，狼人刀人后会通知你，\n";
        msg += "输入.nop放弃本晚行动。注意，即使你没有任何药，夜晚也需要输入.nop来确认行动结束";
        set.sendPrivateMessage(playerQQ, msg);
    }

    Witch(int64_t QQ, int16_t seatNumber) {
        state = PlayerState::Ready;
        role = PlayerRole::Witch;
        playerQQ = QQ;
        seat = seatNumber;
    }
};

class Hunter : public Player {
public:
    bool enableShoot = true;

    virtual void act(vector<string> command, GameSet &set) {
        string failmsg = "验证失败，请检查验证码是否正确，使用.code查看验证码";
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() > 2) {
            return;
        }
        try {
            int code = stoi(command[1]);
            if (code == set.code) {
                state = PlayerState::Wait;
                set.sendPrivateMessage(playerQQ, "验证成功，请等待白天开始");
            } else {
                set.sendPrivateMessage(playerQQ, failmsg);
            }
        } catch (invalid_argument) {
            set.sendPrivateMessage(playerQQ, failmsg);
        } catch (out_of_range) {
            set.sendPrivateMessage(playerQQ, failmsg);
        }
    }

    virtual void act2(vector<string> command, GameSet &set) {
        if (!enableShoot) {
            return;
        }
        if (command.size() != 3) {
            return;
        }
        try {
            int NO = stoi(command[2]);
            if (NO >= 0 && NO < set.players.size()) {
                if (set.players[set.seat[NO]]->state == PlayerState::Die) {
                    send_group_message(set.group, "该玩家之前已经死亡，请重新选择另一名玩家");
                } else {
                    set.kill(NO);
                }
            }
        } catch (invalid_argument) {
            return;
        } catch (out_of_range) {
            return;
        }
    }

    virtual void react(string msg, GameSet &set) {
        enableShoot = false;
        set.sendPrivateMessage(playerQQ, "女巫毒到了你，你死亡后将不能开枪");
    }

    virtual void me(GameSet &set) {
        string msg = "你的身份是猎人，女巫对你使用了毒药时会通知你，届时你不能在死亡后开枪\n";
        msg += "为减少场外，夜间狼人行动会向你发送验证码，\n";
        msg += "死亡后在公屏上使用.shoot [-n]指定目标开枪，如.shoot 1";
        set.sendPrivateMessage(playerQQ, msg);
    }

    Hunter(int64_t QQ, int16_t seatNumber) {
        state = PlayerState::Ready;
        role = PlayerRole::Hunter;
        playerQQ = QQ;
        seat = seatNumber;
    }
};

class Guard : public Player {
public:
    int16_t lastTarget = -1;

    virtual void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() > 2) {
            return;
        }
        try {
            int NO = stoi(command[1]);
            if (NO == lastTarget) {
                set.sendPrivateMessage(playerQQ, "两夜不能守同一个人");
                return;
            }
            if (NO < 0 || NO >= set.seat.size()) {
                set.sendPrivateMessage(playerQQ, "你选择的座位号超出范围");
                return;
            }
            auto target = &set.players.at(set.seat[NO]);
            if ((*target)->state == PlayerState::Die) {
                set.sendPrivateMessage(playerQQ, "该玩家已死亡，请重新选择目标");
                return;
            }
            set.deathNote[set.seat[NO]].insert("guard");
            lastTarget = NO;
            state = PlayerState::Wait;
            set.sendPrivateMessage(playerQQ, "守卫"+to_string(NO)+"号玩家，行动结束，请等待白天开始");
            return;
        } catch (invalid_argument) {
        } catch (out_of_range) {
        }
    }

    virtual void me(GameSet &set) {
        string msg = "你的身份是守卫，.[-n]守卫指定目标，如.1，\n";
        msg += "注意连续两晚不能守同一人\n";
        set.sendPrivateMessage(playerQQ, msg);
    }

    Guard(int64_t QQ, int16_t seatNumber) {
        state = PlayerState::Ready;
        role = PlayerRole::Guard;
        playerQQ = QQ;
        seat = seatNumber;
    }
};

class WhiteWolf : public Player {
public:
    virtual void act(vector<string> command, GameSet &set) {
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        string msg;
        if (command[1] == "target") {
            msg = "当前狼团队中各玩家目标为：";
            for (auto p : set.wolfs) {
                if (p.second == -1) {
                    msg += "\n" + to_string(p.first) + "号玩家还没有指定自己的目标";
                } else if (p.second == 100) {
                    msg += "\n" + to_string(p.first) + "号玩家想要空刀";
                } else  {
                    msg += "\n" + to_string(p.first) + "号玩家想要刀" + to_string(p.second) + "号玩家";
                }
            }
            set.sendPrivateMessage(playerQQ, msg);
            return;
        }
        if (command.size() == 2) {
            try {
                int NO = stoi(command[1]);
                if (NO == 100) {
                    set.wolfs.at(seat) = NO; //设置狼刀
                    set.wolfTalk(to_string(seat) + "号玩家想要空刀");
                    set.wolfKillCheck();
                    return;
                }
                if (NO < 0 || NO >= set.seat.size()) {
                    set.sendPrivateMessage(playerQQ, "你选择的座位号超出范围");
                    return;
                }
                auto target = &set.players.at(set.seat[NO]);
                if ((*target)->state == PlayerState::Die) {
                    set.sendPrivateMessage(playerQQ, "该玩家已死亡，请重新选择目标");
                    return;
                }
                set.wolfs.at(seat) = NO; //设置狼刀
                set.wolfTalk(to_string(seat) + "号玩家想要刀" + to_string(NO) + "号玩家");
                set.wolfKillCheck();
                return;
            } catch (invalid_argument) {
            } catch (out_of_range) {
            }
        }

        //狼人讨论组
        msg = to_string(seat) + "号玩家说:";
        for (int i = 1; i < command.size(); i++) {
            msg += command[i] + " ";
        }
        set.wolfTalk(msg);
    }

    virtual void act2(vector<string> command, GameSet &set) {
        if (command.size() != 3) {
            return;
        }
        try {
            int NO = stoi(command[2]);
            if (NO >= 0 && NO < set.players.size()) {
                if (set.players[set.seat[NO]]->state == PlayerState::Die) {
                    send_group_message(set.group, "该玩家之前已经死亡，请重新选择另一名玩家");
                } else {
                    set.kill(seat);
                    set.kill(NO);
                }
            }
        } catch (invalid_argument) {
            return;
        } catch (out_of_range) {
            return;
        }
    }

    virtual void me(GameSet &set) {
        string msg = "你的身份是白狼王，请选择今晚的刀口，使用.[-n]刀座位号为n的人，如.1，\n";
        msg += ".100为空刀，\n";
        msg += "在消息前+.可以把信息传给狼队友，如.我倒钩。仅晚上可以使用，\n";
        msg += "使用.target查看当前狼团队的狼刀情况\n";
        msg += "狼团队:";
        for (auto wolf = set.wolfs.begin(); wolf != set.wolfs.end(); wolf++) {
            msg += to_string(wolf->first) + ",";
        }
        msg += "死亡后在公屏上使用.boom [-n]指定目标开枪，如.boom 1";
        set.sendPrivateMessage(playerQQ, msg);
    }

    WhiteWolf(int64_t QQ, int16_t seatNumber) {
        state = PlayerState::Ready;
        role = PlayerRole::WhiteWolf;
        playerQQ = QQ;
        seat = seatNumber;
    }
};

class Idiot : public Player {
public:
    virtual void act(vector<string> command, GameSet &set) {
        string failmsg = "验证失败，请检查验证码是否正确，使用.code查看验证码";
        if (set.state != SetState::Night || this->state != PlayerState::Action) {
            return;
        }
        if (command.size() > 2) {
            return;
        }
        try {
            int code = stoi(command[1]);
            if (code == set.code) {
                state = PlayerState::Wait;
                set.sendPrivateMessage(playerQQ, "验证成功，请等待白天开始");
            } else {
                set.sendPrivateMessage(playerQQ, failmsg);
            }
        } catch (invalid_argument) {
            set.sendPrivateMessage(playerQQ, failmsg);
        } catch (out_of_range) {
            set.sendPrivateMessage(playerQQ, failmsg);
        }
    }
    virtual void me(GameSet &set) {
        string msg = "你的身份是白痴，为减少场外，夜间需要输入验证码，使用.code查看今晚的验证码";
        set.sendPrivateMessage(playerQQ, msg);
    }


    Idiot(int64_t QQ, int16_t seatNumber) {
        state = PlayerState::Ready;
        role = PlayerRole::Idiot;
        playerQQ = QQ;
        seat = seatNumber;
    }
};