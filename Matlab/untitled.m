clc
clear all
close all

% Parameters
alpha = 0.6;
beta  = 0.6;
fs    = 48000;      % Sampling rate (must be >= 2*20kHz)
n     = fs*0.05         % <-- change delay  if needed


% Define frequency range
f = linspace(0, 20000, 60000);
w = 2*pi*f/fs;      % normalized radian frequency

% Compute frequency response
z = exp(1j*w);
H = 1 + (alpha.*z.^-n)./(1 - alpha.*beta.*z.^-n);


% Optional: plot in dB
figure;
semilogx(f, 20*log10(abs(H)), 'LineWidth', 1);
grid on;
xlabel('Frekvence (Hz)');
ylabel('Přenos (dB)');
title('Frekvenční charakteristika echo filtru. α=0.6, β=0.6, D=50ms');
