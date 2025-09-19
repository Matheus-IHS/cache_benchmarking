import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt


#-----------------------------------Plota o grafico de violino para l1----------------------------------------#

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

# Cria o violin plot
a = sns.violinplot(y=df_sem_outliers["valor"], inner="box", color="r", bw_adjust=5)  # inner="box" adiciona um boxplot dentro do violino

for collection in a.collections:
    collection.set_alpha(0.5) 

#-----------------------------------Plota o grafico de violino para l2----------------------------------------#

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

# Cria o violin plot
b = sns.violinplot(y=df_sem_outliers["valor"], inner="box", color="b", bw_adjust=5)  # inner="box" adiciona um boxplot dentro do violino

for collection in b.collections:
    collection.set_alpha(0.5) 

#-----------------------------------Plota o grafico de violino para l2----------------------------------------#

# Lê o CSV (um valor por linha)
df = pd.read_csv("log_auto_threshold_l3", header=None, names=["valor"])

# Calcula Q1, Q3 e IQR
Q1 = df["valor"].quantile(0.25)
Q3 = df["valor"].quantile(0.75)
IQR = Q3 - Q1

# Define limites para outliers
limite_inferior = Q1 - 1.5 * IQR
limite_superior = Q3 + 1.5 * IQR

# Remove outliers
df_sem_outliers = df[(df["valor"] >= limite_inferior) & (df["valor"] <= limite_superior)]

# Cria o violin plot
b = sns.violinplot(y=df_sem_outliers["valor"], inner="box", color="g", bw_adjust=5)  # inner="box" adiciona um boxplot dentro do violino

for collection in b.collections:
    collection.set_alpha(0.5) 



plt.title("Violin Plot dos Valores")
plt.ylabel("Valor")
plt.show()

