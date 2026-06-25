import sys
import polars as pl
import seaborn as sns
import matplotlib.pyplot as plt

df = pl.read_csv(sys.stdin.buffer)
plot = sns.displot(df, x="compressed_size", binwidth=128)

#plot.set(yscale="log")
plt.ylim(0,250)
plot.savefig("vis/compressed_size.png")