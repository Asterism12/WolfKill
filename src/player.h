#pragma once
#include <iostream>

using namespace std;

enum class PlayerRole { Human, Wolf, Prophet, Witch, Hunter, Guard, WhiteWolf };
enum class PlayerState { Ready, Day, Action, Wait, Die, AFK };

class Player {
public:
    PlayerRole playerRole;
    PlayerState playerState;
    int64_t playerNo;

    Player(PlayerRole pr, int64_t pn) {
        playerRole = pr;
        playerNo = pn;
        playerState = PlayerState::Ready;
    }

    void action();
    void wait();
};