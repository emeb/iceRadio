% freq_plot.m - frequency plot
% E. Brombaugh 08-03-16
function freq_plot(x, Fs, title_str)
	sz = length(x);
	f = Fs * (((0:sz-1)/sz)-0.5);
	plot(f, 20*log10(abs(fftshift(fft(x)/sz))));
	grid on;
	title(title_str);
	xlabel('Freq');
	ylabel('dB');
end
