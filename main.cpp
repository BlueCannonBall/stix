#include <iostream>
#include <sys/types.h>
#include "stix.hpp"

int main() {
    stix::Game game(stix::PLAYER_B);

    while (true) {
        std::cout << game.game_state[stix::PLAYER_B][stix::HAND_L] << ' ' << game.game_state[stix::PLAYER_B][stix::HAND_R] << std::endl;
        std::cout << game.game_state[stix::PLAYER_A][stix::HAND_L] << ' ' << game.game_state[stix::PLAYER_A][stix::HAND_R] << std::endl;

        std::cout << ">>";
        std::string command;
        std::cin >> command;

        stix::Move move;
        if (command == "attack") {
            std::string from_hand_str;
            std::cin >> from_hand_str;

            std::string to_hand_str;
            std::cin >> to_hand_str;

            if (from_hand_str[0] == 'r' || from_hand_str[0] == 'R') {
                move.from_hand = stix::HAND_R;
            } else if (from_hand_str[0] == 'l' || from_hand_str[0] == 'L') {
                move.from_hand = stix::HAND_L;
            }

            if (to_hand_str[0] == 'r' || to_hand_str[0] == 'R') {
                move.to_hand = stix::HAND_R;
            } else if (to_hand_str[0] == 'l' || to_hand_str[0] == 'L') {
                move.to_hand = stix::HAND_L;
            }
        } else if (command == "split") {

        }

        move.from_player = stix::PLAYER_A;
            move.to_player = stix::PLAYER_B;

            game.move(move);
            game.move(game.find_best_move(20));
    }
}