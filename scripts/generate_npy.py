"""
Generates and saves binary .npy files to the specified directory. Call it with
-h or --help to see the available options.
"""

import argparse
import os

import numpy as np


def is_valid_path(filepath: str) -> bool:
    """
    Disallow navigation up the directory tree or use of absolute path.
    """
    if (
        ".." in filepath
        or filepath.startswith("/")
        or (":" in filepath and "\\" in filepath)
    ):
        return False
    return True


def generate_npy(
    num_entries: int, num_files: int, filepath: str, seed: int = None
) -> None:
    """
    Generates and saves binary .npy files to the specified directory.
    """
    if num_files > 1000:
        raise ValueError("Error: -n argument cannot be greater than 1000.")
    if num_entries > 1e9:
        raise ValueError("Error: -N argument cannot be greater than 1e9.")
    if not is_valid_path(filepath):
        raise ValueError(
            "Error: -f argument must target either the current directory or subdirectories."
        )

    # Ensure the directory exists
    os.makedirs(filepath, exist_ok=True)

    # Initialize the random number generator with the specified seed
    _rng = np.random.default_rng(seed)

    # Determine the number of digits for zero-padding
    num_digits: int = len(str(num_files))

    for i in range(num_files):
        # Generate binary data using the _rng generator
        data = _rng.normal(2.5, 2.5, size=num_entries).astype(np.float32)

        # Create filename with zero-padding
        filename: str = f"binary{str(i).zfill(num_digits)}.npy"
        full_path: str = os.path.join(filepath, filename)

        # Save file
        np.save(full_path, data)

    print(
        f"Generated and saved {num_files} binary files of length {num_entries} to {filepath}"
    )

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate and save binary .npy files.")
    parser.add_argument("-N", type=int, help="Length of the binary data array.")
    parser.add_argument("-n", "--number", type=int, help="Number of files to generate.")
    parser.add_argument("-f", default="data/", help="Filepath for saving the files.")
    parser.add_argument(
        "-s",
        "--seed",
        type=int,
        default=None,
        help="Seed for random number generation.",
    )

    args = parser.parse_args()

    generate_npy(args.N, args.number, args.f, args.seed)
