# GPUDrive: A Fast, GPU-Accelerated Multi-Agent Driving Simulator


`gpudrive` is a lightweight, multi-agent batched simulator built on the [Madrona Engine](https://madrona-engine.github.io/). It is designed to handle traffic scenarios from the Waymo Open Motion Dataset (WOMDB). [...]

**Key features**:

- High-speed simulations up to X FPS
- ...
- ...

## The environment and learning task


[TODO]


## Build Instructions 🛠️

[...]

## Quick start 🚀

You can get started with the intro tutorials [here]()

### Basic GPUDrive RL Usage 

```python
from gpudrive_.env.base_environment import Env

env = Env(
      num_worlds=1, # Number of worlds or environments to run in parallel
      max_cont_agents=1, # Maximum number of agents to control
      data_dir='waymo_data', # Path to the traffic scenario
      device='cuda' # Where to run
  )

obs = env.reset()
    
for _ in range(1000):

  rand_actions = torch.ones((env.num_sims, env.max_cont_agents))
  
  obs, reward, done, info = env.step(rand_actions)

env.close()    
```

### Basic GPUDrive API Usage 

[TODO]

## Baseline algorithms

| Algorithm     | Reference           | Implementation       | File                 |
|---------------|---------------------------------------------|---------------|----------------------|
| MAPPO   |  [Paper](https://arxiv.org/abs/2103.01955) | `Stable Baselines 3`         | run_ppo_sb3.py  |
| MAPPO   |  [Paper](https://arxiv.org/abs/2103.01955)  | `cleanrl`         | ... |



## Citation

If you use GPUDrive in a research project, please cite our paper.

```
@article{...,
}
```
