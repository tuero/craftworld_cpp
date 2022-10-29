#include "craftworld_base.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>

#include "definitions.h"

namespace craftworld {

CraftWorldGameState::CraftWorldGameState(const GameParameters &params)
    : shared_state_ptr(std::make_shared<SharedStateInfo>(params)),
      board(util::parse_board_str(std::get<std::string>(params.at("game_board_str")))) {
    reset();
}

bool CraftWorldGameState::operator==(const CraftWorldGameState &other) const {
    return board == other.board;
}

// ---------------------------------------------------------------------------

void CraftWorldGameState::reset() {
    // Board, local, and shared state info
    board = util::parse_board_str(shared_state_ptr->game_board_str);
    local_state = LocalState();

    // zorbist hashing
    std::mt19937 gen(shared_state_ptr->rng_seed);
    std::uniform_int_distribution<uint64_t> dist(0);
    for (int channel = 0; channel < kNumElementTypes; ++channel) {
        for (int i = 0; i < board.cols * board.rows; ++i) {
            shared_state_ptr->zrbht_world[(channel * board.cols * board.rows) + i] = dist(gen);
        }
    }

    // Set initial hash for game world
    for (int i = 0; i < board.cols * board.rows; ++i) {
        board.zorb_hash ^=
            shared_state_ptr->zrbht_world.at((static_cast<int>(board.item(i)) * board.cols * board.rows) + i);
    }
    // Set initial hash for inventory
    for (int channel = 0; channel < kNumElementTypes; ++channel) {
        for (int i = 0; i < shared_state_ptr->MAX_INV_HASH_ITEMS; ++i) {
            shared_state_ptr->zrbht_inventory[(channel * shared_state_ptr->MAX_INV_HASH_ITEMS) + i] = dist(gen);
        }
    }
}

void CraftWorldGameState::HandleAgentMovement(int action) {
    // Move if in bound and empty tile
    int agent_idx = board.agent_idx;
    int new_idx = IndexFromAction(agent_idx, action);
    if (InBounds(agent_idx, action) && static_cast<ElementType>(board.item(new_idx)) == ElementType::kEmpty) {
        board.zorb_hash ^= shared_state_ptr->zrbht_world.at(
            (static_cast<int>(ElementType::kAgent) * board.cols * board.rows) + agent_idx);
        board.zorb_hash ^= shared_state_ptr->zrbht_world.at(
            (static_cast<int>(ElementType::kEmpty) * board.cols * board.rows) + new_idx);
        board.item(new_idx) = static_cast<char>(ElementType::kAgent);
        board.item(agent_idx) = static_cast<char>(ElementType::kEmpty);
        board.agent_idx = new_idx;
        board.zorb_hash ^= shared_state_ptr->zrbht_world.at(
            (static_cast<int>(ElementType::kAgent) * board.cols * board.rows) + new_idx);
        board.zorb_hash ^= shared_state_ptr->zrbht_world.at(
            (static_cast<int>(ElementType::kEmpty) * board.cols * board.rows) + agent_idx);
    }
}

void CraftWorldGameState::HandleAgentUse() {
    int agent_idx = board.agent_idx;
    // Check all neighbours (we don't have directional look)
    for (auto const &neighbour_idx : GetNeighbours(agent_idx)) {
        // Nothing on this index to do something
        if (static_cast<ElementType>(board.item(neighbour_idx)) == ElementType::kEmpty) {
            continue;
        }

        if (IsPrimitive(neighbour_idx)) {
            // Primitive elements on map are collectable, add to inventory
            ElementType el = static_cast<ElementType>(board.item(neighbour_idx));
            AddToInventory(el, 1);
            board.item(neighbour_idx) = static_cast<char>(ElementType::kEmpty);
            local_state.reward_signal |= static_cast<std::underlying_type_t<RewardCodes>>(kPrimitiveRewardMap.at(el));
            break;
        } else if (IsWorkShop(neighbour_idx)) {
            ElementType el_workshop = static_cast<ElementType>(board.item(neighbour_idx));
            for (const auto &[recipe_type, recipe_item] : kRecipeMap) {
                // Skip recipes not legal at this workshop
                if (recipe_item.location != el_workshop) {
                    continue;
                }
                // Skip if we don't have the items required in our inventory
                if (!CanCraftItem(recipe_item)) {
                    continue;
                }

                // Add crafted item and remove ingredients from inventory
                AddToInventory(recipe_item.output, 1);
                for (auto const &ingredient_item : recipe_item.inputs) {
                    RemoveFromInventory(ingredient_item.element, ingredient_item.count);
                }
                local_state.reward_signal |=
                    static_cast<std::underlying_type_t<RewardCodes>>(kRecipeRewardMap.at(recipe_item.recipe));
            }
            break;
        } else if (IsItem(neighbour_idx, ElementType::kWater)) {
            // Remove water with a bridge
            if (HasItemInInventory(ElementType::kBridge)) {
                RemoveFromInventory(ElementType::kBridge, 1);
                board.item(neighbour_idx) = static_cast<char>(ElementType::kEmpty);
                break;
            }
        } else if (IsItem(neighbour_idx, ElementType::kStone)) {
            // Remove stone with an axe
            if (HasItemInInventory(ElementType::kAxe)) {
                board.item(neighbour_idx) = static_cast<char>(ElementType::kEmpty);
                break;
            }
        }
    }
}

void CraftWorldGameState::apply_action(int action) {
    assert(action >= 0 && action < kNumActions);
    local_state.reward_signal = 0;
    if (action > Actions::kNoop && action < Actions::kUse) {
        HandleAgentMovement(action);
    } else {
        HandleAgentUse();
    }
}

bool CraftWorldGameState::is_solution() const {
    // Inventory contains the goal item
    return local_state.inventory.find(static_cast<ElementType>(board.goal)) != local_state.inventory.end();
}

std::vector<int> CraftWorldGameState::legal_actions() const {
    return {Actions::kUp, Actions::kRight, Actions::kDown, Actions::kLeft, Actions::kUse};
}

std::array<int, 3> CraftWorldGameState::observation_shape() const {
    // Empty doesn't get a channel, empty = all channels 0
    return {kNumChannels, board.cols, board.rows};
}

std::vector<float> CraftWorldGameState::get_observation() const {
    int channel_length = board.cols * board.rows;
    std::vector<float> obs(kNumChannels * channel_length, 0);

    // Board environment + primitives + agent (0-11)
    for (int i = 0; i < channel_length; ++i) {
        ElementType el = static_cast<ElementType>(board.item(i));
        if (el != ElementType::kEmpty) {
            obs[static_cast<int>(el) * channel_length + i] = 1;
        }
    }
    // Inventory (entire channel is filled with # of that item) (12 - 25)
    for (const auto &inventory_item : local_state.inventory) {
        int channel = static_cast<int>(inventory_item.first) - kNumPrimitive + kNumEnvironment + kNumPrimitive;
        std::fill_n(obs.begin() + (channel * channel_length), channel_length, inventory_item.second);
    }
    // Current goal for this level (26-34)
    int channel = kNumChannels - kNumGoals + board.goal - kRecipeStart;
    std::fill_n(obs.begin() + (channel * channel_length), channel_length, 1);
    return obs;
}

uint64_t CraftWorldGameState::get_reward_signal() const {
    return local_state.reward_signal;
}

uint64_t CraftWorldGameState::get_hash() const {
    return board.zorb_hash;
}

int CraftWorldGameState::get_agent_index() const {
    return board.agent_idx;
}

std::vector<int> CraftWorldGameState::get_pruned_subgoals() const {
    std::vector<int> subgoals;
    // Can only do a subgoal if we have the recipe ingredients for it or the items required are in the map
    std::unordered_map<ElementType, int> world_inventory;    // Inventory of items on the map
    for (int index = 0; index < board.rows * board.cols; ++index) {
        if (IsPrimitive(index)) {
            ++world_inventory[static_cast<ElementType>(board.item(index))];
        }
    }
    // Combine inventories
    for (const auto &[element, count] : local_state.inventory) {
        world_inventory[element] += count;
    }

    // Check if we can make the items
    for (const auto &[_, recipe_item] : kRecipeMap) {
        if (CanCraftItem(recipe_item, world_inventory)) {
            subgoals.push_back(static_cast<int>(recipe_item.output));
        }
    }
    return subgoals;
}

std::vector<int> CraftWorldGameState::get_all_subgoals() const {
    return all_subgoals;
}

std::ostream &operator<<(std::ostream &os, const CraftWorldGameState &state) {
    for (int w = 0; w < state.board.cols + 2; ++w) {
        os << "-";
    }
    os << std::endl;
    for (int h = 0; h < state.board.rows; ++h) {
        os << "|";
        for (int w = 0; w < state.board.cols; ++w) {
            int idx = h * state.board.cols + w;
            os << kElementToSymbolMap.at(static_cast<ElementType>(state.board.item(idx)));
        }
        os << "|" << std::endl;
    }
    for (int w = 0; w < state.board.cols + 2; ++w) {
        os << "-";
    }
    os << std::endl;
    os << "Goal: " << kElementToNameMap.at(static_cast<ElementType>(state.board.goal)) << std::endl;
    os << "Inventory: ";
    for (const auto &inventory_item : state.local_state.inventory) {
        os << "(" << kElementToNameMap.at(static_cast<ElementType>(inventory_item.first)) << ", "
           << inventory_item.second << ") ";
    }
    return os;
}

// ---------------------------------------------------------------------------

int CraftWorldGameState::IndexFromAction(int index, int action) const {
    switch (action) {
        case Actions::kNoop:
            return index;
        case Actions::kUp:
            return index - board.cols;
        case Actions::kRight:
            return index + 1;
        case Actions::kDown:
            return index + board.cols;
        case Actions::kLeft:
            return index - 1;
        default:
            __builtin_unreachable();
    }
}

bool CraftWorldGameState::InBounds(int index, int action) const {
    int col = index % board.cols;
    int row = (index - col) / board.cols;
    std::pair<int, int> offsets = kDirectionOffsets.at(action);
    col += offsets.first;
    row += offsets.second;
    return col >= 0 && col < board.cols && row >= 0 && row < board.rows;
}

std::vector<int> CraftWorldGameState::GetNeighbours(int index) const {
    std::vector<int> child_idx;
    for (auto const &action : {Actions::kUp, Actions::kRight, Actions::kDown, Actions::kLeft}) {
        if (InBounds(index, action)) {
            child_idx.push_back(IndexFromAction(index, action));
        }
    }
    return child_idx;
}

bool CraftWorldGameState::IsWorkShop(int index) const {
    return kWorkShops.find(static_cast<ElementType>(board.item(index))) != kWorkShops.end();
}

bool CraftWorldGameState::IsPrimitive(int index) const {
    return kPrimitives.find(static_cast<ElementType>(board.item(index))) != kPrimitives.end();
}

bool CraftWorldGameState::IsItem(int index, ElementType element) const {
    return static_cast<ElementType>(board.item(index)) == element;
}

bool CraftWorldGameState::HasItemInInventory(ElementType element, int min_count) const {
    return local_state.inventory.find(element) != local_state.inventory.end() &&
           local_state.inventory.at(element) >= min_count;
}

bool CraftWorldGameState::HasItemInInventory(ElementType element, std::unordered_map<ElementType, int> inventory,
                                             int min_count) const {
    return inventory.find(element) != inventory.end() && inventory.at(element) >= min_count;
}

void CraftWorldGameState::RemoveFromInventory(ElementType element, int count) {
    // Caller needs to verify that we can remove from inventory
    // Decrement item `count` times and change game state hash
    for (int i = 0; i < count; ++i) {
        board.zorb_hash ^= shared_state_ptr->zrbht_inventory.at(
            (static_cast<int>(element) * shared_state_ptr->MAX_INV_HASH_ITEMS) + local_state.inventory.at(element));
        --local_state.inventory[element];
    }
    if (local_state.inventory[element] <= 0) {
        local_state.inventory.erase(element);
    }
}

void CraftWorldGameState::AddToInventory(ElementType element, int count) {
    // Increment item `count` times and change game state hash
    for (int i = 0; i < count; ++i) {
        ++local_state.inventory[element];
        board.zorb_hash ^= shared_state_ptr->zrbht_inventory.at(
            (static_cast<int>(element) * shared_state_ptr->MAX_INV_HASH_ITEMS) + local_state.inventory.at(element));
    }
}

bool CraftWorldGameState::CanCraftItem(RecipeItem recipe_item) const {
    for (auto const &ingredient_item : recipe_item.inputs) {
        if (!HasItemInInventory(ingredient_item.element, ingredient_item.count)) {
            return false;
        }
    }
    return true;
}

bool CraftWorldGameState::CanCraftItem(RecipeItem recipe_item, std::unordered_map<ElementType, int> inventory) const {
    for (auto const &ingredient_item : recipe_item.inputs) {
        if (!HasItemInInventory(ingredient_item.element, inventory, ingredient_item.count)) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------

}    // namespace craftworld
