#ifndef CRAFTWORLD_BASE_H_
#define CRAFTWORLD_BASE_H_

#include <array>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <variant>

#include "definitions.h"

namespace craftworld {

// Image properties
constexpr int SPRITE_WIDTH = 32;
constexpr int SPRITE_HEIGHT = 32;
constexpr int SPRITE_CHANNELS = 3;
constexpr int SPRITE_DATA_LEN_PER_ROW = SPRITE_WIDTH * SPRITE_CHANNELS;
constexpr int SPRITE_DATA_LEN = SPRITE_WIDTH * SPRITE_HEIGHT * SPRITE_CHANNELS;

// Game parameter can be boolean, integral or floating point
using GameParameter = std::variant<bool, int, float, std::string>;
using GameParameters = std::unordered_map<std::string, GameParameter>;

// Default game parameters
static const GameParameters kDefaultGameParams{
    {"game_board_str",
     GameParameter(std::string(
         "14|14|25|26|26|26|26|08|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|12|26|07|26|26|26|"
         "26|26|26|26|26|26|26|26|26|07|14|07|26|26|26|26|12|26|26|26|26|26|26|26|07|26|26|26|26|26|26|26|11|26|26|26|"
         "26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|04|26|26|26|26|26|26|26|26|26|02|26|26|26|26|26|26|26|26|26|26|"
         "26|26|26|26|26|26|26|26|26|26|26|26|10|26|26|26|26|26|11|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|"
         "26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|00|26|26|26|05|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|"
         "26|26|26|26|26|03|26|26|26|09|26|26|26|26|26|26|26|26|26"))},    // Game board string
    {"workshop_swap", GameParameter(false)},                               // Game board string
};

// Shared global state information relevant to all states for the given game
struct SharedStateInfo {
    SharedStateInfo() = default;
    SharedStateInfo(GameParameters params)
        : game_board_str(std::get<std::string>(params.at("game_board_str"))),
          workshop_swap(std::get<bool>(params.at("workshop_swap"))) {}
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::string game_board_str;                                   // String representation of the starting state
    std::unordered_map<std::size_t, uint64_t> zrbht_world;        // Zobrist hashing table
    std::unordered_map<std::size_t, uint64_t> zrbht_inventory;    // Zobrist hashing table for inventory
    std::vector<std::size_t> neighbours;                          // Reusable buffer for finding neighbours
    std::size_t MAX_INV_HASH_ITEMS = 20;                          // NOLINT
    bool workshop_swap = false;                                   // NOLINT
    // NOLINTEND(misc-non-private-member-variables-in-classes)
    NOP_STRUCTURE(SharedStateInfo, game_board_str, MAX_INV_HASH_ITEMS, workshop_swap);
};

// Information specific for the current game state
struct LocalState {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    LocalState() = default;
    uint8_t current_reward = 0;                            // Reward for the current game state
    uint64_t reward_signal = 0;                            // Signal for external information about events
    std::unordered_map<Element, std::size_t> inventory;    // Inventory of items
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    auto operator==(const LocalState &other) const noexcept -> bool;
    NOP_STRUCTURE(LocalState, current_reward, reward_signal, inventory);
};

// Game state
class CraftWorldGameState {
public:
    CraftWorldGameState(const GameParameters &params = kDefaultGameParams);

    /**
     * Construct from byte serialization.
     * @note this is not safe, only for internal use.
     */
    CraftWorldGameState(const std::vector<uint8_t> &byte_data);

    auto operator==(const CraftWorldGameState &other) const noexcept -> bool;
    auto operator!=(const CraftWorldGameState &other) const noexcept -> bool;

    /**
     * Reset the environment to the state as given by the GameParameters
     */
    void reset();

    /**
     * Serialize the state
     * @return char vector representing state
     */
    [[nodiscard]] auto serialize() const -> std::vector<uint8_t>;

    /**
     * Check if the given element is valid.
     * @param element Element to check
     * @return True if element is valid, false otherwise
     */
    [[nodiscard]] constexpr static auto is_valid_element(Element element) -> bool {
        return static_cast<int>(element) >= 0 && static_cast<int>(element) < static_cast<int>(kNumElements);
    }

    /**
     * Check if the given action is valid.
     * @param action Action to check
     * @return True if action is valid, false otherwise
     */
    [[nodiscard]] constexpr static auto is_valid_action(Action action) -> bool {
        return static_cast<int>(action) >= 0 && static_cast<int>(action) < static_cast<int>(kNumActions);
    }

    /**
     * Apply the action to the current state, and set the reward and signals.
     * @param action The action to apply, should be one of the legal actions
     */
    void apply_action(Action action);

    /**
     * Check if the state is in the solution state (agent inside exit).
     * @return True if terminal, false otherwise
     */
    [[nodiscard]] auto is_solution() const noexcept -> bool;

    /**
     * Get the legal actions which can be applied in the state.
     * @return vector containing each actions available
     */
    [[nodiscard]] auto legal_actions() const noexcept -> std::vector<Action>;

    /**
     * Get the legal actions which can be applied in the state, and store in the given vector.
     * @note Use when wanting to reuse a pre-allocated vector
     * @param actions The vector to store the available actions in
     */
    void legal_actions(std::vector<Action> &actions) const noexcept;

