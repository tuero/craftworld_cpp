import os
import sys
import copy
import random
import numpy as np
from multiprocessing import Pool, Manager
import argparse

kAgent = 0    # Env
kWall = 1
kWorkshop0 = 2
kWorkshop1 = 3
kWorkshop2 = 4
kWater = 5
kStone = 6
kIron = 7    # Primitives
kGrass = 8
kWood = 9
kGold = 10
kGem = 11
kPlank = 12    # Recipes
kAxe = 13
kRope = 14
kStick = 15
kBed = 16
kShears = 17
kCloth = 18
kBridge = 19
kLadder = 20
kGoldBar = 21
kGemRing = 22
kEmpty = 23

PRIMITIVES = [kIron, kGrass, kWood]
RECIPES = {
    kPlank : [(kWood, 1)], 
    kAxe : [(kWood, 1), (kIron, 1)], 
    kRope : [(kGrass, 1)], 
    kStick : [(kWood, 1)], 
    kBed : [(kWood, 1), (kGrass, 1)], 
    kShears : [(kWood, 1), (kIron, 1)], 
    kCloth : [(kGrass, 1)], 
    kBridge : [(kWood, 1), (kIron, 1)], 
    kLadder : [(kWood, 1), (kWood, 1)],
    kGoldBar : [],
    kGemRing : [],
}
WORKSHOPS = [kWorkshop0, kWorkshop1, kWorkshop2]
N_RANDOM_PRIMITIVES = 2

# Get a random empty location in the map
def random_free(grid, rng, blocked_tiles):
    width, height = grid.shape
    empty_coords = [(r, c) for r in range(height) for c in range(width) if grid[r, c] == kEmpty and (r, c) not in blocked_tiles]
    return empty_coords[rng.choice(len(empty_coords))]


def in_bounds(r, c, width, height):
    return r >= 0 and c >= 0 and r < width and c < height

def is_beside_workshop(grid, r, c):
    width, height = grid.shape
    for dr in range(-1, 2):
        for dc in range(-1, 2):
            new_r = r + dr
            new_c = c + dc 
            if in_bounds(new_r, new_c, width, height) and grid[new_r, new_c] in WORKSHOPS:
                return True 
    return False


def is_empty_around(grid, r, c):
    width, height = grid.shape
    for dr in range(-1, 2):
        for dc in range(-1, 2):
            new_r = r + dr
            new_c = c + dc 
            # If in bounds AND blocked, skip
            if in_bounds(new_r, new_c, width, height) and (grid[new_r, new_c] != kEmpty or is_beside_workshop(grid, new_r, new_c)):
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
    return empty_coords[rng.choice(len(empty_coords))]

def random_teasure_location(grid, rng):
    width, height = grid.shape
    empty_coords = [(r, c) for r in range(1, height - 1) for c in range(1, width - 1) if grid[r, c] == kEmpty]
    return empty_coords[rng.choice(len(empty_coords))]

def create_map(args):
    manager_dict, map_size, seed = args
    rng = np.random.default_rng(seed)
    grid = np.array([kEmpty]*(map_size*map_size), dtype=int).reshape(map_size, map_size)

    # Sample goal for the map
    goal = rng.choice(list(RECIPES.keys()))
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
    if make_island:
        for ingredient in RECIPES[kBridge]:
            for i in range(ingredient[1]):
                grid[random_free_extra_space(grid, rng, blocked_tiles)] = ingredient[0]

    # If make_cave, we need an axe
    if make_cave:
        for ingredient in RECIPES[kAxe]:
            for i in range(ingredient[1]):
                grid[random_free_extra_space(grid, rng, blocked_tiles)] = ingredient[0]

    # Other random ingredients to confuse
    for _ in range(N_RANDOM_PRIMITIVES):
        grid[random_free_extra_space(grid, rng, blocked_tiles)] = rng.choice(PRIMITIVES[:-2])

    # Crafting stations
    for workshop in WORKSHOPS:
        grid[random_free_extra_space(grid, rng, blocked_tiles)] = workshop

    # Place agent
    grid[random_free_extra_space(grid, rng, blocked_tiles)] = kAgent

    # Set map str and store
    goal_str = "{}|{}|{}".format(map_size, map_size, goal)
    for i in grid.flatten().tolist():
        goal_str += "|{:02}".format(i)
    manager_dict[seed] = goal_str
    

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--num_train", help="Number of maps in train set", required=False, type=int, default=10000)
    parser.add_argument("--num_test", help="Number of maps in test set", required=False, type=int, default=1000)
    parser.add_argument("--map_size", help="Size of map width/height", required=False, type=int, default=10)
    parser.add_argument("--export_path", help="Export path for file", required=True, type=str)
    args = parser.parse_args()

    manager = Manager()
    data = manager.dict()
    with Pool(32) as p:
        p.map(
            create_map,
            [(data, args.map_size, i) for i in range(args.num_train + args.num_test)],
        )

     # Parse and save to file
    if not os.path.exists(args.export_path):
        os.makedirs(args.export_path)

    export_train_file = os.path.join(args.export_path, "train.txt")
    export_test_file = os.path.join(args.export_path, "test.txt")

    map_idx = 0
    with open(export_train_file, "w") as file:
        for i in range(args.num_train):
            file.write(data[map_idx])
            map_idx += 1
            file.write("\n")

    with open(export_test_file, "w") as file:
        for i in range(args.num_test):
            file.write(data[map_idx])
            map_idx += 1
            file.write("\n")


if __name__ == "__main__":
    main()