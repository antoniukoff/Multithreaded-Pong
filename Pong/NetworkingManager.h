#pragma once
#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

class NetworkingManager
{
public:
    NetworkingManager();
    ~NetworkingManager();
    static bool parseClientPositionPacket(UDPpacket* packet, int& x, int& y, int& w, int& h);
    static bool parseBallServerPositionPacket(UDPpacket* packet, int& serverX, int& serverY, int& serverW, int& serverH, int& ballX, int& ballY, int& ballW, int& ballH);

    static void sendClientPosition(bool& quit, UDPsocket udpSocket, IPaddress serverIP, int& clientX, int& clientY, int& clientW, int& clientH);
    static void receiveServerBallPosition(bool& quit, UDPsocket udpSocket, UDPpacket* receivePacket, int& serverX, int& serverY, int& serverW, int& serverH, int& ballX, int& ballY, int& ballW, int& ballH);
    static void receiveScore(bool& quit, TCPsocket& server, bool& playerConnected, std::string& score);
    static void receiveClientPosition(bool& quit, UDPsocket udpSocket, UDPpacket*& receivePacket, int& clientX, int& clientY, int& clientW, int& clientH);
    static void sendServerBallPosition(bool& quit, UDPsocket udpSocket, IPaddress& clientIP, int& serverX, int& serverY, int& serverW, int& serverH, int& ballX, int& ballY, int& ballW, int& ballH);
    static void sendScore(bool& quit, TCPsocket& client, std::string& score);
};

