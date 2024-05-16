from networks.basic_ffn import FeedForwardPolicy
from dataclasses import dataclass
import torch


@dataclass
class ExperimentConfig:
    """Configurations for experiments."""

    # General
    device: str = "cuda"

    # Dataset
    data_dir: str = "waymo_data_repeat"

    # Rendering settings
    render: bool = False
    render_mode: str = "rgb_array"
    render_freq: int = 25

    # Hyperparameters
    policy: torch.nn.Module = FeedForwardPolicy
    seed: int = 42
    n_steps: int = 1024
    batch_size: int = 512
    verbose: int = 0
    total_timesteps: int = 30_000_000

    # Wandb
    project_name = "gpudrive"
    group_name = "PPO"
    entity = "_emerge"
