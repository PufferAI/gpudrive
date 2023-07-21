#include <madrona/viz/viewer.hpp>

#include "mgr.hpp"
#include "sim.hpp"

#include <filesystem>
#include <fstream>

using namespace madrona;
using namespace madrona::viz;

static inline float srgbToLinear(float srgb)
{
    if (srgb <= 0.04045f) {
        return srgb / 12.92f;
    }

    return powf((srgb + 0.055f) / 1.055f, 2.4f);
}

static inline math::Vector4 rgb8ToFloat(uint8_t r, uint8_t g, uint8_t b)
{
    return {
        srgbToLinear((float)r / 255.f),
        srgbToLinear((float)g / 255.f),
        srgbToLinear((float)b / 255.f),
        1.f,
    };
}

static HeapArray<int32_t> readReplayLog(const char *path)
{
    std::ifstream replay_log(path, std::ios::binary);
    replay_log.seekg(0, std::ios::end);
    int64_t size = replay_log.tellg();
    replay_log.seekg(0, std::ios::beg);

    HeapArray<int32_t> log(size / sizeof(int32_t));

    replay_log.read((char *)log.data(), (size / sizeof(int32_t)) * sizeof(int32_t));

    return log;
}

int main(int argc, char *argv[])
{
    using namespace GPUHideSeek;

    constexpr int64_t num_views = 2;

    uint32_t num_worlds = 1;
    if (argc >= 2) {
        num_worlds = (uint32_t)atoi(argv[1]);
    }

    ExecMode exec_mode = ExecMode::CPU;
    if (argc >= 3) {
        if (!strcmp("--cpu", argv[2])) {
            exec_mode = ExecMode::CPU;
        } else if (!strcmp("--cuda", argv[2])) {
            exec_mode = ExecMode::CUDA;
        }
    }

    const char *replay_log_path = nullptr;
    if (argc >= 4) {
        replay_log_path = argv[3];
    }

    auto replay_log = Optional<HeapArray<int32_t>>::none();
    uint32_t cur_replay_step = 0;
    uint32_t num_replay_steps = 0;
    if (replay_log_path != nullptr) {
        replay_log = readReplayLog(replay_log_path);
        num_replay_steps = replay_log->size() / (num_worlds * num_views * 3);
    }

    std::array<std::string, (size_t)SimObject::NumObjects> render_asset_paths;
    render_asset_paths[(size_t)SimObject::Cube] =
        (std::filesystem::path(DATA_DIR) / "cube_render.obj").string();
    render_asset_paths[(size_t)SimObject::Wall] =
        (std::filesystem::path(DATA_DIR) / "wall_render.obj").string();
    render_asset_paths[(size_t)SimObject::Door] =
        (std::filesystem::path(DATA_DIR) / "wall_render.obj").string();
    render_asset_paths[(size_t)SimObject::Agent] =
        (std::filesystem::path(DATA_DIR) / "agent_render.obj").string();
    render_asset_paths[(size_t)SimObject::Button] =
        (std::filesystem::path(DATA_DIR) / "cube_render.obj").string();
    render_asset_paths[(size_t)SimObject::Plane] =
        (std::filesystem::path(DATA_DIR) / "plane.obj").string();

    std::array<const char *, (size_t)SimObject::NumObjects> render_asset_cstrs;
    for (size_t i = 0; i < render_asset_paths.size(); i++) {
        render_asset_cstrs[i] = render_asset_paths[i].c_str();
    }

    std::array<char, 1024> import_err;
    auto render_assets = imp::ImportedAssets::importFromDisk(
        render_asset_cstrs, Span<char>(import_err.data(), import_err.size()));

    if (!render_assets.has_value()) {
        FATAL("Failed to load render assets: %s", import_err);
    }

    auto materials = std::to_array<imp::SourceMaterial>({
        { rgb8ToFloat(191, 108, 10), -1, 0.8f, 0.2f },
        { math::Vector4{0.4f, 0.4f, 0.4f, 0.0f}, -1, 0.8f, 0.2f,},
        { math::Vector4{1.f, 1.f, 1.f, 0.0f}, 1, 0.5f, 1.0f,},
        { rgb8ToFloat(230, 230, 230),   -1, 0.8f, 1.0f },
        { math::Vector4{0.5f, 0.3f, 0.3f, 0.0f},  0, 0.8f, 0.2f,},
        { rgb8ToFloat(230, 20, 20),   -1, 0.8f, 1.0f },
        { rgb8ToFloat(230, 230, 20),   -1, 0.8f, 1.0f },
    });

    Viewer viewer({
        .gpuID = 0,
        .renderWidth = 2730,
        .renderHeight = 1536,
        .numWorlds = num_worlds,
        .maxViewsPerWorld = num_views,
        .maxInstancesPerWorld = 1000,
        .defaultSimTickRate = 10,
        .execMode = exec_mode,
    });

    // Override materials
    render_assets->objects[(CountT)SimObject::Cube].meshes[0].materialIDX = 0;
    render_assets->objects[(CountT)SimObject::Wall].meshes[0].materialIDX = 1;
    render_assets->objects[(CountT)SimObject::Door].meshes[0].materialIDX = 5;
    render_assets->objects[(CountT)SimObject::Agent].meshes[0].materialIDX = 2;
    render_assets->objects[(CountT)SimObject::Agent].meshes[1].materialIDX = 3;
    render_assets->objects[(CountT)SimObject::Agent].meshes[2].materialIDX = 3;
    render_assets->objects[(CountT)SimObject::Button].meshes[0].materialIDX = 6;
    render_assets->objects[(CountT)SimObject::Plane].meshes[0].materialIDX = 4;

    viewer.loadObjects(render_assets->objects, materials, {
        { (std::filesystem::path(DATA_DIR) /
           "green_grid.png").string().c_str() },
        { (std::filesystem::path(DATA_DIR) /
           "smile.png").string().c_str() },
    });

    viewer.configureLighting({
        { true, math::Vector3{1.0f, 1.0f, -2.0f}, math::Vector3{1.0f, 1.0f, 1.0f} }
    });

    Manager mgr({
        .execMode = exec_mode,
        .gpuID = 0,
        .numWorlds = num_worlds,
        .autoReset = false,
    }, viewer.rendererBridge());

    auto replayStep = [&]() {
        if (cur_replay_step == num_replay_steps) {
            cur_replay_step = 0;
            for (uint32_t i = 0; i < num_worlds; i++) {
                mgr.triggerReset(i);
                mgr.step();
            }
        }

        printf("Step: %u\n", cur_replay_step);

        for (uint32_t i = 0; i < num_worlds; i++) {
            for (uint32_t j = 0; j < num_views; j++) {
                uint32_t base_idx = 0;
                base_idx = 3 * (cur_replay_step * num_views * num_worlds +
                    i * num_views + j);

                int32_t x = (*replay_log)[base_idx];
                int32_t y = (*replay_log)[base_idx + 1];
                int32_t r = (*replay_log)[base_idx + 2];

                mgr.setAction(i, j, x, y, r);
            }
        }

        cur_replay_step++;
    };

    auto pos_printer = mgr.positionObservationTensor().makePrinter();
    auto to_other_printer = mgr.toOtherAgentsTensor().makePrinter();
    auto to_dyn_printer = mgr.toDynEntitiesTensor().makePrinter();
    auto lidar_printer = mgr.lidarTensor().makePrinter();
    auto reward_printer = mgr.rewardTensor().makePrinter();

    auto printObs = [&]() {
        printf("Pos\n");
        pos_printer.print();

        printf("To Other\n");
        to_other_printer.print();

        printf("To Dyn Entities\n");
        to_dyn_printer.print();

        printf("Lidar\n");
        lidar_printer.print();

        printf("Reward\n");
        reward_printer.print();

        printf("\n");
    };

    viewer.loop([&mgr](CountT world_idx, CountT agent_idx,
                       const Viewer::UserInput &input) {
        using Key = Viewer::KeyboardKey;

        int32_t x = 2;
        int32_t y = 2;
        int32_t r = 2;

        if (input.keyPressed(Key::R)) {
            mgr.triggerReset(world_idx);
        }

        if (input.keyPressed(Key::W)) {
            y += 2;
        }
        if (input.keyPressed(Key::S)) {
            y -= 2;
        }

        if (input.keyPressed(Key::D)) {
            x += 2;
        }
        if (input.keyPressed(Key::A)) {
            x -= 2;
        }

        if (input.keyPressed(Key::Q)) {
            r += 2;
        }
        if (input.keyPressed(Key::E)) {
            r -= 2;
        }

        mgr.setAction(world_idx, agent_idx, x, y, r);
    }, [&mgr, &replay_log, &replayStep, &printObs]() {
        if (replay_log.has_value()) {
            replayStep();
        }

        mgr.step();
        
        printObs();
    });
}