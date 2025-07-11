import ntplib
import time

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.dates as mdates

import pandas as pd
from scipy.stats import skew

fig = plt.figure(figsize=(10, 5))
ax1 = fig.add_subplot(111)
ax1.set_title('NTP Offset Over Time')

def estimator(dat):
    while (len(dat) > 15) and (abs(skew(dat)) > 1.0):
        n = len(dat)
        ra = dat.max() - dat.min()
        nbins = int(np.sqrt(n))

        hist = np.histogram(dat, bins=nbins)


        likely = hist[0] > n/nbins
        mins = hist[1][:-1][likely]
        maxs = hist[1][1:][likely]
        counts = hist[0][likely]

        for i in range(len(mins)-1, 0, -1):
            if mins[i] == maxs[i-i]:
                mins = np.delete(mins, i)
                maxs = np.delete(maxs, i)
                counts[i-i] += counts[i]
                counts = np.delete(counts, i)

        maxindex = np.argmax(counts)
        min = mins[maxindex]
        max = maxs[maxindex]
        width = max - min
        count = counts[maxindex]

        dat = dat[(dat >= min-width*0.25) & (dat <= max + width*0.25)]

    return (np.median(dat), np.std(dat)/np.sqrt(len(dat)))

def count_outliers(dat):
    quartiles = np.nanquantile(dat, [0.25, 0.75])
    iqr = quartiles[1] - quartiles[0]
    min = quartiles[0] - 0.5 * iqr
    max = quartiles[1] + 0.5 * iqr
    return np.sum((dat < min) | (dat > max))

def remove_outliers(dat):
    quartiles = np.nanquantile(dat, [0.25, 0.75])
    iqr = quartiles[1] - quartiles[0]
    min = quartiles[0] - 0.5 * iqr
    max = quartiles[1] + 0.5 * iqr
    return dat[(dat >= min) & (dat <= max)]

def estimator2(dat):

    while (len(dat) > 15) and (count_outliers(dat) > len(dat)/20):
        dat = remove_outliers(dat)

    return (np.mean(dat), np.std(dat)/np.sqrt(len(dat)))

def update(frame):
    data = pd.read_csv('offsetlogger.csv', comment='#', names=['txctime', 'txtime', 'localctime', 'localtime', 'offset'])

    times = data['txtime']
    reltimes = times - times.iloc[-1]
    offsets = data['offset']

    width = 300

    txtimes = []
    meanoffsets = []
    stdoffsets = []

    ni = np.argmax(data['txtime']>= data['txtime'].iloc[0])
    for i in range(ni, len(data['offset'])):
        dat = data['offset'][(data['txtime'] < data['txtime'].iloc[i]) & (data['txtime'] >= (data['txtime'].iloc[i] - width))]
        mean, std = estimator2(dat)

        txtimes.append(data['txtime'].iloc[i])
        meanoffsets.append(mean)
        stdoffsets.append(std)

    txtimes = np.array(txtimes)
    meanoffsets = np.array(meanoffsets)
    stdoffsets = np.array(stdoffsets)

    reltimes2 = txtimes-txtimes[-1]

    ax1.clear()
    ax1.plot(reltimes, offsets, marker='o', linestyle='-', color='b', label='NTP Offset')

    ax1.errorbar(reltimes2, meanoffsets, yerr=stdoffsets, fmt='o', linestyle='-', color='r', label='Smoothed NTP Offset')

    qs = np.nanquantile(meanoffsets, [0.10, 0.9], axis=0)
    ax1.set_ylim(qs[0]-0.005, qs[1]+0.005)

ani = animation.FuncAnimation(fig, update, interval=1000)
plt.show()

