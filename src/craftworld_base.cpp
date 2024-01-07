#include "craftworld_base.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>

#include "definitions.h"

namespace craftworld {

auto LocalState::operator==(const LocalState &other) const noexcept -> bool {
    return inventory == other.inventory;
}

CraftWorldGameState::CraftWorldGameState(const GameParameters &params)
    : shared_state_ptr(std::make_shared<SharedStateInfo>(params)),
      board(util::parse_board_str(std::get<std::string>(params.at("game_board_str")))) {
    reset();
}

auto CraftWorldGameState::operator==(const CraftWorldGameState &other) const noexcept -> bool {
    return board == other.board && local_state == other.local_state;
}

const std::vector<Action> CraftWorldGameState::ALL_ACTIONS = {Action::kUp, Action::kRight, Action::kDown, Action::kLeft,
                                                              Action::kUse};

// ---------------------------------------------------------------------------

void CraftWorldGameState::reset() {
    // Board, local, and shared state info
    board = util::parse_board_str(shared_state_ptr->game_board_str);
    local_state = LocalState();
    const std::size_t board_size = board.rows * board.cols;

    // Zorbist hashing for board
    std::mt19937 gen(shared_state_ptr->rng_seed);
    std::uniform_int_distribution<uint64_t> dist(0);
    for (std::size_t channel = 0; channel < kNumElements; ++channel) {
        for (std::size_t i = 0; i < board_size; ++i) {
            shared_state_ptr->zrbht_world[(channel * board_size) + i] = dist(gen);
        }
    }

    // Set initial hash for game world
    for (std::size_t i = 0; i < board_size; ++i) {
        board.zorb_hash ^= shared_state_ptr->zrbht_world.at((static_cast<int>(board.item(i)) * board_size) + i);
    }
    // Zorbist hashing for inventory
    for (std::size_t channel = 0; channel < kNumElements; ++channel) {
        for (std::size_t i = 0; i < shared_state_ptr->MAX_INV_HASH_ITEMS; ++i) {
            shared_state_ptr->zrbht_inventory[(channel * shared_state_ptr->MAX_INV_HASH_ITEMS) + i] = dist(gen);
        }
    }
}

void CraftWorldGameState::RemoveItemFromBoard(std::size_t index) noexcept {
    const Element el = board.item(index);
    const std::size_t board_size = board.rows * board.cols;
    board.zorb_hash ^= shared_state_ptr->zrbht_world.at((static_cast<std::size_t>(el) * board_size) + index);
    board.item(index) = Element::kEmpty;
}

void CraftWorldGameState::HandleAgentMovement(Action action) noexcept {
    // Move if in bound and empty tile
    const std::size_t agent_idx = board.agent_idx;
    const std::size_t new_idx = IndexFromAction(agent_idx, action);
    if (InBounds(agent_idx, action) && board.item(new_idx) == Element::kEmpty) {
        board.zorb_hash ^=
            shared_state_ptr
                ->zrbht_world[(static_cast<std::size_t>(Element::kAgent) * board.cols * board.rows) + agent_idx];
        board.zorb_hash ^=
            shared_state_ptr
                ->zrbht_world[(static_cast<std::size_t>(Element::kEmpty) * board.cols * board.rows) + new_idx];
        board.item(new_idx) = Element::kAgent;
        board.item(agent_idx) = Element::kEmpty;
        board.agent_idx = new_idx;
        board.zorb_hash ^=
            shared_state_ptr
                ->zrbht_world[(static_cast<std::size_t>(Element::kAgent) * board.cols * board.rows) + new_idx];
        board.zorb_hash ^=
            shared_state_ptr
                ->zrbht_world[(static_cast<std::size_t>(Element::kEmpty) * board.cols * board.rows) + agent_idx];
    }
}

void CraftWorldGameState::HandleAgentUse() noexcept {
    const std::size_t agent_idx = board.agent_idx;
    // Check all neighbours (we don't have directional look)
    SetNeighbours(agent_idx);
    for (auto const &neighbour_idx : shared_state_ptr->neighbours) {
        // Nothing on this index to do something
        if (board.item(neighbour_idx) == Element::kEmpty) {
            continue;
        }

        if (IsPrimitive(neighbour_idx)) {
            // Primitive elements on map are collectable, add to inventory
            const Element el = board.item(neighbour_idx);
            AddToInventory(el, 1);
            RemoveItemFromBoard(neighbour_idx);
            local_state.reward_signal |= static_cast<std::underlying_type_t<RewardCode>>(kPrimitiveRewardMap.at(el));
            break;
        } else if (board.item(neighbour_idx) == Element::kIron && HasItemInInventory(Element::kBronzePick)) {
            // Iron ingot is special primitive where we need a cobble stone pickaxe to gather
            const Element el = board.item(neighbour_idx);
            AddToInventory(el, 1);
            RemoveItemFromBoard(neighbour_idx);
            local_state.reward_signal |= static_cast<std::underlying_type_t<RewardCode>>(kPrimitiveRewardMap.at(el));
            break;
        } else if (IsWorkShop(neighbour_idx)) {
            const Element el_workshop = board.item(neighbour_idx);
            for (const auto &[recipe_type, recipe_item] : kRecipeMap) {
                // Skip recipes not legal at this workshop
                const auto recipe_workshop =
                    shared_state_ptr->workshop_swap ? kWorkshopSwap.at(recipe_item.location) : recipe_item.location;
                if (recipe_workshop != el_workshop) {
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
                    static_cast<std::underlying_type_t<RewardCode>>(kRecipeRewardMap.at(recipe_item.recipe));
                local_state.reward_signal |=
                    static_cast<std::underlying_type_t<RewardCode>>(kWorkstationRewardMap.at(el_workshop));
                break;
            }
            break;
        } else if (IsItem(neighbour_idx, Element::kWater)) {
            // Remove water with a bridge
            if (HasItemInInventory(Element::kBridge)) {
                RemoveFromInventory(Element::kBridge, 1);
                RemoveItemFromBoard(neighbour_idx);
                local_state.reward_signal |=
                    static_cast<std::underlying_type_t<RewardCode>>(RewardCode::kRewardCodeUseBridge);
                break;
            }
        } else if (IsItem(neighbour_idx, Element::kStone)) {
            // Remove stone with an axe
            if (HasItemInInventory(Element::kIronPick)) {
                RemoveFromInventory(Element::kIronPick, 1);
                RemoveItemFromBoard(neighbour_idx);
                local_state.reward_signal |=
                    static_cast<std::underlying_type_t<RewardCode>>(RewardCode::kRewardCodeUseAxe);
                break;
            }
        }
    }
}

void CraftWorldGameState::apply_action(Action action) noexcept {
    local_state.reward_signal = 0;
    if (action == Action::kUse) {
        HandleAgentUse();
    } else {
        HandleAgentMovement(action);
    }
}

auto CraftWorldGameState::is_solution() const noexcept -> bool {
    // Inventory contains the goal item
    return local_state.inventory.find(board.goal) != local_state.inventory.end();
}

auto CraftWorldGameState::legal_actions() const noexcept -> std::vector<Action> {
    return ALL_ACTIONS;
}

void CraftWorldGameState::legal_actions(std::vector<Action> &actions) const noexcept {
    actions.clear();
    for (const auto &a : ALL_ACTIONS) {
        actions.push_back(a);
    }
}

auto CraftWorldGameState::observation_shape() const noexcept -> std::array<int, 3> {
    // Empty doesn't get a channel, empty = all channels 0
    return {kNumChannels, static_cast<int>(board.rows), static_cast<int>(board.cols)};
}

auto CraftWorldGameState::observation_shape_environment() const noexcept -> std::array<int, 3> {
    return {kNumEnvironment + kNumPrimitive, static_cast<int>(board.rows), static_cast<int>(board.cols)};
}

auto CraftWorldGameState::get_observation() const noexcept -> std::vector<float> {
    const std::size_t channel_length = board.rows * board.cols;
    std::vector<float> obs(kNumChannels * channel_length, 0);
    get_observation(obs);
    return obs;
}

void CraftWorldGameState::get_observation(std::vector<float> &obs) const noexcept {
    const std::size_t channel_length = board.rows * board.cols;
    const std::size_t obs_size = kNumChannels * channel_length;

    obs.clear();
    obs.reserve(obs_size);
    std::fill_n(std::back_inserter(obs), obs_size, 0);

    // Board environment + primitives + agent
    for (std::size_t i = 0; i < channel_length; ++i) {
        const auto el = board.item(i);
        if (el != Element::kEmpty) {
            obs[static_cast<std::size_t>(el) * channel_length + i] = 1;
        }
    }
    // Inventory (entire channel is filled with # of that item)
    for (const auto &[inv_el, inv_count] : local_state.inventory) {
        const auto channel = static_cast<std::size_t>(inv_el) + kNumEnvironment;
        std::fill_n(obs.begin() + static_cast<int>(channel * channel_length), channel_length, inv_count);
    }
    // Current goal for this level (26-34)
    const std::size_t channel = kNumChannels - kNumGoals + static_cast<std::size_t>(board.goal) - kRecipeStart;
    std::fill_n(obs.begin() + static_cast<int>(channel * channel_length), channel_length, 1);
}

auto CraftWorldGameState::get_observation_environment() const noexcept -> std::vector<float> {
    const std::size_t channel_length = board.cols * board.rows;
    std::vector<float> obs((kNumEnvironment + kNumPrimitive) * channel_length, 0);
    get_observation_environment(obs);
    return obs;
}

void CraftWorldGameState::get_observation_environment(std::vector<float> &obs) const noexcept {
    const std::size_t channel_length = board.cols * board.rows;
    const std::size_t obs_size = (kNumEnvironment + kNumPrimitive) * channel_length;
    obs.clear();
    obs.reserve(obs_size);
    std::fill_n(std::back_inserter(obs), obs_size, 0);

    // Board environment + primitives + agent (0-11)
    for (std::size_t i = 0; i < channel_length; ++i) {
        const auto el = board.item(i);
        if (el != Element::kEmpty) {
            obs[static_cast<std::size_t>(el) * channel_length + i] = 1;
        }
    }
}

// Spite assets
#include "assets_all.inc"

void fill_sprite(std::vector<uint8_t> &img, const std::vector<uint8_t> &sprite_data, std::size_t h, std::size_t w,
                 std::size_t cols) {
    const std::size_t img_idx_top_left = h * (SPRITE_DATA_LEN * cols) + (w * SPRITE_DATA_LEN_PER_ROW);
    for (std::size_t r = 0; r < SPRITE_HEIGHT; ++r) {
        for (std::size_t c = 0; c < SPRITE_WIDTH; ++c) {
            const std::size_t data_idx = (r * SPRITE_DATA_LEN_PER_ROW) + (3 * c);
            const std::size_t img_idx = (r * SPRITE_DATA_LEN_PER_ROW * cols) + (SPRITE_CHANNELS * c) + img_idx_top_left;
            img[img_idx + 0] = sprite_data[data_idx + 0];
            img[img_idx + 1] = sprite_data[data_idx + 1];
            img[img_idx + 2] = sprite_data[data_idx + 2];
        }
    }
}

auto CraftWorldGameState::image_shape() const noexcept -> std::array<std::size_t, 3> {
    const auto rows = board.rows + 4;
    const auto cols = board.cols + 4;
    return {rows * SPRITE_HEIGHT, cols * SPRITE_WIDTH, SPRITE_CHANNELS};
}

auto CraftWorldGameState::to_image() const noexcept -> std::vector<uint8_t> {
    // Pad board with black border
    const auto rows = board.rows + 4;
    const auto cols = board.cols + 4;
    const auto channel_length = rows * cols;
    std::vector<uint8_t> img(channel_length * SPRITE_DATA_LEN, 0);

    // Inner border is wall
    for (std::size_t w = 1; w < cols - 1; ++w) {
        fill_sprite(img, img_asset_map.at(Element::kWall), 1, w, cols);
        fill_sprite(img, img_asset_map.at(Element::kWall), rows - 2, w, cols);
    }
    for (std::size_t h = 1; h < rows - 1; ++h) {
        fill_sprite(img, img_asset_map.at(Element::kWall), h, 1, cols);
        fill_sprite(img, img_asset_map.at(Element::kWall), h, cols - 2, cols);
    }

    // Outer border is inventory
    std::vector<std::pair<std::size_t, std::size_t>> indices;
    for (std::size_t w = 0; w < cols; ++w) {
        indices.emplace_back(0, w);
    }
    for (std::size_t w = 0; w < cols; ++w) {
        indices.emplace_back(rows - 1, w);
    }
    std::size_t inv_idx = 0;
    for (const auto &[inv_item, inv_count] : local_state.inventory) {
        for (std::size_t i = 0; i < inv_count; ++i) {
            fill_sprite(img, img_asset_map.at(inv_item), indices[inv_idx].first, indices[inv_idx].second, cols);
            ++inv_idx;
        }
    }

    // Reset of board is inside the border
    std::size_t board_idx = 0;
    for (std::size_t h = 2; h < rows - 2; ++h) {
        for (std::size_t w = 2; w < cols - 2; ++w) {
            const auto el = board.item(board_idx);
            fill_sprite(img, img_asset_map.at(el), h, w, cols);
            ++board_idx;
        }
    }
    return img;
}

auto CraftWorldGameState::get_reward_signal() const noexcept -> uint64_t {
    return local_state.reward_signal;
}

auto CraftWorldGameState::get_hash() const noexcept -> uint64_t {
    return board.zorb_hash;
}

auto CraftWorldGameState::get_agent_index() const noexcept -> std::size_t {
    return board.agent_idx;
}

auto CraftWorldGameState::get_indices(Element element) const noexcept -> std::vector<std::size_t> {
    std::vector<std::size_t> indices;
    for (std::size_t index = 0; index < board.rows * board.cols; ++index) {
        if (board.item(index) == element) {
            indices.push_back(index);
        }
    }
    return indices;
}

auto CraftWorldGameState::get_all_subgoals() const noexcept -> std::vector<std::size_t> {
    return all_subgoals;
}

std::ostream &operator<<(std::ostream &os, const CraftWorldGameState &state) {
    for (std::size_t w = 0; w < state.board.cols + 2; ++w) {
        os << "-";
    }
    os << std::endl;
    for (std::size_t h = 0; h < state.board.rows; ++h) {
        os << "|";
        for (std::size_t w = 0; w < state.board.cols; ++w) {
            const std::size_t idx = h * state.board.cols + w;
            os << kElementToSymbolMap.at(state.board.item(idx));
        }
        os << "|" << std::endl;
    }
    for (std::size_t w = 0; w < state.board.cols + 2; ++w) {
        os << "-";
    }
    os << std::endl;
    os << "Goal: " << kElementToNameMap.at(state.board.goal) << std::endl;
    os << "Inventory: ";
    for (const auto &[inv_item, inv_count] : state.local_state.inventory) {
        os << "(" << kElementToNameMap.at(inv_item) << ", " << inv_count << ") ";
    }
    return os;
}

// ---------------------------------------------------------------------------

auto CraftWorldGameState::IndexFromAction(std::size_t index, Action action) const noexcept -> std::size_t {
    switch (action) {
        case Action::kUp:
            return index - board.cols;
        case Action::kRight:
            return index + 1;
        case Action::kDown:
            return index + board.cols;
        case Action::kLeft:
            return index - 1;
        case Action::kUse:
            return index;
        default:
            __builtin_unreachable();
    }
}

auto CraftWorldGameState::InBounds(std::size_t index, Action action) const noexcept -> bool {
    int col = static_cast<int>(index % board.cols);
    int row = static_cast<int>((index - col) / board.cols);
    const std::pair<int, int> &offsets = kDirectionOffsets.at(static_cast<std::size_t>(action));
    col += offsets.first;
    row += offsets.second;
    return col >= 0 && col < static_cast<int>(board.cols) && row >= 0 && row < static_cast<int>(board.rows);
}

void CraftWorldGameState::SetNeighbours(std::size_t index) const noexcept {
    shared_state_ptr->neighbours.clear();
    for (auto const &action : ALL_ACTIONS) {
        if (InBounds(index, action)) {
            shared_state_ptr->neighbours.push_back(IndexFromAction(index, action));
        }
    }
}

auto CraftWorldGameState::IsWorkShop(std::size_t index) const noexcept -> bool {
    return kWorkShops.find(board.item(index)) != kWorkShops.end();
}

auto CraftWorldGameState::IsPrimitive(std::size_t index) const noexcept -> bool {
    return kPrimitives.find(board.item(index)) != kPrimitives.end();
}

auto CraftWorldGameState::IsItem(std::size_t index, Element element) const noexcept -> bool {
    return board.item(index) == element;
}

auto CraftWorldGameState::HasItemInInventory(Element element, std::size_t min_count) const noexcept -> bool {
    return local_state.inventory.find(element) != local_state.inventory.end() &&
           local_state.inventory.at(element) >= min_count;
}

void CraftWorldGameState::RemoveFromInventory(Element element, std::size_t count) noexcept {
    // Caller needs to verify that we can remove from inventory
    // Decrement item `count` times and change game state hash
    assert(local_state.inventory.find(element) != local_state.inventory.end());
    assert(local_state.inventory[element] >= count);
    for (std::size_t i = 0; i < count; ++i) {
        board.zorb_hash ^= shared_state_ptr->zrbht_inventory.at(
            (static_cast<std::size_t>(element) * shared_state_ptr->MAX_INV_HASH_ITEMS) +
            local_state.inventory[element]);
        --local_state.inventory[element];
    }
    if (local_state.inventory[element] <= 0) {
        local_state.inventory.erase(element);
    }
}

void CraftWorldGameState::AddToInventory(Element element, std::size_t count) noexcept {
    // Increment item `count` times and change game state hash
    for (std::size_t i = 0; i < count; ++i) {
        ++local_state.inventory[element];
        board.zorb_hash ^= shared_state_ptr->zrbht_inventory.at(
            (static_cast<std::size_t>(element) * shared_state_ptr->MAX_INV_HASH_ITEMS) +
            local_state.inventory[element]);
    }
}

auto CraftWorldGameState::CanCraftItem(RecipeItem recipe_item) const noexcept -> bool {
    for (auto const &ingredient_item : recipe_item.inputs) {
        if (!HasItemInInventory(ingredient_item.element, ingredient_item.count)) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------

}    // namespace craftworld
