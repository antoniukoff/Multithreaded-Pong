#include "NetworkingManager.h"
#include <SDL_rect.h>

NetworkingManager::NetworkingManager() {}

NetworkingManager::~NetworkingManager() {}

bool NetworkingManager::parseClientPositionPacket(UDPpacket* packet, int& x, int& y, int& w, int& h) {
    if (packet && packet->data) {
        std::string packetData(reinterpret_cast<char*>(packet->data));

        // Find the positions of 'x:', 'y:', 'w:', 'h:' in the packet data
        size_t xPos = packetData.find("x:");
        size_t yPos = packetData.find("y:");
        size_t wPos = packetData.find("w:");
        size_t hPos = packetData.find("h:");

        // Check if 'x:', 'y:', 'r:', 'g:', and 'b:' are found
        if (xPos != std::string::npos && yPos != std::string::npos &&
            wPos != std::string::npos && hPos != std::string::npos) {
            // Extract the values of 'x', 'y', 'r', 'g', and 'b' using substrings
            std::string xSubstring = packetData.substr(xPos + 2, yPos - (xPos + 2));
            std::string ySubstring = packetData.substr(yPos + 2, wPos - (yPos + 2));
            std::string wSubstring = packetData.substr(wPos + 2, hPos - (wPos + 2));
            std::string hSubstring = packetData.substr(hPos + 2);

            // Convert the substrings to integers
            std::istringstream(xSubstring) >> x;
            std::istringstream(ySubstring) >> y;
            std::istringstream(wSubstring) >> w;
            std::istringstream(hSubstring) >> h;

            return true;
        }
        else {
            std::cout << "Invalid packet data: " << packetData << std::endl;
        }
    }

    return false;
}

bool NetworkingManager::parseBallServerPositionPacket(UDPpacket* packet, 
    int& serverX, int& serverY, int& serverW, int& serverH,
    int& ballX, int& ballY, int& ballW, int& ballH)
{
    if (packet && packet->data) {
        std::string packetData(reinterpret_cast<char*>(packet->data));

        // Find the positions of 'x:', 'y:', 'w:', 'h:' in the packet data
        size_t xPos = packetData.find("x:");
        size_t yPos = packetData.find("y:");
        size_t wPos = packetData.find("w:");
        size_t hPos = packetData.find("h:");
        size_t xPos2 = packetData.find("x:", xPos + 1);
        size_t yPos2 = packetData.find("y:", yPos + 1);
        size_t wPos2 = packetData.find("w:", wPos + 1);
        size_t hPos2 = packetData.find("h:", hPos + 1);

        if (xPos != std::string::npos && yPos != std::string::npos &&
            wPos != std::string::npos && hPos != std::string::npos &&
            xPos2 != std::string::npos && yPos2 != std::string::npos &&
            wPos2 != std::string::npos && hPos2 != std::string::npos) {

            std::string xServer = packetData.substr(xPos + 2, yPos - (xPos + 2));
            std::string yServer = packetData.substr(yPos + 2, wPos - (yPos + 2));
            std::string wServer = packetData.substr(wPos + 2, hPos - (wPos + 2));
            std::string hServer = packetData.substr(hPos + 2, xPos2 - (hPos + 2));
            std::string xBall = packetData.substr(xPos2 + 2, yPos2 - (xPos2 + 2));
            std::string yBall = packetData.substr(yPos2 + 2, wPos2 - (yPos2 + 2));
            std::string wBall = packetData.substr(wPos2 + 2, hPos2 - (wPos2 + 2));
            std::string hBall = packetData.substr(hPos2 + 2);

            std::istringstream(xServer) >> serverX;
            std::istringstream(yServer) >> serverY;
            std::istringstream(wServer) >> serverW;
            std::istringstream(hServer) >> serverH;
            std::istringstream(xBall) >> ballX;
            std::istringstream(yBall) >> ballY;
            std::istringstream(wBall) >> ballW;
            std::istringstream(hBall) >> ballH;

            return true;
        }
        else {
            std::cout << "Invalid packet data: " << packetData << std::endl;
        }
    }

    return false;
}

///=================================================================================================
///
///=======================================SERVER_NETWORK_HANDLING===================================


void NetworkingManager::receiveClientPosition(bool& quit, UDPsocket udpSocket, UDPpacket*& receivePacket, int& clientX, int& clientY, int& clientW, int& clientH) {
    while (!quit) {
		// Check for incoming UDP packets (data from the client)
        if (SDLNet_UDP_Recv(udpSocket, receivePacket)) {

            if (!parseClientPositionPacket(receivePacket, clientX, clientY, clientW, clientH)){
                quit = true;
            }
        }
	}
}

