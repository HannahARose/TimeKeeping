import time

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.dates as mdates

import pandas as pd
from numpy.polynomial.polynomial import Polynomial as P
from scipy.stats import linregress

import allantools as at

fig, axes = plt.subplots(nrows=2, ncols=1, figsize=(10, 10))
ax1 = axes[0]
ax2 = axes[1]

def count_outliers(data, level=0.5):
    quartiles = np.nanquantile(data, [0.25, 0.75])
    iqr = quartiles[1] - quartiles[0]
    lower_bound = quartiles[0] - level * iqr
    upper_bound = quartiles[1] + level * iqr
    return np.sum((data < lower_bound) | (data > upper_bound))

def remove_outliers(data, weights=None, times=None, level=0.5, degree=3):
    if times is not None:
        if len(data) != len(times):
            raise ValueError("Data and times must have the same length.")
    if weights is not None:
        if len(data) != len(weights):
            raise ValueError("Data and weights must have the same length.")
    if times is not None:
        trend = P.fit(times, data, degree, w=weights)
        deviations = data - trend(times)
    else:
        trend = P.fit(np.arange(len(data)), data, degree, w=weights)
        deviations = data - trend(np.arange(len(data)))

    qts = np.quantile(deviations, [0.25, 0.75])
    iqr = qts[1] - qts[0]
    lower_bound = qts[0] - level * iqr
    upper_bound = qts[1] + level * iqr
    
    trim_data = data[(deviations >= lower_bound) & (deviations <= upper_bound)]

    if times is not None:
        times = times[(deviations >= lower_bound) & (deviations <= upper_bound)]
        if weights is not None:
            weights = weights[(deviations >= lower_bound) & (deviations <= upper_bound)]
            return trim_data, times, weights
        else :
            return trim_data, times
    else:
        return trim_data

def corrected_offset(time, offset, history, window=3000, degree=3):
    """
    Calculate the corrected offset for a given time using past offsets.
    
    Parameters:
    - time: The current time for which to calculate the corrected offset.
    - offset: The current offset value.
    - history: A DataFrame containing past offsets and their corresponding times.
    - window: The time window in seconds to consider for past offsets.
    - degree: The polynomial degree for fitting the past offsets.
    
    Returns:
    - Corrected offset value.
    """
    past_data = history[(history['txtime'] < time) & (history['txtime'] >= time - window)]
    
    if len(past_data) < 5:
        if len(past_data) == 0:
            return 0  # Not enough data to fit a polynomial
        else:
            return offset - past_data['offset'].mean()  # Use mean of available data
    
    past_data.loc[past_data['sigoffset'] <= 0, 'sigoffset'] = 0.5e-6  # Remove non-positive sigoffsets
    past_offset, past_times, past_weights = remove_outliers(
        past_data['offset'].values, 
        times=past_data['txtime'].values,
        weights=1/np.pow(past_data['sigoffset'].values,2), 
        level=0.5, 
        degree=degree)
    
    if len(past_offset) < 5:
        return offset - past_offset.mean() # Not enough data after outlier removal
    
    offset_fit = P.fit(past_times, past_offset, degree, w=past_weights)
    
    return offset - offset_fit(time)



# def remove_outliers(dat, level=0.5):
#     quartiles = np.nanquantile(dat, [0.25, 0.75])
#     iqr = quartiles[1] - quartiles[0]
#     min_time = quartiles[0] - level * iqr
#     max_time = quartiles[1] + level * iqr
#     return dat[(dat >= min_time) & (dat <= max_time)]

