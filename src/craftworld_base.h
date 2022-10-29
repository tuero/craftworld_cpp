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
    GameParameters params;                                // Copy of game parameters for state resetting
    int rng_seed;                                         // Seed
    std::string game_board_str;                           // String representation of the starting state
    std::mt19937 gen;                                     // Generator for RNG
    std::uniform_int_distribution<int> dist;              // Random int distribution
    std::unordered_map<int, uint64_t> zrbht_world;        // Zobrist hashing table
    std::unordered_map<int, uint64_t> zrbht_inventory;    // Zobrist hashing table for inventory
    const int MAX_INV_HASH_ITEMS = 20;
};

// Information specific for the current game state
struct LocalState {
    LocalState() = default;
    uint8_t current_reward = 0;                        // Reward for the current game state
    uint64_t reward_signal = 0;                        // Signal for external information about events
    std::unordered_map<ElementType, int> inventory;    // Inventory of items
};

// Game state
class CraftWorldGameState {
public:
    CraftWorldGameState(const GameParameters &params = kDefaultGameParams);

    bool operator==(const CraftWorldGameState &other) const;

    /**
     * Reset the environment to the state as given by the GameParameters
     */
    void reset();

    /**
     * Apply the action to the current state, and set the reward and signals.
     * @param action The action to apply, should be one of the legal actions
     */
    void apply_action(int action);

    /**
     * Check if the state is in the solution state (agent inside exit).
     * @return True if terminal, false otherwise
     */
    bool is_solution() const;

    /**
     * Get the legal actions which can be applied in the state.
     * @return vector containing each actions available
     */
    std::vector<int> legal_actions() const;

    /**
     * Get the shape the observations should be viewed as.
     * @return vector indicating observation CHW
     */
    std::array<int, 3> observation_shape() const;

    /**
     * Get a flat representation of the current state observation.
     * The observation should be viewed as the shape given by observation_shape().
     * @return vector where 1 represents object at position
     */
    std::vector<float> get_observation() const;

    /**
     * Get the current reward signal as a result of the previous action taken.
     * @return bit field representing events that occured
     */
    uint64_t get_reward_signal() const;

    /**
     * Get the hash representation for the current state.
     * @return hash value
     */
    uint64_t get_hash() const;

    /**
     * Get the agent index position, even if in exit
     * @return Agent index
     */
    int get_agent_index() const;

    /**
     * Get all the pruned subgoals that can be done
     * @return Vector of pruned subgoals
     */
    std::vector<int> get_pruned_subgoals() const;

    /**
     * Get all possible subgoals
     * @return Vector of subgoals
     */
    std::vector<int> get_all_subgoals() const;

    /**
     * Observation shape if rows and cols are given.
     * @param rows Number of rows
     * @param cols Number of cols
     * @return vector indicating observation CHW
     */
    static std::array<int, 3> observation_shape(int rows, int cols) {
        return {kNumChannels, rows, cols};
    }

    friend std::ostream &operator<<(std::ostream &os, const CraftWorldGameState &state);

private:
    int IndexFromAction(int index, int action) const;
    bool InBounds(int index, int action = Actions::kNoop) const;
    std::vector<int> GetNeighbours(int index) const;
    bool IsWorkShop(int index) const;
    bool IsPrimitive(int index) const;
    bool IsItem(int index, ElementType element) const;
    bool HasItemInInventory(ElementType element, int min_count = 1) const;
    bool HasItemInInventory(ElementType element, std::unordered_map<ElementType, int> inventory,
                            int min_count = 1) const;
    void RemoveFromInventory(ElementType element, int count);
    void AddToInventory(ElementType element, int count);
    bool CanCraftItem(RecipeItem recipe_item) const;
    bool CanCraftItem(RecipeItem recipe_item, std::unordered_map<ElementType, int> inventory) const;
    void HandleAgentMovement(int action);
    void HandleAgentUse();

    std::shared_ptr<SharedStateInfo> shared_state_ptr;
    Board board;
    LocalState local_state;
};

}    // namespace craftworld

#endif    // CRAFTWORLD_BASE_H_