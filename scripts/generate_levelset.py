import argparse
import copy
import os
import random
import sys
from multiprocessing import Manager, Pool

import numpy as np

kAgent = 0  # Env
kWall = 1
kWorkshop1 = 2
kWorkshop2 = 3
kWorkshop3 = 4
kFurnace = 5
kWater = 6
kStone = 7
kIron = 8
kTin = 9  # Primitives
kCopper = 10
kWood = 11
kGrass = 12
kGold = 13
kGem = 14
kBronzeBar = 15  # Recipes
kStick = 16
kPlank = 17
kRope = 18
kNails = 19
kBronzeHammer = 20
kBronzePick = 21
kBridge = 22
kIronPick = 23
kGoldBar = 24
kGemRing = 25
kEmpty = 26

PRIMITIVES = [kGrass, kWood]
RECIPES = {
    # kStick: [(kWood, 1)],
    # kPlank: [(kWood, 1)],
    # kBronzeBar: [(kCopper, 1), (kTin, 1)],
    # kNails: [(kCopper, 1), (kTin, 1)],
    # kBronzeHammer: [(kCopper, 1), (kTin, 1), (kWood, 1)],
    kBronzePick: [(kCopper, 1), (kTin, 1), (kWood, 1)],
    kIronPick: [(kIron, 1), (kWood, 2), (kCopper, 1), (kTin, 1)],
    # kRope: [(kGrass, 1)],
    # kBridge: [(kWood, 2), (kCopper, 2), (kTin, 2)],
    # kGoldBar: [(kWood, 2), (kCopper, 2), (kTin, 2)],
    kGemRing: [(kIron, 1), (kWood, 2), (kCopper, 1), (kTin, 1)],
}
RECIPE_PROBS_TRAIN = [0.2, 0.3, 0.5]
RECIPE_PROBS_HARD = [0.0, 0.05, 0.95]
RECIPE_PROBS_TEST = [0.0, 0.0, 1.0]
WORKSHOPS = [kWorkshop1, kWorkshop2, kWorkshop3, kFurnace]
N_RANDOM_PRIMITIVES = 0
N_RANDOM_GRASS = 0


# Get a random empty location in the map
def random_free(grid, rng, blocked_tiles):
    width, height = grid.shape
    empty_coords = [
        (r, c)
        for r in range(height)
        for c in range(width)
        if grid[r, c] == kEmpty and (r, c) not in blocked_tiles
    ]
    return empty_coords[rng.choice(len(empty_coords))]


def in_bounds(r, c, width, height):
    return r >= 0 and c >= 0 and r < width and c < height


def is_beside_workshop(grid, r, c):
    width, height = grid.shape
    for dr in range(-1, 2):
        for dc in range(-1, 2):
            new_r = r + dr
            new_c = c + dc
            if (
                in_bounds(new_r, new_c, width, height)
                and grid[new_r, new_c] in WORKSHOPS
            ):
                return True
    return False


def is_empty_around(grid, r, c):
    width, height = grid.shape
    for dr in range(-1, 2):
        for dc in range(-1, 2):
            new_r = r + dr
            new_c = c + dc
            # If in bounds AND blocked, skip
            if in_bounds(new_r, new_c, width, height) and (
                grid[new_r, new_c] != kEmpty or is_beside_workshop(grid, new_r, new_c)
            ):
                return False

    return True


def random_free_extra_space(grid, rng, blocked_tiles):
    width, height = grid.shape
    empty_coords = []
    for r in range(height):
        for c in range(width):
            if (r, c) in blocked_tiles or grid[r, c] != kEmpty:
                continue
            if is_empty_around(grid, r, c):
                empty_coords.append((r, c))
    if len(empty_coords) == 0:
        raise ValueError
    return empty_coords[rng.choice(len(empty_coords))]


def random_teasure_location(grid, rng):
    width, height = grid.shape
    empty_coords = [
        (r, c)
        for r in range(1, height - 1)
        for c in range(1, width - 1)
        if grid[r, c] == kEmpty
    ]
    if len(empty_coords) == 0:
        raise ValueError
    return empty_coords[rng.choice(len(empty_coords))]


