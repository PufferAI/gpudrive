from networks.basic_ffn import FeedForwardPolicy
from dataclasses import dataclass
import torch


@dataclass
class ExperimentConfig:
    """Configurations for experiments."""

    # General
    device: str = "cuda"

    # Dataset
    data_dir: str = "waymo_data"

    # Rendering options
    render: bool = False
    render_mode: str = "rgb_array"
    render_freq: int = 10

    # Hyperparameters
    policy: torch.nn.Module = FeedForwardPolicy
    seed: int = 42
    n_steps: int = 1024
    batch_size: int = 512
    verbose: int = 0
    total_timesteps: int = 50_000_000
