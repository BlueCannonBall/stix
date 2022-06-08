#include <algorithm>
#include <iterator>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <utility>
#include <vector>

namespace stix {
    typedef int8_t Hand;

    enum PlayerID : int8_t {
        PLAYER_NONE = -1,
        PLAYER_A = 0,
        PLAYER_B = 1
    };

    enum HandID : uint8_t {
        HAND_L = 0,
        HAND_R = 1
    };

    inline PlayerID opposite_player(PlayerID player_id) {
        return (PlayerID) !((int8_t) player_id);
    }

    class Player {
    public:
        Hand hands[2];

        inline Hand& operator[](HandID hand) {
            return hands[hand];
        }

        inline const Hand& operator[](HandID hand) const {
            return hands[hand];
        }

        inline Hand& operator[](uint8_t hand) {
            return hands[hand];
        }

        inline const Hand& operator[](uint8_t hand) const {
            return hands[hand];
        }

        bool operator==(Player player) const {
            return ((this->hands[HAND_L] == player.hands[HAND_L]) && (this->hands[HAND_R] == player.hands[HAND_R])) ||
                   ((this->hands[HAND_L] == player.hands[HAND_R]) && (this->hands[HAND_R] == player.hands[HAND_L]));
        }

        inline bool operator!=(Player player) const {
            return !(*this == player);
        }
    };

    class Move {
    public:
        PlayerID from_player;
        HandID from_hand;

        PlayerID to_player;
        HandID to_hand;

        uint8_t amount = 1;

        Move() = default;
        Move(PlayerID from_player, HandID from_hand, PlayerID to_player, HandID to_hand) :
            from_player(from_player),
            from_hand(from_hand),
            to_player(to_player),
            to_hand(to_hand) { }
        Move(PlayerID from_player, uint8_t from_hand, PlayerID to_player, uint8_t to_hand) :
            from_player(from_player),
            from_hand((HandID) from_hand),
            to_player(to_player),
            to_hand((HandID) to_hand) { }

        Move(PlayerID from_player, HandID from_hand, PlayerID to_player, HandID to_hand, uint8_t amount) :
            from_player(from_player),
            from_hand(from_hand),
            to_player(to_player),
            to_hand(to_hand),
            amount(amount) { }
        Move(PlayerID from_player, uint8_t from_hand, PlayerID to_player, uint8_t to_hand, uint8_t amount) :
            from_player(from_player),
            from_hand((HandID) from_hand),
            to_player(to_player),
            to_hand((HandID) to_hand),
            amount(amount) { }
    };

    typedef Player GameState[2];

    class Game {
    public:
        GameState game_state = {{1, 1}, {1, 1}};
        PlayerID player_id;

        Game(PlayerID player_id) :
            player_id(player_id) { }

        void move(Move move) {
            if (move.from_player != move.to_player) {
                game_state[move.to_player][move.to_hand] = (game_state[move.to_player][move.to_hand] + game_state[move.from_player][move.from_hand]) % 5;
            } else {
                game_state[move.to_player][move.to_hand] += move.amount;
                game_state[move.from_player][move.from_hand] -= move.amount;
            }
        }

        int evaluate(PlayerID player_id = PLAYER_NONE) {
            if (player_id == PLAYER_NONE) {
                player_id = this->player_id;
            }

            if (game_state[PLAYER_A][HAND_L] + game_state[PLAYER_A][HAND_R] <= 0) {
                return player_id == PLAYER_A ? -1 : 1;
            } else if (game_state[PLAYER_B][HAND_L] + game_state[PLAYER_B][HAND_R] <= 0) {
                return player_id == PLAYER_B ? -1 : 1;
            } else {
                return 0;
            }
        }

