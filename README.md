# craftworld_cpp

A modified C++ implementation of the Craftworld environment from [Modular Multitask Reinforcement Learning with Policy Sketches](https://arxiv.org/pdf/1611.01796.pdf).
Andreas, J., Klein, D. &amp; Levine, S.. (2017). Modular Multitask Reinforcement Learning with Policy Sketches. *Proceedings of the 34th International Conference on Machine Learning*, in *Proceedings of Machine Learning Research*.

## Include to Your Project: CMake

### FetchContent
```shell
include(FetchContent)
# ...
FetchContent_Declare(craftworld
    GIT_REPOSITORY https://github.com/tuero/craftworld_cpp.git
    GIT_TAG master
)

# make available
FetchContent_MakeAvailable(craftworld)
link_libraries(craftworld)
```

### Git Submodules
```shell
# assumes project is cloned into external/craftworld_cpp
add_subdirectory(external/craftworld_cpp)
link_libraries(craftworld)
```

## Generate Levels
The levelset generator will generate a curriculum of levels to gather the gem ring:
make a bronze pick, make an iron pick, and collect the gem ring.
```shell
cd scripts
python generate_levelset.py --export_path=EXPORT_PATH --map_size=14 --num_train=50000 --num_test=1000 --num_grass=2
```
