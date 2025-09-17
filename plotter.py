import pandas as pd
import matplotlib.pyplot as plt
import math


#-----------------------------------------Cria Histograma Para L1------------------------------------#

# Lê o CSV (um valor por linha)
df = pd.read_csv("log_auto_threshold_l1", header=None, names=["valor"])

# Calcula Q1, Q3 e IQR
Q1 = df["valor"].quantile(0.25)
Q3 = df["valor"].quantile(0.75)
IQR = Q3 - Q1

# Define limites para outliers
limite_inferior = Q1 - 1.5 * IQR
limite_superior = Q3 + 1.5 * IQR

# Remove outliers
df_sem_outliers = df[(df["valor"] >= limite_inferior) & (df["valor"] <= limite_superior)]

# Extrai os valores em uma lista
valores = df_sem_outliers["valor"]

# Calcula número de bins (agrupando de 2 em 2)
min_val, max_val = valores.min(), valores.max()
num_bins = math.ceil((max_val - min_val))

# Cria histograma
plt.hist(valores, bins=num_bins, edgecolor="black", rwidth=0.9, fc=(0, 0, 1, 0.5))


#-----------------------------------------Cria Histograma Para L2------------------------------------#

# Lê o CSV (um valor por linha)
df = pd.read_csv("log_auto_threshold_l2", header=None, names=["valor"])

# Calcula Q1, Q3 e IQR
Q1 = df["valor"].quantile(0.25)
Q3 = df["valor"].quantile(0.75)
IQR = Q3 - Q1

# Define limites para outliers
limite_inferior = Q1 - 1.5 * IQR
limite_superior = Q3 + 1.5 * IQR

# Remove outliers
df_sem_outliers = df[(df["valor"] >= limite_inferior) & (df["valor"] <= limite_superior)]

# Extrai os valores em uma lista
valores = df_sem_outliers["valor"]

# Calcula número de bins (agrupando de 2 em 2)
min_val, max_val = valores.min(), valores.max()
num_bins = math.ceil((max_val - min_val))

# Cria histograma
plt.hist(valores, bins=num_bins, edgecolor="black", rwidth=0.9, fc=(1, 0, 0, 0.5))




plt.xlabel("Intervalos de valores (agrupados de 2 em 2)")
plt.ylabel("Frequência")
plt.title("Histograma dos valores (bin = 2)")
plt.grid(axis="y", alpha=0.5)
plt.show()

