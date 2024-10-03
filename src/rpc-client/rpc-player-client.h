#ifndef RPC_PLAYER_CLIENT_H
#define RPC_PLAYER_CLIENT_H

#include <rcsc/player/player_agent.h>


class RpcPlayerClient {
public:
    bool doPreprocess(rcsc::PlayerAgent* agent);
    bool checkPreprocess(rcsc::PlayerAgent* agent);
    bool checkdoForceKick(rcsc::PlayerAgent* agent);
    bool checkdoHeardPassReceive(rcsc::PlayerAgent* agent);
    bool doForceKick(rcsc::PlayerAgent * agent);
    bool doHeardPassReceive(rcsc::PlayerAgent * agent);
};

#endif