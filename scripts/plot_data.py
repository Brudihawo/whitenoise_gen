import numpy as np
import pandas as pd
from matplotlib import pyplot as plt


def gaussian(x, mu, sigma):
    return 1 / (sigma * np.sqrt(2 * np.pi)) \
           * np.exp(- 0.5 * np.power(((x - mu) / sigma), 2))


fig, ax = plt.subplots(2, 1)
freqs = pd.read_csv("./data/frequencies.csv", names=["freq", "intensity"])
wave = pd.read_csv("./data/wave.csv", names=["time", "intensity"])
ax[0].plot(freqs["freq"], freqs["intensity"])
ax[1].plot(wave["time"], wave["intensity"])
plt.show()
