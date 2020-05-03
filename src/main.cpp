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
            privateControl(event);
        } catch (ApiError &err) {
        }
    });

    on_group_message([](const GroupMessageEvent &event) {
        try {
            groupControl(event);
        } catch (ApiError &) { // 忽略发送失败
        }
        event.block(); // 阻止当前事件传递到下一个插件
    });
}