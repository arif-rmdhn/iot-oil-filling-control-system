# Mengimpor kembali pustaka yang dibutuhkan
import numpy as np
from sklearn.linear_model import LinearRegression

# Data dari tabel
real_ph = np.array([4.00, 6.86, 9.10]).reshape(-1, 1)
sensor_ph = np.array([4.0, 4.3, 6.0])

# Melatih model regresi linear
model = LinearRegression()
model.fit(sensor_ph.reshape(-1, 1), real_ph)

# Mengambil koefisien dan intercept
slope = model.coef_[0][0]
intercept = model.intercept_[0]

print(slope, intercept)


