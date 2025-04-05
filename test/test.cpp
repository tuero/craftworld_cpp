#include <craftworld/craftworld.h>

#include <iomanip>
#include <iostream>

using namespace craftworld;

const std::unordered_map<std::string, Action> ActionMap{
    {"w", Action::kUp}, {"d", Action::kRight}, {"s", Action::kDown}, {"a", Action::kLeft}, {"e", Action::kUse},
};

void print_state(const CraftWorldGameState &state) {
    std::cout << state << std::endl;
    std::cout << state.get_hash() << std::endl;
    const auto obs = state.get_observation_alt();
    const auto [c, h, w] = state.observation_shape_alt();

    for (int _h = 0; _h < h; ++_h) {
        for (int _w = 0; _w < w; ++_w) {
            int idx = -1;
            int count = 0;
            for (int _c = 0; _c < c; ++_c) {
                if (obs[_c * (h * w) + _h * w + _w] == 1) {
                    idx = _c;
                    ++count;
                }
            }
            std::cout << std::setfill('0') << std::setw(2) << idx << " ";
        }
        std::cout << std::endl;
    }
}

void test_play() {
    std::string board_str;
    std::cout << "Enter board str: ";
    std::cin >> board_str;
    GameParameters params = kDefaultGameParams;
    params["game_board_str"] = GameParameter(board_str);

    CraftWorldGameState state(params);
    print_state(state);

    std::string action_str;
    while (!state.is_solution()) {
        std::cin >> action_str;
        if (ActionMap.find(action_str) != ActionMap.end()) {
            state.apply_action(ActionMap.at(action_str));
        }
        print_state(state);
    }
}

int main() {
    test_play();
}