void NetworkingManager::sendServerBallPosition(bool& quit, UDPsocket udpSocket, IPaddress& clientIP, int& serverX, int& serverY, int& serverW, int& serverH,
    int& ballX, int& ballY, int& ballW, int& ballH) {
    while (!quit) {
		// Construct the message with the 'x', 'y' coordinates and 'r', 'g', 'b' color values of the square
		std::string msg_data = "x:" + std::to_string(serverX) + ",y:" + std::to_string(serverY)
			+ ",w:" + std::to_string(serverW) + ",h:" + std::to_string(serverH);

        std::string msg_data2 = "x:" + std::to_string(ballX) + ",y:" + std::to_string(ballY)
            + ",w:" + std::to_string(ballW) + ",h:" + std::to_string(ballH);

        std::string msg_data3 = msg_data + "," + msg_data2;


		// Allocate a larger packet to accommodate the color data
		UDPpacket* packet = SDLNet_AllocPacket(1024); // Adjust the size as needed
        if (packet) {
			packet->address = clientIP;
			packet->len = msg_data3.size() + 1; // +1 for the null terminator
			memcpy(packet->data, msg_data3.c_str(), packet->len);

            if (SDLNet_UDP_Send(udpSocket, -1, packet) == 0) {
				std::cerr << "SDLNet_UDP_Send error: " << SDLNet_GetError() << std::endl;
				quit = true;
			}

			SDLNet_FreePacket(packet);
		}

		SDL_Delay(16); // Cap the sending rate to approximately 60 FPS
	}

}


void NetworkingManager::sendScore(bool& quit, TCPsocket& client, std::string& score) {


    // Continuously listen for messages from the client
    while (!quit)
    {
        // Assuming 'score' holds the score you want to send
        // Check if the score is not null
        if (!score.empty() && client != nullptr) {
            // Get the length of the score string
            size_t len = strlen(score.c_str());

            printf("Sending score: %s\n", score.c_str());
            // Send the score to the client
            // Use len + 1 to include the null terminator in the message
            SDLNet_TCP_Send(client, score.c_str(), len + 1);

            if (strcmp("Game started", score.c_str()) == 0) {
                printf("Game started\n");
            }
        }
        else {
            //std::cout << "TCP client has not been established" << std::endl;
        }
    }
}

///==================================================================================================
///
/// =======================================CLIENT_NETWORK_HANDLING===================================


void NetworkingManager::sendClientPosition(bool& quit, UDPsocket udpSocket, IPaddress serverIP, int& clientX, int& clientY, int& clientW, int& clientH) {
    while (!quit) {
		// Construct the message with the 'x', 'y' coordinates and 'r', 'g', 'b' color values of the square
        std::string msg_data = "x:" + std::to_string(clientX) + ",y:" + std::to_string(clientY)
			+ ",w:" + std::to_string(clientW) + ",h:" + std::to_string(clientH);

		// Allocate a larger packet to accommodate the color data
		UDPpacket* packet = SDLNet_AllocPacket(1024); // Adjust the size as needed
        if (packet) {
			packet->address = serverIP;
			packet->len = msg_data.size() + 1; // +1 for the null terminator
			memcpy(packet->data, msg_data.c_str(), packet->len);

            if (SDLNet_UDP_Send(udpSocket, -1, packet) == 0) {
				std::cerr << "SDLNet_UDP_Send error: " << SDLNet_GetError() << std::endl;
				quit = true;
			}

			SDLNet_FreePacket(packet);
		}

		SDL_Delay(16); // Cap the sending rate to approximately 60 FPS
	}

}

void NetworkingManager::receiveServerBallPosition(bool& quit, UDPsocket udpSocket, UDPpacket* receivePacket, int& serverX, int& serverY, int& serverW, int& serverH,
    int& ballX, int& ballY, int& ballW, int& ballH) {
    while (!quit) {
        // Check for incoming UDP packets (data from the server)
        if (SDLNet_UDP_Recv(udpSocket, receivePacket)) {

            if (!parseBallServerPositionPacket(receivePacket, serverX, serverY, serverW, serverH, ballX, ballY, ballW, ballH)) {
                quit = true;
            }
        }
    }
}

void NetworkingManager::receiveScore(bool& quit, TCPsocket& server, bool& playerConnected, std::string& score) {
    char buffer[1024];

    while (!quit) {
        //Receive DATA
        //####################################################################
        //####################################################################
        int received = SDLNet_TCP_Recv(server, buffer, sizeof(buffer));
        if (received > 0) {

            buffer[received - 1] = '\0';  // Null-terminate the received data
            if (buffer) {
                score = buffer;
               // printf("%s\n", buffer);
            }
            if (strcmp("Game started", buffer) == 0) {
                playerConnected = true;
			}
            if (strcmp("Game ended", buffer) == 0) {
				playerConnected = false;
			}
        }
        //####################################################################
        //####################################################################
        
	}
}