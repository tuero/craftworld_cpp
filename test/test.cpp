#include <craftworld/craftworld.h>

#include <iostream>

using namespace craftworld;
using namespace craftworld::util;

const std::unordered_map<std::string, int> ActionMap{
    {"w", 0}, {"d", 1}, {"s", 2}, {"a", 3}, {"e", 4},
};

void print_state(const CraftWorldGameState &state) {
    std::cout << state << std::endl;
    std::cout << state.get_hash() << std::endl;
    std::cout << "Pruned subgoals: ";
    for (auto const &sb : state.get_pruned_subgoals()) {
        std::cout << kElementToNameMap.at(static_cast<ElementType>(sb)) << " ";
    }
    std::cout << std::endl;
    std::cout << "Reward signal: " << state.get_reward_signal() << std::endl;
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