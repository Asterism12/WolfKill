#include "control.h"
map<int64_t, Set> gamingGroups;

void groupControl(const GroupMessageEvent &event) {
    bool isGaming = gamingGroups.count(event.group_id);
    vector<string> command = commandAnalyse(event.message);
    if (command[0] != ".") {
        return;
    }
    if (command[1] == "start") {
        if (isGaming) {
            string msg = "本群已在游戏中！输入exit强制退出游戏模式";
            send_group_message(event.group_id, msg);
        } else {
            //添加一个对局
            gamingGroups.insert(pair<int64_t, Set>(event.group_id, Set()));
            gamingGroups[event.group_id].init(event);
        }
    } else if (command[1] == "exit" && isGaming) {
        //移除一个对局
        gamingGroups.erase(event.group_id);

        string msg = "已强制退出游戏模式";
        send_group_message(event.group_id, msg);
    } else if (command[1] == "rd") {
        int r = 1;
        int d = 100;
        try {
            switch (command.size()) {
            case 2:
                break;
            case 3:
                d = stoi(command[2]);
                break;
            case 4:
                r = stoi(command[2]);
                d = stoi(command[3]);
                break;
            default:
                string err1 = "格式错误，rd，rd [-range]，rd [-dicenumber] [-range]";
                send_group_message(event.group_id, err1);
                return;
            }
        } catch (invalid_argument) {
            string err1 = "格式错误，rd，rd [-range]，rd [-dicenumber] [-range]";
            send_group_message(event.group_id, err1);
            return;
        } catch (out_of_range) {
            string err1 = "请输入一个较小的值";
            send_group_message(event.group_id, err1);
            return;
        }
        if (r <= 0 || d <= 0) {
            string err2 = "请输入一个正值";
            send_group_message(event.group_id, err2);
            return;
        }
        if (r > 20 || d > 1000000) {
            string err3 = "请输入一个较小的值";
            send_group_message(event.group_id, err3);
            return;
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, d);
        if (r == 1) {
            int res = dist(gen);
            string msg = "rd:";
            msg += to_string(res);
            send_group_message(event.group_id, msg);
        } else {
            int sum = 0;
            string msg = "rd:\n";
            for (int i = 0; i < r; i++) {
                int res = dist(gen);
                sum += res;
                msg += to_string(res) + ',';
            }
            msg += "\ntotal:" + to_string(sum);
            send_group_message(event.group_id, msg);
        }
    } else if (command[1] == "debug") {
        if (isGaming) {
            if (gamingGroups[event.group_id].setState == SetState::Setting) {
                string msg = event.message;
                send_group_message(event.group_id, msg);
            }
        } else {
            string msg = "游戏模式x";
            send_group_message(event.group_id, msg);
        }
    } else if (isGaming) {
        gamingGroups[event.group_id].control(event);
    }
}

void privateControl(const PrivateMessageEvent &event) {
    vector<string> command = commandAnalyse(event.message);
    if (command[0] != ".") {
        return;
    }
    for (auto it = gamingGroups.begin(); it != gamingGroups.end(); it++) {
        if (it->second.setState != SetState::Night) {
            continue;
        }
        for (Player pl : it->second.players) {
            if (pl.playerNo == event.user_id) {
                if (pl.playerState != PlayerState::Action) {
                    return;
                }
                string msg;
                if (command[0] == "code") {
                    msg = to_string(it->second.code);
                    send_private_message(event.user_id, msg);
                }
                if (command[0] == "mine") {
                    switch (pl.playerRole) {
                    case PlayerRole::Human:
                        msg = "你的身份是平民，为减少场外，夜间狼人行动会向你发送验证码";
                        send_private_message(event.user_id, msg);
                        break;
                    case PlayerRole::Hunter:
                        msg = "你的身份是猎人，女巫如果放毒会通知你，\n";
                        msg += "为减少场外，夜间狼人行动会向你发送验证码，\n";
                        msg += "死亡后使用.+n指定目标开枪，如.1";
                        send_private_message(event.user_id, msg);
                        break;
                    case PlayerRole::Prophet:
                        msg = "你的身份是预言家，.+n选择今晚的验人，如.1\n";
                        send_private_message(event.user_id, msg);
                        break;
                    case PlayerRole::Witch:
                        msg = "你的身份是女巫，等待狼人行动，\n";
                        msg += ".+n向指定目标使用毒药，如.1，如果你还有解药，狼人刀人后会通知你，\n";
                        msg += "输入.nop放弃本晚行动";
                        send_private_message(event.user_id, msg);
                        break;
                    case PlayerRole::Wolf:
                        msg = "你的身份是狼人，请选择今晚的刀口，使用.+n刀座位号为n的人，如.1，\n";
                        msg += ".66为空刀，\n";
                        msg += "在消息前+.可以把信息传给狼队友，如.我倒钩。仅限晚上生效，\n";
                        msg += "狼团队:";
                        for (auto wolf : it->second.wolfs) {
                            msg += to_string(wolf.first) + ".";
                        }
                        send_private_message(event.user_id, msg);
                        break;
                    case PlayerRole::Idiot:
                        msg = "你的身份是白痴，为减少场外，夜间狼人行动会向你发送验证码，\n";
                        send_private_message(event.user_id, msg);
                        break;
                    default:
                        break;
                    }
                    return;
                }

                switch (pl.playerRole) {
                case PlayerRole::Human:
                    it->second.humanAct(event);
                    break;
                case PlayerRole::Hunter:
                    it->second.hunterAct(event);
                    break;
                case PlayerRole::Prophet:
                    it->second.prophetAct(event);
                    break;
                case PlayerRole::Witch:
                    it->second.witchAct(event);
                    break;
                case PlayerRole::Wolf:
                    it->second.wolfAct(event);
                    break;
                case PlayerRole::Idiot:
                    it->second.idiotAct(event);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

vector<string> commandAnalyse(string message) {
    vector<string> res = {"."};
    vector<string> empty = {"!"};

    if (message.size() <= 1 || message[0] != '.') {
        return empty;
    }

    int start = 1;
    bool inString = true;
    for (int i = 1; i <= message.size(); i++) {
        if (inString) {
            if (message[i] == ' ' || message[i] == '\n' || message[i] == '\0') {
                if (start == i) {
                    return empty;
                } else {
                    inString = false;
                    res.push_back(message.substr(start, i - start));
                }
            }
        } else {
            if (message[i] == ' ' || message[i] == '\n' || message[i] == '\0') {
                continue;
            } else {
                inString = true;
                start = i;
            }
        }
    }

    return res;
}

void destorySet(int64_t set) {
    gamingGroups.erase(set);
}