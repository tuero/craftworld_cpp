#include <cassert>
#include <sstream>
#include <string>
#include <vector>

#include "definitions.h"

namespace craftworld::util {

auto parse_board_str(const std::string &board_str) -> Board {
    std::stringstream board_ss(board_str);
    std::string segment;
    std::vector<std::string> seglist;
    // string split on |
    while (std::getline(board_ss, segment, '|')) {
        seglist.push_back(segment);
    }
    if (seglist.size() < 4) {
        throw std::invalid_argument("Board string should have at minimum 4 values separated by '|'.");
    }

    // Get general info
    const auto rows = static_cast<std::size_t>(std::stoi(seglist[0]));
    const auto cols = static_cast<std::size_t>(std::stoi(seglist[1]));
    const auto goal = static_cast<std::size_t>(std::stoi(seglist[2]));
    if (seglist.size() != static_cast<std::size_t>(rows * cols) + 3) {
        throw std::invalid_argument("Supplied rows/cols does not match input board length.");
    }
    if (goal < kPrimitiveStart || goal >= (kNumPrimitive + kNumRecipeTypes + kPrimitiveStart)) {
        throw std::invalid_argument("Unknown goal element.");
    }

    Board board(rows, cols, static_cast<Element>(goal));

    // Parse grid
    for (std::size_t i = 3; i < seglist.size(); ++i) {
        const int el_idx = std::stoi(seglist[i]);
        if (el_idx < 0 || el_idx > kNumElements) {
            throw std::invalid_argument(std::string("Unknown element type: ") + seglist[i]);
        }

        if (static_cast<Element>(el_idx) == Element::kAgent) {
            board.agent_idx = static_cast<std::size_t>(i) - 3;
        }

        board.item(i - 3) = static_cast<Element>(el_idx);
    }

    return board;
}

}    // namespace craftworld::util
