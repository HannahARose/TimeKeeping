import ntplib
import time
import numpy as np
import pandas as pd

import logging

import atexit

logging.basicConfig(filename='offsetlogger.log',
                    level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')

def weights_from_uncertainty(sig_data):
    sigs = sig_data.copy()
    sigs[sigs<=0] = sigs[sigs > 0].min() / 2  # Remove non-positive uncertainties
    return 1 / np.pow(sigs, 2)

def weighted_mean(data, sig_data):
    weights = weights_from_uncertainty(sig_data)
    average = np.average(data, weights=weights)
    sigma = np.sqrt(1 / np.sum(weights))

    return average, sigma

def polyfit_with_uncertainty(data, x_column, y_column, sig_column=None, degree=3):
    """
    Fit a polynomial to the data with optional uncertainty handling.
    
    Parameters:
    - data: DataFrame containing the data.
    - x_column: Column to use as x values.
    - y_column: Column to use as y values.
    - sig_column: Optional column for uncertainties in y values.
    - degree: Degree of polynomial to fit.
    
    Returns:
    - Coefficients of the fitted polynomial.
    """
    
    x_data = data[x_column]
    y_data = data[y_column]


    if sig_column is not None:
        sig_data = data[sig_column]
        weights = weights_from_uncertainty(sig_data)
        fit, residuals, rank, singular_values, rcond = np.polyfit(x_data, y_data, degree, full=True, w=weights)
        return fit

    fit, residuals, rank, singular_values, rcond = np.polyfit(x_data, y_data, degree, full=True)
    return fit

def outlier_filter(data, filter_column, fit_column=None, sig_column=None, level=0.5, degree=3):
    """
    generate a filter list of non-outliers based on the IQR method.
    
    Parameters:
    - data: DataFrame containing the data.
    - filter_column: Column to filter for outliers.
    - fit_column: Optional column to fit a trend.
    - level: IQR multiplier for outlier detection.
    - degree: Degree of polynomial for trend fitting.
    
    Returns:
    - List of boolean values indicating non-outliers.
    """
    
    filter_data = data[filter_column]

    if fit_column is not None:
        fit_data = data[fit_column]

        if len(filter_data) != len(fit_data):
            raise ValueError("Filter and fit columns must have the same length.")
        
        trend = polyfit_with_uncertainty(data, fit_column, filter_column, sig_column, degree)

        filter_data = filter_data - np.polyval(trend, fit_data)

    quartiles = np.nanquantile(filter_data, [0.25, 0.75])
    iqr = quartiles[1] - quartiles[0]
    min_val = quartiles[0] - level * iqr
    max_val = quartiles[1] + level * iqr

    return (filter_data >= min_val) & (filter_data <= max_val)

def count_outliers(data, filter_column, fit_column=None, sig_column=None, level=0.5, degree=3):
    """
    Count the number of outliers in the specified column of a DataFrame.

    Parameters:
    - data: DataFrame containing the data.
    - filter_column: Column to filter for outliers.
    - fit_column: Optional column to fit a trend.
    - level: IQR multiplier for outlier detection.
    - degree: Degree of polynomial for trend fitting.

    Returns:
    - Number of outliers in the specified column.
    """
    filter = outlier_filter(data, filter_column, fit_column, sig_column, level, degree)

    return np.sum(~filter)

def remove_outliers(data, filter_column, fit_column=None, sig_column=None, level=0.5, degree=3):
    """
    Remove outliers from a DataFrame based on the IQR method.

    Parameters:
    - data: DataFrame containing the data.
    - filter_column: Column to filter for outliers.
    - fit_column: Optional column to fit a trend.
    - level: IQR multiplier for outlier detection.
    - degree: Degree of polynomial for trend fitting.

    Returns:
    - DataFrame with outliers removed.
    """

    filter = outlier_filter(data, filter_column, fit_column, sig_column, level, degree)

    return data[filter]

def predict_time(local_time, history, window=3000, level=1.5, degree=3):
    """
    Predict the server time based on local time and historical data.
    
    Parameters:
    - local_time: The current local time.
    - history: A DataFrame containing past offsets and their corresponding times.
    - window: The time window in seconds to consider for past offsets.
    - degree: The polynomial degree for fitting the past offsets.
    
    Returns:
    - Predicted server time.
    """
    
    past_data = history[(history['local_time'] < local_time) & (history['local_time'] >= local_time - window)]
    
    if len(past_data) < 5:

        if len(past_data) == 0:
            return local_time  # Not enough data to fit a polynomial
        else:
            return local_time + past_data['offset'].mean()  # Use mean offset if not enough data

    filtered_data = remove_outliers(past_data, 'offset', 'local_time', 'sig_offset', level=level, degree=degree)

    if len(filtered_data) < 5:

        if len(filtered_data) == 0:
            return local_time # Not enough data to fit a polynomial
        else:
            return local_time + past_data['offset'].mean()  # Use mean offset if not enough data

    trend = polyfit_with_uncertainty(filtered_data, 'local_time', 'offset', 'sig_offset', degree)

    return local_time + np.polyval(trend, local_time)

file = open('offsetlogger.csv', 'a')
history = pd.read_csv('offsetlogger.csv', comment='#', names=['local_time', 'pred_time', 'serv_time', 'offset', 'sig_offset', 'pred_offset', 'sig_pred_offset'])

client = ntplib.NTPClient()

logging.info("# NTP Offset Logger")
logging.info("# Fetching NTP data from utcnist.colorado.edu every 5 seconds")
logging.info("NTP Offset Logger started")

file.write("#local_time, pred_time, serv_time, offset, sig_offset, pred_offset, sig_pred_offset\n")
atexit.register(file.close)

while True:

    time.sleep(5-(time.time()%5))

    responses = []
    for i in range(60):
        try:
            responses.append(client.request('utcnist.colorado.edu', version=3, timeout=0.5))
        except Exception as e:
            logging.warning(f"Failed to fetch NTP data: {e}")
            print(f"Error: {e}")

    orig_times = np.array([response.orig_time for response in responses])
    dest_times = np.array([response.dest_time for response in responses])

    local_times = (orig_times + dest_times) / 2


    pred_times = np.array([predict_time(localtime, history, window = 3000, level = 1.5, degree = 3) for localtime in local_times]) 

    tx_times = np.array([response.tx_time for response in responses])
    rx_times = np.array([response.recv_time for response in responses])

    serv_times = (tx_times + rx_times) / 2

    offsets = np.array([response.offset for response in responses])
    sig_offsets = np.array([response.root_dispersion for response in responses])  # Using root dispersion as a proxy for uncertainty


    pred_offsets = np.array([serv_times[i] - pred_times[i] for i in range(len(pred_times))])
    sig_pred_offsets = sig_offsets


    sample_data = pd.DataFrame({
        'local_time': local_times,
        'pred_time': pred_times,
        'serv_time': serv_times,
        'offset': offsets,
        'sig_offset': sig_offsets,
        'pred_offset': pred_offsets,
        'sig_pred_offset': sig_pred_offsets
    })


    while (count_outliers(sample_data, 'offset', 'local_time', 'sig_offset', level=0.5, degree=3) > len(offsets) / 20):

        if len(sample_data) < 5:
            break

        sample_data = remove_outliers(sample_data, 'offset', 'local_time', 'sig_offset', level=0.5, degree=3)

    local_time = np.mean(local_times)  # Average local time
    pred_time = np.mean(pred_times)  # Average predicted time
    serv_time = np.mean(serv_times)  # Average server time
    offset, sig_offset = weighted_mean(offsets, sig_offsets)
    pred_offset, sig_pred_offset = weighted_mean(pred_offsets, sig_pred_offsets)

    new_data = {
        'local_time': local_time,
        'pred_time': pred_time,
        'serv_time': serv_time,
        'offset': offset,
        'sig_offset': sig_offset,
        'pred_offset': pred_offset,
        'sig_pred_offset': sig_pred_offset
    }

    history.loc[len(history)] = new_data

    new_line = f"{local_time}, {pred_time}, {serv_time}, {offset:.4e}, {sig_offset:.4e}, {pred_offset:.4e}, {sig_pred_offset:.4e}\n"

    print(new_line.strip())
    file.write(new_line)
    file.flush()