def create_map(args):
    manager_dict, map_size, seed, recipe_probs = args
    rng = np.random.default_rng(seed)
    grid = np.array([kEmpty] * (map_size * map_size), dtype=int).reshape(
        map_size, map_size
    )

    # Sample goal for the map
    # goal = rng.choice(list(RECIPES.keys()))
    goal = rng.choice(list(RECIPES.keys()), p=recipe_probs)
    ingredients = RECIPES[goal]
    make_island = goal == kGoldBar
    make_cave = goal == kGemRing
    blocked_tiles = []

    # Treasure (island or cave)
    if make_island or make_cave:
        treasure_type = kGold if make_island else kGem
        wall_type = kWater if make_island else kStone
        treasure_location = random_teasure_location(grid, rng)
        grid[treasure_location] = treasure_type
        wall_locations = [
            (treasure_location[0] - 1, treasure_location[1]),
            (treasure_location[0] + 1, treasure_location[1]),
            (treasure_location[0], treasure_location[1] - 1),
            (treasure_location[0], treasure_location[1] + 1),
        ]
        for wall_location in wall_locations:
            grid[wall_location] = wall_type

    # Ingredients required for the goal
    for ingredient in ingredients:
        for i in range(ingredient[1]):
            grid[random_free_extra_space(grid, rng, blocked_tiles)] = ingredient[0]

    # If make_island, we need a bridge
    # if make_island:
    #     for ingredient in RECIPES[kBridge]:
    #         for i in range(ingredient[1]):
    #             grid[random_free_extra_space(grid, rng, blocked_tiles)] = ingredient[0]

    # If make_cave, we need an axe
    # if make_cave:
    #     for ingredient in RECIPES[kAxe]:
    #         for i in range(ingredient[1]):
    #             grid[random_free_extra_space(grid, rng, blocked_tiles)] = ingredient[0]

    # Other random ingredients to confuse
    for _ in range(N_RANDOM_PRIMITIVES):
        grid[random_free_extra_space(grid, rng, blocked_tiles)] = rng.choice(PRIMITIVES)

    # Crafting stations
    for workshop in WORKSHOPS:
        grid[random_free_extra_space(grid, rng, blocked_tiles)] = workshop

    # Place agent
    grid[random_free_extra_space(grid, rng, blocked_tiles)] = kAgent

    # Random grass for complexity
    for _ in range(N_RANDOM_GRASS):
        grid[random_free_extra_space(grid, rng, blocked_tiles)] = kGrass

    # Set map str and store
    goal_str = "{}|{}|{}".format(map_size, map_size, goal)
    for i in grid.flatten().tolist():
        goal_str += "|{:02}".format(i)
    manager_dict[seed] = goal_str


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--num_train",
        help="Number of maps in train set",
        required=False,
        type=int,
        default=10000,
    )
    parser.add_argument(
        "--num_test",
        help="Number of maps in test set",
        required=False,
        type=int,
        default=1000,
    )
    parser.add_argument(
        "--map_size",
        help="Size of map width/height",
        required=False,
        type=int,
        default=10,
    )
    parser.add_argument(
        "--export_path", help="Export path for file", required=True, type=str
    )
    parser.add_argument(
        "--num_primitive",
        help="Number of random environment elements to add",
        required=False,
        type=int,
        default=0,
    )
    parser.add_argument(
        "--num_grass",
        help="Number of random grass elements to add",
        required=False,
        type=int,
        default=0,
    )
    parser.add_argument(
        "--hard",
        help="Only hard problems",
        required=False,
        type=bool,
        default=False,
    )
    args = parser.parse_args()
    global N_RANDOM_PRIMITIVES
    N_RANDOM_PRIMITIVES = args.num_primitive
    global N_RANDOM_GRASS
    N_RANDOM_GRASS = args.num_grass

    manager = Manager()
    data = manager.dict()
    with Pool(32) as p:
        p.map(
            create_map,
            [
                (
                    data,
                    args.map_size,
                    i,
                    RECIPE_PROBS_TRAIN if not args.hard else RECIPE_PROBS_HARD,
                )
                for i in range(args.num_train)
            ],
        )
    with Pool(32) as p:
        p.map(
            create_map,
            [
                (data, args.map_size, i, RECIPE_PROBS_TEST)
                for i in range(args.num_train, args.num_train + args.num_test)
            ],
        )

    # Parse and save to file
    if not os.path.exists(args.export_path):
        os.makedirs(args.export_path)

    export_train_file = os.path.join(args.export_path, "train.txt")
    export_test_file = os.path.join(args.export_path, "test.txt")

    map_idx = 0
    with open(export_train_file, "w") as file:
        for _ in range(args.num_train):
            file.write(data[map_idx])
            map_idx += 1
            file.write("\n")

    with open(export_test_file, "w") as file:
        for _ in range(args.num_test):
            file.write(data[map_idx])
            map_idx += 1
            file.write("\n")


if __name__ == "__main__":
    main()
