#ifndef CRAFTWORLD_DEFS_H_
#define CRAFTWORLD_DEFS_H_

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace craftworld {

enum class ElementType {
    kAgent = 0,    // Env
    kWall = 1,
    kWorkshop0 = 2,
    kWorkshop1 = 3,
    kWorkshop2 = 4,
    kWater = 5,
    kStone = 6,
    kIron = 7,    // Primitives
    kGrass = 8,
    kWood = 9,
    kGold = 10,
    kGem = 11,
    kPlank = 12,    // Recipes
    kAxe = 13,
    kRope = 14,
    kStick = 15,
    kBed = 16,
    kShears = 17,
    kCloth = 18,
    kBridge = 19,
    kLadder = 20,
    kGoldBar = 21,
    kGemRing = 22,
    kEmpty = 23,
};

// enum class Subgoals {
//     kCraftPlank = 0,
//     kCraftAxe = 1,
//     kCraftRope = 2,
//     kCraftStick = 3,
//     kCraftBed = 4,
//     kCraftShears = 5,
//     kCraftCloth = 6,
//     kCraftBridge = 7,
//     kCraftLadder = 8,
//     kCraftGoldBar = 9,
//     kCraftGemRing = 10,
//     kUseAxe = 11,
//     kUseBridge = 12,
// };
enum class Subgoals {
    kCollectIron = 0,
    kCollectGrass = 1,
    kCollectWood = 2,
    kCollectGold = 3,
    kCollectGem = 4,
    kUseStation1 = 5,
    kUseStation2 = 6,
    kUseStation3 = 7,
};

// const std::vector<int> all_subgoals{
//     static_cast<int>(Subgoals::kCraftPlank),   static_cast<int>(Subgoals::kCraftAxe),
//     static_cast<int>(Subgoals::kCraftRope),    static_cast<int>(Subgoals::kCraftStick),
//     static_cast<int>(Subgoals::kCraftBed),     static_cast<int>(Subgoals::kCraftShears),
//     static_cast<int>(Subgoals::kCraftCloth),   static_cast<int>(Subgoals::kCraftBridge),
//     static_cast<int>(Subgoals::kCraftLadder),  static_cast<int>(Subgoals::kCraftGoldBar),
//     static_cast<int>(Subgoals::kCraftGemRing), 
//     static_cast<int>(Subgoals::kUseAxe),
//     static_cast<int>(Subgoals::kUseBridge),
// };
const std::vector<int> all_subgoals{
    static_cast<int>(Subgoals::kCollectIron),   static_cast<int>(Subgoals::kCollectGrass),
    static_cast<int>(Subgoals::kCollectWood),    static_cast<int>(Subgoals::kCollectGold),
    static_cast<int>(Subgoals::kCollectGem),     static_cast<int>(Subgoals::kUseStation1),
    static_cast<int>(Subgoals::kUseStation2),   static_cast<int>(Subgoals::kUseStation3),
};

constexpr int kNumElementTypes = 24;
constexpr int kPrimitiveStart = 7;
constexpr int kRecipeStart = 12;

enum class RecipeType {
    kPlank = 0,
    kAxe = 1,
    kRope = 2,
    kStick = 3,
    kBed = 4,
    kShears = 5,
    kCloth = 6,
    kBridge = 7,
    kLadder = 8,
    kGoldBar = 9,
    kGemRing = 10,
};

enum class RewardCodes : uint64_t {
    kRewardCodeCraftPlank = 1 << 0,
    kRewardCodeCraftAxe = 1 << 1,
    kRewardCodeCraftRope = 1 << 2,
    kRewardCodeCraftStick = 1 << 3,
    kRewardCodeCraftBed = 1 << 4,
    kRewardCodeCraftShears = 1 << 5,
    kRewardCodeCraftCloth = 1 << 6,
    kRewardCodeCraftBridge = 1 << 7,
    kRewardCodeCraftLadder = 1 << 8,
    kRewardCodeCraftGoldBar = 1 << 9,
    kRewardCodeCraftGemRing = 1 << 10,
    kRewardCodeUseAxe = 1 << 11,
    kRewardCodeUseBridge = 1 << 12,
    kRewardCodeCollectIron = 1 << 13,
    kRewardCodeCollectGrass = 1 << 14,
    kRewardCodeCollectWood = 1 << 15,
    kRewardCodeCollectGold = 1 << 16,
    kRewardCodeCollectGem = 1 << 17,
    kRewardCodeUseAtWorkstation1 = 1 << 18,
    kRewardCodeUseAtWorkstation2 = 1 << 19,
    kRewardCodeUseAtWorkstation3 = 1 << 20,
};

constexpr int kNumRecipeTypes = 11;
constexpr int kNumEnvironment = 7;
constexpr int kNumPrimitive = 5;
constexpr int kNumInventory = kNumPrimitive + kNumRecipeTypes;
// craft recipes (11)
constexpr int kNumGoals = kNumRecipeTypes;

// Channels
// Environment (7), Primitives (5), Inventory (11), Level Goal (11)
constexpr int kNumChannels = kNumEnvironment + kNumPrimitive + kNumInventory + kNumGoals;

struct RecipeInputItem {
    ElementType element;
    int count;
};

struct RecipeItem {
    RecipeType recipe;
    std::vector<RecipeInputItem> inputs;
    ElementType location;
    ElementType output;
};

const RecipeItem kRecipePlank{
    RecipeType::kPlank,
    {{ElementType::kWood, 1}},
    ElementType::kWorkshop0,
    ElementType::kPlank,
};
const RecipeItem kRecipeAxe{
    RecipeType::kAxe,
    {{ElementType::kStick, 1}, {ElementType::kIron, 1}},
    ElementType::kWorkshop0,
    ElementType::kAxe,
};
const RecipeItem kRecipeRope{
    RecipeType::kRope,
    {{ElementType::kGrass, 1}},
    ElementType::kWorkshop1,
    ElementType::kRope,
};
const RecipeItem kRecipeStick{
    RecipeType::kStick,
    {{ElementType::kWood, 1}},
    ElementType::kWorkshop1,
    ElementType::kStick,
};
const RecipeItem kRecipeBed{
    RecipeType::kBed,
    {{ElementType::kPlank, 1}, {ElementType::kGrass, 1}},
    ElementType::kWorkshop1,
    ElementType::kBed,
};
const RecipeItem kRecipeShears{
    RecipeType::kShears,
    {{ElementType::kStick, 1}, {ElementType::kIron, 1}},
    ElementType::kWorkshop1,
    ElementType::kShears,
};
const RecipeItem kRecipeCloth{
    RecipeType::kCloth,
    {{ElementType::kGrass, 1}},
    ElementType::kWorkshop2,
    ElementType::kCloth,
};
const RecipeItem kRecipeBridge{
    RecipeType::kBridge,
    {{ElementType::kWood, 1}, {ElementType::kIron, 1}},
    ElementType::kWorkshop2,
    ElementType::kBridge,
};
const RecipeItem kRecipeLadder{
    RecipeType::kLadder,
    {{ElementType::kPlank, 1}, {ElementType::kStick, 1}},
    ElementType::kWorkshop2,
    ElementType::kLadder,
};
const RecipeItem kRecipeGoldBar{
    RecipeType::kGoldBar,
    {{ElementType::kGold, 1}},
    ElementType::kWorkshop0,
    ElementType::kGoldBar,
};
const RecipeItem kRecipeGemRing{
    RecipeType::kGemRing,
    {{ElementType::kGem, 1}},
    ElementType::kWorkshop1,
    ElementType::kGemRing,
};

const std::unordered_map<RecipeType, RecipeItem> kRecipeMap{
    {RecipeType::kPlank, kRecipePlank},     {RecipeType::kAxe, kRecipeAxe},
    {RecipeType::kRope, kRecipeRope},       {RecipeType::kStick, kRecipeStick},
    {RecipeType::kBed, kRecipeBed},         {RecipeType::kShears, kRecipeShears},
    {RecipeType::kCloth, kRecipeCloth},     {RecipeType::kBridge, kRecipeBridge},
    {RecipeType::kLadder, kRecipeLadder},   {RecipeType::kGoldBar, kRecipeGoldBar},
    {RecipeType::kGemRing, kRecipeGemRing},
};

const std::unordered_map<ElementType, RewardCodes> kPrimitiveRewardMap{
    {ElementType::kIron, RewardCodes::kRewardCodeCollectIron},
    {ElementType::kGrass, RewardCodes::kRewardCodeCollectGrass},
    {ElementType::kWood, RewardCodes::kRewardCodeCollectWood},
    {ElementType::kGold, RewardCodes::kRewardCodeCollectGold},
    {ElementType::kGem, RewardCodes::kRewardCodeCollectGem},
};

const std::unordered_map<RecipeType, RewardCodes> kRecipeRewardMap{
    {RecipeType::kPlank, RewardCodes::kRewardCodeCraftPlank},
    {RecipeType::kAxe, RewardCodes::kRewardCodeCraftAxe},
    {RecipeType::kRope, RewardCodes::kRewardCodeCraftRope},
    {RecipeType::kStick, RewardCodes::kRewardCodeCraftStick},
    {RecipeType::kBed, RewardCodes::kRewardCodeCraftBed},
    {RecipeType::kShears, RewardCodes::kRewardCodeCraftShears},
    {RecipeType::kCloth, RewardCodes::kRewardCodeCraftCloth},
    {RecipeType::kBridge, RewardCodes::kRewardCodeCraftBridge},
    {RecipeType::kLadder, RewardCodes::kRewardCodeCraftLadder},
    {RecipeType::kGoldBar, RewardCodes::kRewardCodeCraftGoldBar},
    {RecipeType::kGemRing, RewardCodes::kRewardCodeCraftGemRing},
};
const std::unordered_map<ElementType, RewardCodes> kWorkstationRewardMap{
    {ElementType::kWorkshop0, RewardCodes::kRewardCodeUseAtWorkstation1},
    {ElementType::kWorkshop1, RewardCodes::kRewardCodeUseAtWorkstation2},
    {ElementType::kWorkshop2, RewardCodes::kRewardCodeUseAtWorkstation3},
};

const std::unordered_map<ElementType, std::string> kElementToSymbolMap{
    {ElementType::kAgent, "@"},    // Env
    {ElementType::kWall, "#"},      {ElementType::kWorkshop0, "1"}, {ElementType::kWorkshop1, "2"},
    {ElementType::kWorkshop2, "3"}, {ElementType::kWater, "~"},     {ElementType::kStone, "o"},
    {ElementType::kIron, "i"},    // Primitives
    {ElementType::kGrass, "g"},     {ElementType::kWood, "w"},      {ElementType::kGold, "."},
    {ElementType::kGem, "*"},       {ElementType::kEmpty, " "},
};
const std::unordered_map<ElementType, std::string> kElementToNameMap{
    {ElementType::kIron, "Iron"},       {ElementType::kGrass, "Grass"},   {ElementType::kWood, "Wood"},
    {ElementType::kGold, "Gold"},       {ElementType::kGem, "Gem"},       {ElementType::kPlank, "Plank"},
    {ElementType::kAxe, "Axe"},         {ElementType::kRope, "Rope"},     {ElementType::kStick, "Stick"},
    {ElementType::kBed, "Bed"},         {ElementType::kShears, "Shears"}, {ElementType::kCloth, "Cloth"},
    {ElementType::kBridge, "Bridge"},   {ElementType::kLadder, "Ladder"}, {ElementType::kGoldBar, "GoldBar"},
    {ElementType::kGemRing, "GemRing"},
};
const std::unordered_set<ElementType> kWorkShops{
    ElementType::kWorkshop0,
    ElementType::kWorkshop1,
    ElementType::kWorkshop2,
};
const std::unordered_set<ElementType> kPrimitives{
    ElementType::kIron, ElementType::kGrass, ElementType::kWood, ElementType::kGold, ElementType::kGem,
};

// Directions the interactions take place
enum Actions {
    kNoop = -1,
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
const std::unordered_map<int, std::string> kActionsToString{
    {Actions::kUp, "up"},       {Actions::kLeft, "left"}, {Actions::kDown, "down"},
    {Actions::kRight, "right"}, {Actions::kUse, "use"},
};

// directions to offsets (col, row)
const std::unordered_map<int, std::pair<int, int>> kDirectionOffsets{
    {Actions::kUp, {0, -1}},
    {Actions::kLeft, {-1, 0}},
    {Actions::kDown, {0, 1}},
    {Actions::kRight, {1, 0}},
};

struct Board {
    Board() = delete;
    Board(int rows, int cols, int goal)
        : zorb_hash(0), rows(rows), cols(cols), goal(goal), agent_idx(-1), grid(rows * cols, 0) {}

    bool operator==(const Board &other) const {
        return grid == other.grid;
    }

    char &item(int index) {
        assert(index < rows * cols);
        return grid[index];
    }

    char item(int index) const {
        assert(index < rows * cols);
        return grid[index];
    }

    uint64_t zorb_hash = 0;
    int rows = 0;
    int cols = 0;
    int goal = -1;
    int agent_idx = -1;
    std::vector<char> grid;
};

}    // namespace craftworld

#endif    // CRAFTWORLD_DEFS_H_