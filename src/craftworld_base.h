#ifndef CRAFTWORLD_BASE_H_
#define CRAFTWORLD_BASE_H_

#include <array>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "definitions.h"
#include "util.h"

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
    {"rng_seed", GameParameter(0)},    // Seed for anything that uses the rng
    {"game_board_str", GameParameter(std::string("3|3|10|00|11|21|21|21|21|21|21|21"))},    // Game board string
};

// Shared global state information relevant to all states for the given game
struct SharedStateInfo {
    SharedStateInfo(const GameParameters &params)
        : params(params),
          rng_seed(std::get<int>(params.at("rng_seed"))),
          game_board_str(std::get<std::string>(params.at("game_board_str"))),
          gen(rng_seed),
          dist(0) {}
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    GameParameters params;                                        // Copy of game parameters for state resetting
    int rng_seed;                                                 // Seed
    std::string game_board_str;                                   // String representation of the starting state
    std::mt19937 gen;                                             // Generator for RNG
    std::uniform_int_distribution<int> dist;                      // Random int distribution
    std::unordered_map<std::size_t, uint64_t> zrbht_world;        // Zobrist hashing table
    std::unordered_map<std::size_t, uint64_t> zrbht_inventory;    // Zobrist hashing table for inventory
    std::vector<std::size_t> neighbours;                          // Reusable buffer for finding neighbours
    const std::size_t MAX_INV_HASH_ITEMS = 20;                    // NOLINT
    // NOLINTEND(misc-non-private-member-variables-in-classes)
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
};

// Game state
class CraftWorldGameState {
public:
    CraftWorldGameState(const GameParameters &params = kDefaultGameParams);

    auto operator==(const CraftWorldGameState &other) const noexcept -> bool;

    /**
     * Reset the environment to the state as given by the GameParameters
     */
    void reset();

    /**
     * Apply the action to the current state, and set the reward and signals.
     * @param action The action to apply, should be one of the legal actions
     */
    void apply_action(Action action) noexcept;

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

    std::shared_ptr<SharedStateInfo> shared_state_ptr;
    Board board;
    LocalState local_state;
};

}    // namespace craftworld

#endif    // CRAFTWORLD_BASE_H_
