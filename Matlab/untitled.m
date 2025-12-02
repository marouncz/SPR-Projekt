clc
clear all
close all

% Parameters
alpha = 0.6;
beta  = 0.6;
fs    = 48000;      % Sampling rate (must be >= 2*20kHz)
n     = fs*0.05         % <-- change delay order if needed


% Define frequency range
f = linspace(100, 150, 1000);
w = 2*pi*f/fs;      % normalized radian frequency

% Compute frequency response
z = exp(1j*w);
H = alpha ./ (z.^n - alpha*beta);

% Plot magnitude response
figure;
plot(f, abs(H), 'LineWidth', 2);
grid on;
xlabel('Frequency (Hz)');
ylabel('|H(f)|');
title('Magnitude Response of H(z) = \alpha / (z^n - \alpha\beta)');

% Optional: plot in dB
figure;
plot(f, 20*log10(abs(H)), 'LineWidth', 2);
grid on;
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('Magnitude Response in dB');