def update(frame):
    # data = pd.read_csv('offsetlogger5.csv', comment='#', names=['txctime', 'txtime', 'localctime', 'localtime', 'offset', 'sigoffset'])
    data = pd.read_csv('offsetlogger.csv', comment='#', names=['local_time', 'pred_time', 'serv_time', 'offset', 'sig_offset', 'pred_offset', 'sig_pred_offset'])

    serv_times = data['serv_time'].values
    datetimes = pd.to_datetime(serv_times, unit='s')
    deltas = serv_times[1:] - serv_times[:-1]
    rate = 1/np.mean(deltas)

    duration = serv_times.max() - serv_times.min()

    taus = np.logspace(np.log10(1/rate), np.log10(duration), num=int(duration * rate / 20))
    taus = np.logspace(0, 15, 481, base=2) / rate

    adev = at.oadev(data['offset'], rate=rate, taus=taus)

    offsets = data['offset'].values
    sigoffsets = data['sig_offset'].values

    # adjusted_offset = np.zeros_like(data['offset'])

    # window = 3000  # seconds
    # for i in range(0, len(data)):
    #     current_time = data['txtime'][i]
    #     current_offset = data['offset'][i]
    #     history = data[:i]  # Use all previous data up to the current index

    #     adjusted_offset[i] = corrected_offset(current_time, current_offset, history, window=window, degree=3)

    adjusted_offset = data['pred_offset'].values
    sig_adjusted_offset = data['sig_pred_offset'].values

    adjusted_adev = at.oadev(adjusted_offset, rate=rate, taus=taus)
    adjusted_no_out_offsets, adjusted_no_out_times = remove_outliers(adjusted_offset, times=serv_times, level=5, degree=3)
    adjusted_no_out_adev = at.oadev(adjusted_no_out_offsets, rate=rate, taus=taus)
    adjusted_no_out_datetimes = pd.to_datetime(adjusted_no_out_times, unit='s')

    fit = linregress(serv_times, offsets)

    offsets = offsets - (fit.intercept + fit.slope * serv_times)

    ax1.clear()
    ax1.errorbar(adev[0], adev[1], yerr=adev[2], fmt='.', linestyle='-', color='b', label='Allan Deviation')
    ax1.errorbar(adjusted_adev[0], adjusted_adev[1], yerr=adjusted_adev[2], fmt='.', linestyle='-', color='r', label='Adjusted Allan Deviation')
    ax1.errorbar(adjusted_no_out_adev[0], adjusted_no_out_adev[1], yerr=adjusted_no_out_adev[2], fmt='.', linestyle='-', color='g', label='Adjusted Allan Deviation, Outliers Removed')

    ax1.set_xlim(taus.min(), taus.max())
    ax1.set_ylim(np.pow(10,np.floor(np.log10(adjusted_no_out_adev[1]).min())), np.pow(10,np.ceil(np.log10(adev[1]).max())))

    ax1.set_xscale('log')
    ax1.set_yscale('log')
    ax1.set_xlabel('Time (s)'.format(rate))
    ax1.set_ylabel('Allan Deviation')
    ax1.set_title('Allan Deviation of NTP Offset')
    ax1.grid(True, which='both', linestyle='--', linewidth=0.5)
    ax1.legend()


    ax2.clear()
    ax2.errorbar(datetimes, offsets, yerr=sigoffsets, fmt='.', linestyle='-', color='b', label='NTP Offset')
    ax2.errorbar(datetimes, adjusted_offset, fmt='o', markersize=1, linestyle='-', color='r', label='Adjusted NTP Offset')
    ax2.errorbar(adjusted_no_out_datetimes, adjusted_no_out_offsets, fmt='o', markersize=1, linestyle='-', color='g', label='Adjusted NTP Offset, Outliers Removed')

    low = np.nanmin(remove_outliers(offsets, level=3))
    high = np.nanmax(remove_outliers(offsets, level=3))
    diff = high - low

    ax2.set_ylim(low-diff/10, high+diff/10)

    date_format = mdates.DateFormatter('%d.%m %H:%M:%S')
    ax2.xaxis.set_major_formatter(date_format)
    ax2.xaxis.set_major_locator(mdates.AutoDateLocator())

    # fig.autofmt_xdate()

    ax2.set_xlabel(f'Time (UTC), total duration: {duration:.2f} s')
    ax2.set_ylabel('NTP Offset (s)')
    ax2.set_title(f'NTP Offset Over Time, Excluding Drift: {fit.slope:.2e} s/s (Current Absolute Offset: {data['offset'].iloc[-1]:.3f} s)')
    ax2.grid(True, which='both', linestyle='--', linewidth=0.5)
    ax2.legend()


ani = animation.FuncAnimation(fig, update, interval=1000)
plt.show()

