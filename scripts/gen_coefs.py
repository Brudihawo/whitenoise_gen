import numpy as np

N_COEFS = 20
sqrt_pi_2 = np.sqrt(np.pi) / 2

coefs = np.zeros(N_COEFS)

coefs[0] = sqrt_pi_2

for k in range(1, N_COEFS):
    for m in range(k):
        coefs[k] += (coefs[m] * coefs[k - 1 - m]) / ((m + 1) * (2 * m + 1))
        coefs[k] *= np.power(sqrt_pi_2, 2 * k + 1) / (2 * k + 1)


for c in coefs:
    print(c)