    /**
     * Get the shape the observations should be viewed as.
     * @return vector indicating observation CHW
     */
    [[nodiscard]] auto observation_shape() const noexcept -> std::array<int, 3>;

    /**
     * Get the shape the observations should be viewed as for the binary observation.
     * @return vector indicating observation CHW
     */
    [[nodiscard]] auto observation_shape_binary() const noexcept -> std::array<int, 3>;

    /**
     * Get the shape the observations should be viewed as.
     * @return vector indicating observation CHW
     */
    [[nodiscard]] auto observation_shape_environment() const noexcept -> std::array<int, 3>;

    /**
     * Get a flat representation of the current state observation.
     * The observation should be viewed as the shape given by observation_shape().
     * @return vector where 1 represents element at position
     */
    [[nodiscard]] auto get_observation() const noexcept -> std::vector<float>;

    /**
     * Get a flat representation of the current state observation in binary format.
     * The observation should be viewed as the shape given by observation_shape().
     * @return vector where 1 represents element at position
     */
    [[nodiscard]] auto get_binary_observation() const noexcept -> std::vector<float>;

    /**
     * Get a flat representation of the current state observation, and store in the given vector.
     * @note Use when wanting to reuse a pre-allocated vector
     * The observation should be viewed as the shape given by observation_shape(), where 1 represents the element at the
     * given position.
     * @param obs Vector to store the observation in
     */
    void get_observation(std::vector<float> &obs) const noexcept;

    /**
     * Get a flat representation of the current state observation without the goal or inventory
     * The observation should be viewed as the shape given by observation_shape().
     * @return vector where 1 represents element at position
     */
    [[nodiscard]] auto get_observation_environment() const noexcept -> std::vector<float>;

    /**
     * Get a flat representation of the current state observation without the goal or inventory, and store in the given
     * vector.
     * @note Use when wanting to reuse a pre-allocated vector
     * The observation should be viewed as the shape given by observation_shape(), where 1 represents the element at the
     * given position.
     * @param obs Vector to store the observation in
     */
    void get_observation_environment(std::vector<float> &obs) const noexcept;

    /**
     * Get the shape the image should be viewed as.
     * @return array indicating observation HWC
     */
    [[nodiscard]] auto image_shape() const noexcept -> std::array<std::size_t, 3>;

    /**
     * Get the flat (HWC) image representation of the current state
     * @return flattened byte vector represending RGB values (HWC)
     */
    [[nodiscard]] auto to_image() const noexcept -> std::vector<uint8_t>;

    /**
     * Get the current reward signal as a result of the previous action taken.
     * @return bit field representing events that occured
     */
    [[nodiscard]] auto get_reward_signal() const noexcept -> uint64_t;

    /**
     * Get the hash representation for the current state.
     * @return hash value
     */
    [[nodiscard]] auto get_hash() const noexcept -> uint64_t;

    /**
     * Add the given element to the inventory
     */
    void add_to_inventory(Element element, std::size_t count);

    /**
     * Get the agent index position, even if in exit
     * @return Agent index
     */
    [[nodiscard]] auto get_agent_index() const noexcept -> std::size_t;

    /**
     * Get all indices for a given element type
     * @param element The hidden cell type of the element to search for
     * @return flat indices for each instance of element
     */
    [[nodiscard]] auto get_indices(Element element) const noexcept -> std::vector<std::size_t>;

    /**
     * Get all possible subgoals
     * @return Vector of subgoals
     */
    [[nodiscard]] auto get_all_subgoals() const noexcept -> std::vector<std::size_t>;

    /**
     * Get string representation of subgoal
     * @param subgoal Subgoal to query
     * @return String name of subgoal
     */
    [[nodiscard]] auto subgoal_to_str(Subgoal subgoal) const noexcept -> std::string;

    // All possible actions
    static const std::vector<Action> ALL_ACTIONS;

    friend auto operator<<(std::ostream &os, const CraftWorldGameState &state) -> std::ostream &;

private:
    auto IndexFromAction(std::size_t index, Action action) const noexcept -> std::size_t;
    auto InBounds(std::size_t index, Action action) const noexcept -> bool;
    void SetNeighbours(std::size_t index) const noexcept;
    auto IsWorkShop(std::size_t index) const noexcept -> bool;
    auto IsPrimitive(std::size_t index) const noexcept -> bool;
    auto IsItem(std::size_t index, Element element) const noexcept -> bool;
    auto HasItemInInventory(Element element, std::size_t min_count = 1) const noexcept -> bool;
    void RemoveFromInventory(Element element, std::size_t count) noexcept;
    void AddToInventory(Element element, std::size_t count) noexcept;
    auto CanCraftItem(RecipeItem recipe_item) const noexcept -> bool;
    void HandleAgentMovement(Action action) noexcept;
    void HandleAgentUse() noexcept;
    void RemoveItemFromBoard(std::size_t index) noexcept;
    void InitZrbhtTable() noexcept;

    std::shared_ptr<SharedStateInfo> shared_state_ptr;
    Board board;
    LocalState local_state;
};

}    // namespace craftworld

#endif    // CRAFTWORLD_BASE_H_
