#ifndef CRAFTWORLD_UTIL_H_
#define CRAFTWORLD_UTIL_H_

#include <string>

#include "definitions.h"

namespace craftworld {
namespace util {

Board parse_board_str(const std::string &board_str);

}    // namespace util
}    // namespace craftworld

#endif    // CRAFTWORLD_UTIL_H_