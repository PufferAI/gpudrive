import gpudrive
import torch

# Create an instance of RewardParams
reward_params = gpudrive.RewardParams()
reward_params.rewardType = gpudrive.RewardType.DistanceBased  # Or any other value from the enum
reward_params.distanceToGoalThreshold = 1.0  # Set appropriate values
reward_params.distanceToExpertThreshold = 1.0  # Set appropriate values

# Create an instance of Parameters
params = gpudrive.Parameters()
params.polylineReductionThreshold = 0.5  # Set appropriate value
params.observationRadius = 10.0  # Set appropriate value
params.collisionBehaviour = gpudrive.CollisionBehaviour.Ignore  # Set appropriate value
params.datasetInitOptions = gpudrive.DatasetInitOptions.PadN  # Set appropriate value
params.rewardParams = reward_params  # Set the rewardParams attribute to the instance created above
params.maxNumControlledVehicles = 0

# Now use the 'params' instance when creating SimManager
sim = gpudrive.SimManager(
    exec_mode=gpudrive.madrona.ExecMode.CUDA,
    gpu_id=0,
    num_worlds=3,
    auto_reset=False,
    json_path="waymo_data_repeat",
    params=params,
    enable_batch_renderer=False, # Optional parameter
    # batch_render_view_width=1024,
    # batch_render_view_height=1024
)

frames = []
for _ in range(200):
    print(f"steps remaining: {sim.steps_remaining_tensor().to_torch()[0][0].item()}")
    sim.step()
    # rgb_tensor = sim.rgb_tensor().to_torch()
    # frames.append(rgb_tensor.cpu().numpy()[0,3,:,:,:3]) 
    # Use the obs tensor for further processing
    if sim.steps_remaining_tensor().to_torch()[0][0].item() == 1:
        print("RESET")
        for sim_idx in range(3):
            sim.reset(sim_idx)

# import imageio
# imageio.mimsave('movie.gif', frames, duration=0.1)  # Save the frames as a gif