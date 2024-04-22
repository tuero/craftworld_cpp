#include <craftworld/craftworld.h>

using namespace craftworld;

void test_serialization() {
    const std::string board_str =
        "14|14|25|26|26|26|26|26|26|26|26|12|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|"
        "03|26|26|26|26|26|26|26|02|26|26|26|26|26|26|26|26|26|26|26|26|26|26|09|26|26|26|26|26|07|26|26|26|00|26|26|"
        "26|26|26|26|26|26|07|14|07|26|26|26|26|26|26|26|26|26|26|26|26|07|26|26|26|26|26|26|04|26|26|26|26|26|26|26|"
        "26|26|26|26|26|26|26|26|26|26|11|26|26|10|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|"
        "26|26|26|26|11|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|08|26|26|26|"
        "26|26|26|26|26|26|26|26|12|26|26|26|26|26|26|26|05|26|26";
    GameParameters params = kDefaultGameParams;
    params["game_board_str"] = GameParameter(board_str);

    CraftWorldGameState state(params);
    state.apply_action(Action(1));

    std::vector<uint8_t> bytes = state.serialize();

    CraftWorldGameState state_copy(bytes);

    state.apply_action(Action(2));
    state.apply_action(Action(2));
    state_copy.apply_action(Action(2));
    state_copy.apply_action(Action(2));

    if (state != state_copy) {
        std::cout << "serialization error." << std::endl;
    }
    std::cout << state << std::endl;
    std::cout << state.get_hash() << std::endl;

    if (state.get_hash() != state_copy.get_hash()) {
        std::cout << "serialization error." << std::endl;
    }
    std::cout << state_copy << std::endl;
    std::cout << state_copy.get_hash() << std::endl;
}

int main() {
    test_serialization();
}
