#ifndef CRAFTWORLD_DEFS_H_
#define CRAFTWORLD_DEFS_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace craftworld {

enum class Element : unsigned char {
    kAgent = 0,    // Env
    kWall = 1,
    kWorkshop1 = 2,
    kWorkshop2 = 3,
    kWorkshop3 = 4,
    kFurnace = 5,
    kWater = 6,
    kStone = 7,
    kIron = 8,    // Primitives
    kTin = 9,
    kCopper = 10,
    kWood = 11,
    kGrass = 12,
    kGold = 13,
    kGem = 14,
    kBronzeBar = 15,    // Recipes
    kStick = 16,
    kPlank = 17,
    kRope = 18,
    kNails = 19,
    kBronzeHammer = 20,
    kBronzePick = 21,
    kBridge = 22,
    kIronPick = 23,
    kGoldBar = 24,
    kGemRing = 25,
    kEmpty = 26,
};

enum class Subgoal {
    kCollectTin = 0,
    kCollectCopper = 1,
    kCollectWood = 2,
    kCollectGrass = 3,
    kCollectIron = 4,
    kCollectGold = 5,
    kCollectGem = 6,
    kUseStation1 = 7,
    kUseStation2 = 8,
    kUseStation3 = 9,
    kUseFurnace = 10,
};

const std::vector<std::size_t> all_subgoals{
    static_cast<std::size_t>(Subgoal::kCollectTin),  static_cast<std::size_t>(Subgoal::kCollectCopper),
    static_cast<std::size_t>(Subgoal::kCollectWood), static_cast<std::size_t>(Subgoal::kCollectGrass),
    static_cast<std::size_t>(Subgoal::kCollectIron), static_cast<std::size_t>(Subgoal::kCollectGold),
    static_cast<std::size_t>(Subgoal::kCollectGem),  static_cast<std::size_t>(Subgoal::kUseStation1),
    static_cast<std::size_t>(Subgoal::kUseStation2), static_cast<std::size_t>(Subgoal::kUseStation3),
    static_cast<std::size_t>(Subgoal::kUseFurnace),
};

constexpr int kNumElements = 27;
constexpr int kPrimitiveStart = 8;
constexpr int kRecipeStart = 15;

enum class RecipeType {
    kBronzeBar = 0,
    kStick = 1,
    kPlank = 2,
    kRope = 3,
    kNails = 4,
    kBronzeHammer = 5,
    kBronzePick = 6,
    kBridge = 7,
    kIronPick = 8,
    kGoldBar = 9,
    kGemRing = 10,
};

enum class RewardCode : uint64_t {
    kRewardCodeCraftBronzeBar = 1 << 0,
    kRewardCodeCraftStick = 1 << 1,
    kRewardCodeCraftPlank = 1 << 2,
    kRewardCodeCraftRope = 1 << 3,
    kRewardCodeCraftNails = 1 << 4,
    kRewardCodeCraftBronzeHammer = 1 << 5,
    kRewardCodeCraftBronzePick = 1 << 6,
    kRewardCodeCraftIronPick = 1 << 7,
    kRewardCodeCraftBridge = 1 << 8,
    kRewardCodeCraftGoldBar = 1 << 9,
    kRewardCodeCraftGemRing = 1 << 10,
    kRewardCodeUseAxe = 1 << 11,
    kRewardCodeUseBridge = 1 << 12,
    kRewardCodeCollectTin = 1 << 13,
    kRewardCodeCollectCopper = 1 << 14,
    kRewardCodeCollectWood = 1 << 15,
    kRewardCodeCollectGrass = 1 << 16,
    kRewardCodeCollectIron = 1 << 17,
    kRewardCodeCollectGold = 1 << 18,
    kRewardCodeCollectGem = 1 << 19,
    kRewardCodeUseAtWorkstation1 = 1 << 20,
    kRewardCodeUseAtWorkstation2 = 1 << 21,
    kRewardCodeUseAtWorkstation3 = 1 << 22,
    kRewardCodeUseAtFurnace = 1 << 23,
};

constexpr std::size_t kNumRecipeTypes = 11;
constexpr std::size_t kNumEnvironment = 8;
constexpr std::size_t kNumPrimitive = 7;
constexpr std::size_t kNumInventory = kNumPrimitive + kNumRecipeTypes;
// craft recipes (11)
constexpr std::size_t kNumGoals = kNumRecipeTypes;

// Channels
// Environment (7), Primitives (5), Inventory (11), Level Goal (11)
constexpr std::size_t kNumChannels = kNumEnvironment + kNumPrimitive + kNumInventory + kNumGoals;

