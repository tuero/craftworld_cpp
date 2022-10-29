#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "definitions.h"

namespace craftworld {
namespace util {

Board parse_board_str(const std::string &board_str) {
    std::stringstream board_ss(board_str);
    std::string segment;
    std::vector<std::string> seglist;
    // string split on |
    while (std::getline(board_ss, segment, '|')) {
        seglist.push_back(segment);
    }

    assert(seglist.size() >= 4);

    // Get general info
    int rows = std::stoi(seglist[0]);
    int cols = std::stoi(seglist[1]);
    int goal = std::stoi(seglist[2]);
    assert((int)seglist.size() == rows * cols + 3);
    assert(goal >= kPrimitiveStart && goal < kNumPrimitive + kNumRecipeTypes + kPrimitiveStart);
    Board board(rows, cols, goal);

    // Parse grid
    for (std::size_t i = 3; i < seglist.size(); ++i) {
        int el_idx = std::stoi(seglist[i]);
        if (el_idx < 0 || el_idx > kNumElementTypes) {
            std::cerr << "Unknown element type: " << el_idx << std::endl;
            exit(1);
        }

        if (static_cast<ElementType>(el_idx) == ElementType::kAgent) {
            board.agent_idx = i - 3;
        }

        board.item(i - 3) = static_cast<char>(el_idx);
    }

    return board;
}

}    // namespace util
}    // namespace craftworld
