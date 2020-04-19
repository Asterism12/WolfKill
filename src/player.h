#pragma once
#include <iostream>

using namespace std;

enum class PlayerRole { Human, Wolf, Prophet, Witch, Hunter, Guard, WhiteWolf, Idiot };
enum class PlayerState { Ready, Day, Action, Wait, Die, AFK };

class Player {
public:
    PlayerRole playerRole;
    PlayerState playerState;
    int64_t playerNo;
    bool hasPoison;
    bool hasAntidote;
    bool enableShoot;

    Player(PlayerRole pr, int64_t pn) {
        playerRole = pr;
        playerNo = pn;
        playerState = PlayerState::Ready;
        hasAntidote = true;
        hasPoison = true;
        enableShoot = true;
    }

    void action();
    void wait();
};