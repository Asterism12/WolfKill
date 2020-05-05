#include "control_new.h"
map<int64_t, GameSet> gamingGroups;
map<int64_t, bool> rdBeginZero;//true投骰子从0开始，false从1开始

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
            if (!gamingGroups[event.group_id].init(command, event)) {
                //创建失败，直接销毁
                destorySet(event.group_id);
            }
        }
    } else if (command[1] == "exit" && isGaming) {
        //移除一个对局
        gamingGroups.erase(event.group_id);
        string msg = "已强制退出游戏模式";
        send_group_message(event.group_id, msg);
    } else if (command[1] == "begin0") {
        rdBeginZero[event.group_id] = true;
        send_group_message(event.group_id, "当前rd命令投骰子点数从0开始");
    } else if (command[1] == "begin1") {
        rdBeginZero[event.group_id] = false;
        send_group_message(event.group_id, "当前rd命令投骰子点数从1开始");
    } else if (command[1] == "help") {
        string msg = "使用.rd [骰子数量] [上界] 命令投骰子\n";
        msg += "使用.begin0命令使骰子下界为0\n";
        msg += "使用.begin1命令使骰子下界为1\n";
        msg += "使用.start [板子代码]命令开始一局狼人杀游戏\n";
        msg += "当前有的板子类别及对应代码为：\n";
        msg += "2民2狼预守(6人) 代码：'6'\n";
        msg += "3民3狼预女猎(9人) 代码：'9'\n";
        msg += "4民4狼预女猎白痴(12人) 代码：'白痴'\n";
        msg += "4民3狼白狼王预女猎守(12人) 代码：'白狼王'";
        send_group_message(event.group_id, msg);
    } else if (command[1] == "rd") {
        int begin = 1;
        if (rdBeginZero.count(event.group_id) == 1 && rdBeginZero[event.group_id]) {
            begin = 0;
        }
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
                string err1 = "格式错误，rd，rd [上界]，rd [骰子数量] [上界]";
                send_group_message(event.group_id, err1);
                return;
            }
        } catch (invalid_argument) {
            string err1 = "格式错误，rd，rd [上界]，rd [骰子数量] [上界]";
            send_group_message(event.group_id, err1);
            return;
        } catch (out_of_range) {
            string err4 = "请输入一个较小的值";
            send_group_message(event.group_id, err4);
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
        std::uniform_int_distribution<int> dist(begin, d);
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
            gamingGroups[event.group_id].debug = true;
            string msg = "debug模式开启";
            send_group_message(event.group_id, msg);
        } else {
            string msg = "本群没有正在进行的游戏";
            send_group_message(event.group_id, msg);
        }
    } else if (isGaming) {
        gamingGroups[event.group_id].receiveGroupMessage(event.user_id, command);
    }
}

void privateControl(const PrivateMessageEvent &event) {
    vector<string> command = commandAnalyse(event.message);
    if (command[0] != ".") {
        return;
    }
    for (auto it = gamingGroups.begin(); it != gamingGroups.end(); it++) {
        if (it->second.players.count(event.user_id)) {
            it->second.receivePrivateMessage(event.user_id, command);
            break;
        }
    }
}