struct RecipeInputItem {
    Element element;
    int count;
};

struct RecipeItem {
    RecipeType recipe;
    std::vector<RecipeInputItem> inputs;
    Element location;
    Element output;
};

const RecipeItem kRecipeStick{
    RecipeType::kStick,
    {{Element::kWood, 1}},
    Element::kWorkshop1,
    Element::kStick,
};
const RecipeItem kRecipePlank{
    RecipeType::kPlank,
    {{Element::kWood, 1}},
    Element::kWorkshop3,
    Element::kPlank,
};
const RecipeItem kRecipeBronzeBar{
    RecipeType::kBronzeBar,
    {{Element::kCopper, 1}, {Element::kTin, 1}},
    Element::kFurnace,
    Element::kBronzeBar,
};
const RecipeItem kRecipeNails{
    RecipeType::kNails,
    {{Element::kBronzeBar, 1}},
    Element::kWorkshop1,
    Element::kNails,
};
const RecipeItem kRecipeBronzeHammer{
    RecipeType::kBronzeHammer,
    {{Element::kBronzeBar, 1}, {Element::kStick, 1}},
    Element::kWorkshop2,
    Element::kBronzeHammer,
};
const RecipeItem kRecipeBronzePick{
    RecipeType::kBronzePick,
    {{Element::kBronzeBar, 1}, {Element::kStick, 1}},
    Element::kWorkshop3,
    Element::kBronzePick,
};
const RecipeItem kRecipeBridge{
    RecipeType::kBridge,
    {{Element::kPlank, 1}, {Element::kNails, 1}, {Element::kBronzeHammer, 1}},
    Element::kWorkshop1,
    Element::kBridge,
};
const RecipeItem kRecipeIronPick{
    RecipeType::kIronPick,
    {{Element::kIron, 1}, {Element::kStick, 1}},
    Element::kWorkshop3,
    Element::kIronPick,
};
const RecipeItem kRecipeRope{
    RecipeType::kRope,
    {{Element::kGrass, 1}},
    Element::kWorkshop2,
    Element::kRope,
};

const RecipeItem kRecipeGoldBar{
    RecipeType::kGoldBar,
    {{Element::kGold, 1}},
    Element::kWorkshop1,
    Element::kGoldBar,
};
const RecipeItem kRecipeGemRing{
    RecipeType::kGemRing,
    {{Element::kGem, 1}},
    Element::kWorkshop2,
    Element::kGemRing,
};

const std::unordered_map<RecipeType, RecipeItem> kRecipeMap{
    {RecipeType::kStick, kRecipeStick},
    {RecipeType::kPlank, kRecipePlank},
    {RecipeType::kBronzeBar, kRecipeBronzeBar},
    {RecipeType::kNails, kRecipeNails},
    {RecipeType::kBronzeHammer, kRecipeBronzeHammer},
    {RecipeType::kBronzePick, kRecipeBronzePick},
    {RecipeType::kBridge, kRecipeBridge},
    {RecipeType::kIronPick, kRecipeIronPick},
    // {RecipeType::kRope, kRecipeRope},
    {RecipeType::kGoldBar, kRecipeGoldBar},
    {RecipeType::kGemRing, kRecipeGemRing},
};

const std::unordered_map<Element, RewardCode> kPrimitiveRewardMap{
    {Element::kTin, RewardCode::kRewardCodeCollectTin},   {Element::kCopper, RewardCode::kRewardCodeCollectCopper},
    {Element::kWood, RewardCode::kRewardCodeCollectWood}, {Element::kGrass, RewardCode::kRewardCodeCollectGrass},
    {Element::kIron, RewardCode::kRewardCodeCollectIron}, {Element::kGold, RewardCode::kRewardCodeCollectGold},
    {Element::kGem, RewardCode::kRewardCodeCollectGem},
};

