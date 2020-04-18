#pragma once
enum class PlayerRole { Human, Wolf, Prophet, Witch, Hunter };
enum class PlayerState { Ready, Day, Action, Wait, Die, AFK };
class Player {
public:
    PlayerRole PlayerRole;
    PlayerState PlayerState;

    virtual void action();
    virtual void wait();
};
class Human : public Player {

};