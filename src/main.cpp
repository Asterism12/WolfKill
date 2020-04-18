#include <iostream>
#include <set>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <string>

#include <cqcppsdk/cqcppsdk.h>

#include "set.h"

using namespace cq;
using namespace std;
using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;

map<int64_t, Set> gamingGroups;

void groupControl(const GroupMessageEvent &event) {
    bool isGaming = gamingGroups.count(event.group_id);
    if (event.message == ".start") {
        if (isGaming) {
            string msg = "本群已在游戏中！输入exit强制退出游戏模式";
            send_group_message(event.group_id, msg);
        } else {
            gamingGroups.insert(pair<int64_t, Set>(event.group_id, Set()));
            gamingGroups[event.group_id].init(event);
        }
    } else if (event.message == ".exit" && isGaming) {
        gamingGroups.erase(event.group_id);
        string msg = "已强制退出游戏模式";
        send_group_message(event.group_id, msg);
    } else if (event.message == ".rd") {
        srand(time(0));
        int res = rand() % 100;
        string msg = "rd:";
        msg += to_string(res);
        send_group_message(event.group_id, msg);
    } else if (event.message == ".debug") {
        if (isGaming) {
            if (gamingGroups[event.group_id].setState == SetState::Setting) {
                string msg = event.message;
                send_group_message(event.group_id, msg);
            }
        } else {
            string msg = "游戏模式x";
            send_group_message(event.group_id, msg);
        }
    } else if (event.message[0] == '.') {
        if (isGaming) {
            gamingGroups[event.group_id].control(event);
        }
    }
}

void privateControl(const PrivateMessageEvent &event) {
    for (auto it = gamingGroups.begin(); it != gamingGroups.end();it++) {
        if ((it->second.setState != SetState::Day) || (it->second.setState != SetState::Night)) {
            continue;
        }
        for (Player pl : it->second.players) {
            if (pl.playerNo == event.message_id) {
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
                default:
                    break;
                }
            }
        }
    }
}

CQ_INIT {
    on_enable([] { logging::info("启用", "插件已启用"); });

    on_private_message([](const PrivateMessageEvent &event) {
        try {
            privateControl(event);
        } catch (ApiError &err) {
            logging::warning("私聊", "私聊消息复读失败, 错误码: " + to_string(err.code));
        }
    });

    on_message([](const MessageEvent &event) {
        logging::debug("消息", "收到消息: " + event.message + "\n实际类型: " + typeid(event).name());
    });

    on_group_message([](const GroupMessageEvent &event) {
        try {
            groupControl(event);
        } catch (ApiError &) { // 忽略发送失败
        }
        if (event.is_anonymous()) {
            logging::info("群聊", "消息是匿名消息, 匿名昵称: " + event.anonymous.name);
        }
        event.block(); // 阻止当前事件传递到下一个插件
    });
}

CQ_MENU(menu_demo_1) {
    logging::info("菜单", "点击菜单1");
}

CQ_MENU(menu_demo_2) {
    send_private_message(10000, "测试");
}