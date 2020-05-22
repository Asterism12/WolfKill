#include <cstdlib>
#include <ctime>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include <cqcppsdk/cqcppsdk.h>

//#include "control.h"
#include "control_new.h"

using namespace cq;
using namespace std;
using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;

CQ_INIT {
    on_enable([] { logging::info("启用", "插件已启用"); });

    on_private_message([](const PrivateMessageEvent &event) {
        try {
            bool block = privateControl(event);
            if (block) {
                event.block();
            }
        } catch (ApiError &) {
        }
    });

    on_group_message([](const GroupMessageEvent &event) {
        try {
            bool block = groupControl(event);
            if (block) {
                event.block();
            }
        } catch (ApiError &) { // 忽略发送失败
        }
    });
}

CQ_MENU(menu_dice_on) {
    diceTurn();
}