const std::unordered_map<RecipeType, RewardCode> kRecipeRewardMap{
    {RecipeType::kStick, RewardCode::kRewardCodeCraftStick},
    {RecipeType::kPlank, RewardCode::kRewardCodeCraftPlank},
    {RecipeType::kBronzeBar, RewardCode::kRewardCodeCraftBronzeBar},
    {RecipeType::kNails, RewardCode::kRewardCodeCraftNails},
    {RecipeType::kBronzeHammer, RewardCode::kRewardCodeCraftBronzeHammer},
    {RecipeType::kBronzePick, RewardCode::kRewardCodeCraftBronzeHammer},
    {RecipeType::kBridge, RewardCode::kRewardCodeCraftBridge},
    {RecipeType::kIronPick, RewardCode::kRewardCodeCraftIronPick},
    // {RecipeType::kRope, RewardCode::kRewardCodeCraftRope},
    {RecipeType::kGoldBar, RewardCode::kRewardCodeCraftGoldBar},
    {RecipeType::kGemRing, RewardCode::kRewardCodeCraftGemRing},
};
const std::unordered_map<Element, RewardCode> kWorkstationRewardMap{
    {Element::kWorkshop1, RewardCode::kRewardCodeUseAtWorkstation1},
    {Element::kWorkshop2, RewardCode::kRewardCodeUseAtWorkstation2},
    {Element::kWorkshop3, RewardCode::kRewardCodeUseAtWorkstation3},
    {Element::kFurnace, RewardCode::kRewardCodeUseAtFurnace},
};

const std::unordered_map<Element, std::string> kElementToSymbolMap{
    {Element::kAgent, "@"},    // Env
    {Element::kWall, "#"},      {Element::kWorkshop1, "1"}, {Element::kWorkshop2, "2"},
    {Element::kWorkshop3, "3"}, {Element::kFurnace, "F"},   {Element::kWater, "~"},
    {Element::kStone, "o"},     {Element::kIron, "i"},      {Element::kTin, "T"},    // Primitives
    {Element::kCopper, "c"},    {Element::kGrass, "g"},     {Element::kWood, "w"},
    {Element::kGold, "."},      {Element::kGem, "*"},       {Element::kEmpty, " "},
};
const std::unordered_map<Element, std::string> kElementToNameMap{
    {Element::kTin, "Tin"},
    {Element::kCopper, "Copper"},
    {Element::kIron, "Iron"},
    {Element::kGrass, "Grass"},
    {Element::kWood, "Wood"},
    {Element::kGold, "Gold"},
    {Element::kGem, "Gem"},
    {Element::kStick, "Stick"},
    {Element::kPlank, "Plank"},
    {Element::kBronzeBar, "BronzeBar"},
    {Element::kRope, "Rope"},
    {Element::kNails, "Nails"},
    {Element::kBronzeHammer, "BronzeHammer"},
    {Element::kBronzePick, "BronzePick"},
    {Element::kIronPick, "IronPick"},
    {Element::kBridge, "Bridge"},
    {Element::kGoldBar, "GoldBar"},
    {Element::kGemRing, "GemRing"},
};
const std::unordered_set<Element> kWorkShops{
    Element::kWorkshop1,
    Element::kWorkshop2,
    Element::kWorkshop3,
    Element::kFurnace,
};
const std::unordered_set<Element> kPrimitives{
    Element::kGrass, Element::kWood, Element::kGold, Element::kGem, Element::kCopper, Element::kTin,
};

const std::unordered_map<Element, Element> kWorkshopSwap{
    {Element::kWorkshop1, Element::kWorkshop2},
    {Element::kWorkshop2, Element::kWorkshop3},
    {Element::kWorkshop3, Element::kWorkshop1},
};

// Directions the interactions take place
enum class Action {
    kUp = 0,
    kRight = 1,
    kDown = 2,
    kLeft = 3,
    kUse = 4,
};
// Agent can only take a subset of all directions
constexpr int kNumDirections = 4;
constexpr int kNumActions = kNumDirections + 1;

// actions to strings
const std::unordered_map<Action, std::string> kActionToString{
    {Action::kUp, "up"},       {Action::kLeft, "left"}, {Action::kDown, "down"},
    {Action::kRight, "right"}, {Action::kUse, "use"},
};

// directions to offsets (col, row)
const std::array<std::pair<int, int>, 5> kDirectionOffsets{{
    {0, -1},
    {1, 0},
    {0, 1},
    {-1, 0},
    {0, 0},
}};

struct Board {
    Board() = delete;
    Board(std::size_t rows, std::size_t cols, Element goal)
        : rows(rows), cols(cols), goal(goal), grid(rows * cols, Element::kEmpty) {}

    bool operator==(const Board &other) const {
        return grid == other.grid;
    }

    Element &item(std::size_t index) {
        assert(index < rows * cols);
        return grid[index];
    }

    [[nodiscard]] Element item(std::size_t index) const {
        assert(index < rows * cols);
        return grid[index];
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    uint64_t zorb_hash = 0;
    std::size_t rows;
    std::size_t cols;
    Element goal;
    std::size_t agent_idx{};
    std::vector<Element> grid;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}    // namespace craftworld

#endif    // CRAFTWORLD_DEFS_H_