        template <typename InsertIt>
        void find_all_moves(InsertIt ret, PlayerID player_id = PLAYER_NONE) {
            if (player_id == PLAYER_NONE) {
                player_id = this->player_id;
            }

            // Attacks
            for (uint8_t i = 0; i < 2; i++) {
                if (game_state[player_id][i] != 0) {
                    for (uint8_t j = 0; j < 2; j++) {
                        if (game_state[opposite_player(player_id)][j] != 0) {
                            ret = Move(player_id, i, opposite_player(player_id), j);
                        }
                    }
                }
            }

            // Splits
            Player new_player = game_state[player_id];
            std::vector<Player> tested_positions;

            for (;;) {
                new_player[HAND_L]--;
                new_player[HAND_R]++;
                if (new_player[HAND_L] >= 0 && new_player[HAND_R] < 5) {
                    if (std::find(tested_positions.begin(), tested_positions.end(), new_player) == tested_positions.end()) {
                        ret = Move(player_id, HAND_L, player_id, HAND_R, new_player[HAND_R] - game_state[player_id][HAND_R]);
                        tested_positions.push_back(new_player);
                    }
                } else {
                    break;
                }
            }

            for (;;) {
                new_player[HAND_R]--;
                new_player[HAND_L]++;
                if (new_player[HAND_R] >= 0 && new_player[HAND_L] < 5) {
                    if (std::find(tested_positions.begin(), tested_positions.end(), new_player) == tested_positions.end()) {
                        ret = Move(player_id, HAND_R, player_id, HAND_L, new_player[HAND_L] - game_state[player_id][HAND_L]);
                        tested_positions.push_back(new_player);
                    }
                } else {
                    break;
                }
            }
        }

        int mini(Move move, int alpha, int beta, unsigned int depth, PlayerID player_id = PLAYER_NONE) {
            if (player_id == PLAYER_NONE) {
                player_id = this->player_id;
            }

            GameState old_game_state;
            memcpy(old_game_state, game_state, sizeof(GameState));
            this->move(move);

            if (depth == 0) {
                int ret = evaluate(player_id);
                memcpy(old_game_state, game_state, sizeof(GameState));
                return ret;
            }

            std::vector<Move> moves;
            find_all_moves(std::back_inserter(moves), opposite_player(player_id));
            if (moves.size() == 0) {
                int ret = 1;
                memcpy(old_game_state, game_state, sizeof(GameState));
                return ret;
            }

            int min = INT_MAX;
            for (Move move : moves) {
                int score = maxi(move, alpha, beta, depth - 1, player_id);
                if (score < min) {
                    min = score;
                }
            }

            memcpy(game_state, old_game_state, sizeof(GameState));
            return min;
        }

        int maxi(Move move, int alpha, int beta, unsigned int depth, PlayerID player_id = PLAYER_NONE) {
            if (player_id == PLAYER_NONE) {
                player_id = this->player_id;
            }

            GameState old_game_state;
            memcpy(old_game_state, game_state, sizeof(GameState));
            this->move(move);

            if (depth == 0) {
                int ret = evaluate(player_id);
                memcpy(old_game_state, game_state, sizeof(GameState));
                return ret;
            }

            std::vector<Move> moves;
            find_all_moves(std::back_inserter(moves), player_id);
            if (moves.size() == 0) {
                int ret = -1;
                memcpy(old_game_state, game_state, sizeof(GameState));
                return ret;
            }

            int max = INT_MIN;
            for (Move move : moves) {
                int score = mini(move, alpha, beta, depth - 1, player_id);
                if (score > max) {
                    max = score;
                }
            }

            memcpy(game_state, old_game_state, sizeof(GameState));
            return max;
        }

        Move find_best_move(unsigned int depth, PlayerID player_id = PLAYER_NONE) {
            if (player_id == PLAYER_NONE) {
                player_id = this->player_id;
            }

            std::vector<Move> moves;
            find_all_moves(std::back_inserter(moves), player_id);

            Move best_move;
            int best_move_score = INT_MIN;

            for (Move move : moves) {
                int score = mini(move, INT_MIN, INT_MAX, depth - 1, player_id);
                if (score > best_move_score) {
                    best_move = move;
                    best_move_score = score;
                }
            }

            return best_move;
        }
    };
} // namespace stix