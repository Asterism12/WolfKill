#pragma once
#include <string>
#include <map>
#include <set>
#include <random>

#include <cqcppsdk/cqcppsdk.h>

using namespace cq;
using namespace std;

enum class SetState { Uninitialized, Setting, Waiting, Day, Morning, Night, Analysing };

enum class PlayerRole;
enum class PlayerState;
class Player;

class GameSet {
public:
    std::random_device rd; //种子
    int64_t group; //群号
    int64_t host; //主机玩家
    SetState state;
    int16_t code; //闭眼玩家的验证码，用于减少场外因素
    vector<int64_t> seat; //玩家列表，数组下标对应座位号
    map<int64_t, Player*> players;
    vector<PlayerRole> rolePool;
    int16_t date;
    map<int64_t, set<string>> deathNote; //每晚玩家与死亡有关的行动列表，<死亡玩家，行动列表>
    map<int16_t, int16_t> wolfs; //狼队座位号，狼刀目标，-1为初始值，100为空刀
    int16_t humanNum, wolfNum, godNum;
    int16_t botNum; //仅用于开局检测

    bool debug; //是否开启debug模式

    void init(vector<string> command, GroupMessageEvent event);
    void c5();
    void c9();
    void cIdiot();
    void cWhite();
    void addPlayer(int64_t player);
    void go();
    void night();
    void morning();
    void daybreakCheck();
    void day();
    void handleDeathEvent();
    void seatShow();
    void setPlayersState(PlayerState state, PlayerState lastState);
    void wolfTalk(string s);
    void wolfKillCheck();
    void kill(int16_t seatNumber);
    void winCheck();
    void analyse();
    void roleMessage();
    void generateCode();

    void sendPrivateMessage(int64_t QQ, string msg);
    void receivePrivateMessage(int64_t QQ, vector<string> command);
    void receiveGroupMessage(int64_t QQ, vector<string> command);

    GameSet(){};
};