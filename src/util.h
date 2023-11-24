#ifndef CRAFTWORLD_UTIL_H_
#define CRAFTWORLD_UTIL_H_

#include <string>

#include "definitions.h"

namespace craftworld::util {

auto parse_board_str(const std::string &board_str) -> Board;

}    // namespace craftworld::util

#endif    // CRAFTWORLD_UTIL_H_
