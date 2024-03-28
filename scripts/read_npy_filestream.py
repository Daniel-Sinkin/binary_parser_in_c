import numpy as np


def print_npy_floats(file_path):
    arr = np.load(file_path)

    print(type(arr))
    print(arr.shape)
    for val in np.nditer(arr):
        print(float(val))


if __name__ == "__main__":
    file_path = "data/binary0.npy"
    print_npy_floats(file_path)
