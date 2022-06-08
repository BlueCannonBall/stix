#include "threadpool.hpp"
#include <algorithm>
#include <atomic>
#include <boost/container/static_vector.hpp>
#include <iterator>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <thread>
#include <utility>

namespace stix {
    namespace detail {
        tp::ThreadPool pool; // NOLINT
    }

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

        inline bool operator==(Player player) const {
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

        inline bool operator==(const Move& move) const {
            return from_player == move.from_player &&
                   from_hand == move.from_hand &&
                   to_player == move.to_player &&
                   to_hand == move.to_hand &&
                   amount == move.amount;
        }

        inline bool operator!=(const Move& move) const {
            return from_player != move.from_player ||
                   from_hand != move.from_hand ||
                   to_player != move.to_player ||
                   to_hand != move.to_hand ||
                   amount != move.amount;
        }
    };

    class BestMove: public Move {
    public:
        unsigned int depth = 0;

        BestMove() = default;
        BestMove(const Move& move, unsigned int depth = 0) :
            Move(move),
            depth(depth) { }
    };

    typedef Player GameState[2];

    class Game {
    public:
        GameState game_state = {{1, 1}, {1, 1}};
        PlayerID player_id;

        Game(PlayerID player_id) :
            player_id(player_id) { }

        inline void move(Move move) {
            if (move.from_player != move.to_player) {
                game_state[move.to_player][move.to_hand] = (game_state[move.to_player][move.to_hand] + game_state[move.from_player][move.from_hand]) % 5;
            } else {
                game_state[move.to_player][move.to_hand] += move.amount;
                game_state[move.from_player][move.from_hand] -= move.amount;
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

            for (;;) {
                new_player[HAND_L]--;
                new_player[HAND_R]++;
                if (new_player[HAND_L] >= 0 && new_player[HAND_R] < 5) {
                    if (new_player != game_state[player_id]) {
                        ret = Move(player_id, HAND_L, player_id, HAND_R, new_player[HAND_R] - game_state[player_id][HAND_R]);
                    }
                } else {
                    break;
                }
            }

            new_player = game_state[player_id];
            for (;;) {
                new_player[HAND_R]--;
                new_player[HAND_L]++;
                if (new_player[HAND_R] >= 0 && new_player[HAND_L] < 5) {
                    if (new_player != game_state[player_id]) {
                        ret = Move(player_id, HAND_R, player_id, HAND_L, new_player[HAND_L] - game_state[player_id][HAND_L]);
                    }
                } else {
                    break;
                }
            }
        }

        inline int evaluate(PlayerID player_id = PLAYER_NONE) {
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

        int maxi(Move move, int alpha, int beta, unsigned int depth, const std::atomic<bool>& stop_flag, PlayerID player_id = PLAYER_NONE) {
            if (player_id == PLAYER_NONE) {
                player_id = this->player_id;
            }

            GameState old_game_state;
            memcpy(old_game_state, game_state, sizeof(GameState));
            this->move(move);

            if (depth == 0) {
                int evaluation = evaluate(player_id);
                memcpy(game_state, old_game_state, sizeof(GameState));
                return evaluation;
            }

            boost::container::static_vector<Move, 32> moves;
            find_all_moves(std::back_inserter(moves), player_id);

            int ret;
            if (moves.size() == 0) {
                ret = -1;
                goto cutoff;
            } else {
                for (Move move : moves) {
                    int score = mini(move, alpha, beta, depth - 1, stop_flag, player_id);
                    if (stop_flag) {
                        memcpy(game_state, old_game_state, sizeof(GameState));
                        return alpha;
                    } else if (score >= beta) {
                        ret = beta;
                        goto cutoff;
                    } else if (score > alpha) {
                        alpha = score;
                    }
                }
                ret = alpha;
            }

        cutoff:
            memcpy(game_state, old_game_state, sizeof(GameState));
            return ret;
        }

        int mini(Move move, int alpha, int beta, unsigned int depth, const std::atomic<bool>& stop_flag, PlayerID player_id = PLAYER_NONE) {
            if (player_id == PLAYER_NONE) {
                player_id = this->player_id;
            }

            GameState old_game_state;
            memcpy(old_game_state, game_state, sizeof(GameState));
            this->move(move);

            if (depth == 0) {
                int evaluation = evaluate(player_id);
                memcpy(game_state, old_game_state, sizeof(GameState));
                return evaluation;
            }

            boost::container::static_vector<Move, 32> moves;
            find_all_moves(std::back_inserter(moves), opposite_player(player_id));

            int ret;
            if (moves.size() == 0) {
                ret = 1;
                goto cutoff;
            } else {
                for (Move move : moves) {
                    int score = maxi(move, alpha, beta, depth - 1, stop_flag, player_id);
                    if (stop_flag) {
                        memcpy(game_state, old_game_state, sizeof(GameState));
                        return beta;
                    } else if (score <= alpha) {
                        ret = alpha;
                        goto cutoff;
                    } else if (score < beta) {
                        beta = score;
                    }
                }
                ret = beta;
            }

        cutoff:
            memcpy(game_state, old_game_state, sizeof(GameState));
            return ret;
        }

        template <typename DurationT>
        BestMove find_best_move(DurationT search_time, unsigned int starting_depth = 10, PlayerID player_id = PLAYER_NONE) {
            if (player_id == PLAYER_NONE) {
                player_id = this->player_id;
            }

            boost::container::static_vector<Move, 32> moves;
            find_all_moves(std::back_inserter(moves), player_id);

            BestMove ret;
            std::atomic<bool> stop(false);

            std::thread deepening_thread([this, starting_depth, &moves, player_id, &stop, &ret]() {
                for (unsigned int depth = starting_depth; !stop; depth++) {
                    Move best_move;
                    int best_move_score = INT_MIN;
                    std::mutex mtx;

                    std::vector<std::shared_ptr<tp::Task>> tasks;
                    for (auto& move : moves) {
                        tasks.push_back(detail::pool.schedule(
                            [move, depth, &stop, player_id, &mtx, &best_move, &best_move_score](void* data) {
                                Game* game = (Game*) data;
                                Game copy = *game;

                                int score = copy.mini(move, INT_MIN, INT_MAX, depth - 1, stop, player_id);

                                if (!stop) {
                                    mtx.lock();
                                    if (score > best_move_score) {
                                        best_move = move;
                                        best_move_score = score;
                                    }
                                    mtx.unlock();
                                }
                            },
                            this));
                    }

                    for (const auto& task : tasks) {
                        task->await();
                    }

                    if (!stop) {
                        ret = BestMove(best_move, depth);
                    }
                }
            });

            std::this_thread::sleep_for(search_time);
            while (ret.depth == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
            stop = true;
            deepening_thread.join();

            return ret;
        }
    };
} // namespace stix