import numpy as np
import soundfile as sf

def echo_reverb_exact(x, Fs, N_samples, alpha, beta):
    # ensure 2D array
    if x.ndim == 1:
        x = x[:, None]

    T, C = x.shape
    y = np.zeros_like(x)

    # circular buffer d[n]
    buf_len = N_samples + 1
    d_buf = np.zeros((buf_len, C))
    w = 0  # write index

    maxAmp = 1 + alpha/(1+alpha*beta)

    for n in range(T):
        
        xn = x[n]/maxAmp

        # read delayed d[n-N]
        r = (w - N_samples) % buf_len
        d_delay = d_buf[r]

        # s[n] = α * d[n−N]
        s = alpha * d_delay

        # y[n] = x[n] + s[n]
        y[n] = xn + s

        # d[n] = x[n] + β * s[n]
        d_n = xn + beta * s

        # write to circular buffer
        d_buf[w] = d_n
        w = (w + 1) % buf_len

    # return same shape as input
    return y if y.shape[1] > 1 else y[:,0]


x, Fs = sf.read("audio_original.mp3")
N = int(0.2 * Fs)   # 200 ms
alpha = 0.2
beta  = 0.2

y = echo_reverb_exact(x, Fs, N, alpha, beta)
sf.write("echoOutput.wav", y, Fs)
