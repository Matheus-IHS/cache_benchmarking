import pandas as pd
import matplotlib.pyplot as plt
import math

def remove_outliers(df):
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

    return valores


#-----------------------------------------Print L1 Statistics------------------------------------#

# Lê o CSV (um valor por linha)
df_l1 = pd.read_csv("log_latencia_cache_l1", header=None, names=["valor"])

# Extrai os valores em uma lista
values_l1 = remove_outliers(df_l1)

mean_l1 = values_l1.mean()
std_deviation_l1 = values_l1.std()

print("media de latencia da cache L1 em CLK: {:.1f}".format(mean_l1))
print("desvio padrao de latencia de cache L1 em CLK: {:.1f}".format(std_deviation_l1))

#-----------------------------------------Print L2 Statistics------------------------------------#

# Lê o CSV (um valor por linha)
df_l2 = pd.read_csv("log_latencia_cache_l2", header=None, names=["valor"])

# Extrai os valores em uma lista
values_l2 = remove_outliers(df_l2)

mean_l2 = values_l2.mean()
std_deviation_l2 = values_l2.std()

print("media de latencia da cache L2 em CLK: {:.1f}".format(mean_l2))
print("desvio padrao de latencia de cache L2 em CLK {:.1f}:".format(std_deviation_l2))


#-----------------------------------------Print L3 Statistics------------------------------------#

# Lê o CSV (um valor por linha)
df_l3 = pd.read_csv("log_latencia_cache_l3", header=None, names=["valor"])

# Extrai os valores em uma lista
values_l3 = remove_outliers(df_l3)

mean_l3 = values_l3.mean()
std_deviation_l3 = values_l3.std()

print("media de latencia da cache L3 em CLK: {:.1f}".format(mean_l3))
print("desvio padrao de latencia de cache L3 em CLK: {:.1f}".format(std_deviation_l3))

