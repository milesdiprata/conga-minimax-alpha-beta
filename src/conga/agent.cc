#include <conga/agent.h>
#include <conga/board.h>

#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

namespace conga {

Agent::Agent(const Board::Player player)
    : player_(player),
      opponent_(player == Board::Player::k1 ? Board::Player::k2
                                            : Board::Player::k1) {
  if (player == Board::Player::kNone) {
    throw invalid_argument("Agent cannot be 'None' player!");
  }
}

Agent::~Agent() {}

const vector<Board::Point> Agent::OccupiedPoints(const Board& board) const {
  auto occupied_points = vector<Board::Point>();
  for (int x = 1; x <= Board::kBoardLength; ++x) {
    for (int y = 1; y <= Board::kBoardLength; ++y) {
      if (board.At(x, y).occupier == player_) {
        occupied_points.push_back(Board::Point(x, y));
      }
    }
  }

  return occupied_points;
}

const bool Agent::ValidMove(const Board& board, const Board::Point& point,
                            const Board::Move move) const {
  if (!board.HasPoint(point) || board.At(point).occupier == opponent_ ||
      board.At(point).num_stones <= 0) {
    return false;
  }

  auto next_point = point + Board::kMoves.at(move);
  if (!board.HasPoint(next_point) ||
      board.At(next_point).occupier == opponent_) {
    return false;
  }

  return true;
}

const vector<Board::Move> Agent::ValidMoves(const Board& board,
                                            const Board::Point& point) const {
  auto valid_moves = vector<Board::Move>();
  for (const auto& [possible_move, direction] : Board::kMoves) {
    if (ValidMove(board, point, possible_move)) {
      valid_moves.push_back(possible_move);
    }
  }

  return valid_moves;
}

const Board::Move Agent::BestMove(const Board& board,
                                  const Board::Point& from) const {
  return Board::Move::kNone;
}

const Board::Move Agent::RandomMove(const Board& board,
                                    const Board::Point& point) const {
  auto valid_moves = ValidMoves(board, point);
  if (valid_moves.empty()) {
    return Board::Move::kNone;
  }

  static random_device rd;
  static mt19937 gen(rd());

  auto distribution = uniform_int_distribution<>(
      0, distance(valid_moves.begin(), valid_moves.end()) - 1);
  auto it = valid_moves.begin();
  advance(it, distribution(gen));

  return *it;
}

void Agent::MakeMove(Board& board, const Board::Point& point,
                     const Board::Move move) const {
  if (!ValidMove(board, point, move)) {
    throw invalid_argument("Invalid move " + to_string(move) + " for " +
                           to_string(player_) + " from position " +
                           to_string(point) + "!");
  }

  for (int i = 1; i <= 3; ++i) {
    auto curr_point = point + (i * Board::kMoves.at(move));
    if (board.At(point).num_stones > 0 && board.HasPoint(curr_point) &&
        board.At(curr_point).occupier != opponent_) {
      board.At(curr_point).occupier = player_;
      auto next_point = point + ((i + 1) * Board::kMoves.at(move));
      if ((i < 3 && (!board.HasPoint(next_point) ||
                     board.At(next_point).occupier == opponent_)) ||
          board.At(point).num_stones < i || i == 3) {
        board.At(curr_point).num_stones += board.At(point).num_stones;
        board.At(point).num_stones = 0;
        board.At(point).occupier = Board::Player::kNone;
        break;
      } else {
        board.At(curr_point).num_stones += i;
        board.At(point).num_stones -= i;
      }
    }
  }
}

const bool Agent::Lost(const Board& board) const {
  auto occupied_points = OccupiedPoints(board);
  for (const auto& occupied_point : occupied_points) {
    auto valid_moves = ValidMoves(board, occupied_point);
    if (!valid_moves.empty()) {
      return true;
    }
  }

  return false;
}

template <typename T>
string to_string(const T& value) {
  stringstream sstream;
  sstream << value;
  return sstream.str();
}

}  // namespace conga