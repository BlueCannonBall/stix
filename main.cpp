#include "stix.hpp"
#include <chrono>
#include <iostream>

void print_game(const stix::Game& game) {
    std::cout << "          L" << ' ' << 'R' << std::endl;
    std::cout << "Computer: " << +game.game_state[stix::PLAYER_B][stix::HAND_L] << ' ' << +game.game_state[stix::PLAYER_B][stix::HAND_R] << std::endl;
    std::cout << "Player:   " << +game.game_state[stix::PLAYER_A][stix::HAND_L] << ' ' << +game.game_state[stix::PLAYER_A][stix::HAND_R] << std::endl;
}

int main() {
    stix::Game game(stix::PLAYER_B);

    while ((game.game_state[stix::PLAYER_A][stix::HAND_R] + game.game_state[stix::PLAYER_A][stix::HAND_L] > 0) &&
           (game.game_state[stix::PLAYER_B][stix::HAND_R] + game.game_state[stix::PLAYER_B][stix::HAND_L] > 0)) {
        print_game(game);
        std::cout << "Your turn" << std::endl;

        std::cout << ">> " << std::flush;
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

            move.from_player = stix::PLAYER_A;
            move.to_player = stix::PLAYER_B;
        } else if (command == "split") {
            std::string from_hand_str;
            std::cin >> from_hand_str;

            std::string to_hand_str;
            std::cin >> to_hand_str;

            unsigned short amount;
            std::cin >> amount;
            move.amount = amount;

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

            move.from_player = stix::PLAYER_A;
            move.to_player = stix::PLAYER_A;
        }

        if (command != "forfeit" && command != "continue" && command != "pass") {
            std::vector<stix::Move> possible_moves;
            game.find_all_moves(std::back_inserter(possible_moves), stix::PLAYER_A);

            if (std::find(possible_moves.begin(), possible_moves.end(), move) == possible_moves.end()) {
                std::cout << "Invalid move" << std::endl;
                continue;
            } else {
                game.move(move);
                print_game(game);
            }
        }

        std::cout << "Computer's turn" << std::endl;
        stix::BestMove computer_move = game.find_best_move(std::chrono::seconds(2));
        game.move(computer_move);
        if (computer_move.from_player == computer_move.to_player) {
            std::cout << "<< split " << (computer_move.from_hand == stix::HAND_L ? 'l' : 'r') << ' ' << (computer_move.to_hand == stix::HAND_L ? 'l' : 'r') << ' ' << +computer_move.amount;
        } else {
            std::cout << "<< attack " << (computer_move.from_hand == stix::HAND_L ? 'l' : 'r') << ' ' << (computer_move.to_hand == stix::HAND_L ? 'l' : 'r');
        }
        std::cout << " # Found at " << computer_move.depth << " depth\n";
    }

    std::cout << "Game over!" << std::endl;
    return 0;
}