#include "set_new.h"
void GameSet::init(vector<string> command, GroupMessageEvent event) {
    state = SetState::Setting;
    group = event.group_id;
    if (command[2] == "9") {
        c9();
    } else if (command[2] == "白痴") {
        cIdiot();
    } else if (command[2] == "白狼王") {
        cWhite();
    } else {
        
    }
}