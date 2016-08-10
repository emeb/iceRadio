% sync_am.m - synchronous AM testbed
% 08-03-16 E. Brombaugh

% setup some stuff
Fs = 40e6/2048;	% sample rate
Fc = 30;			% carrier freq
Fm = 1000;			% modulation freq
Mod_ratio = 0.5;	% modulation ratio
sz = 2^16;			% signal length
SNR_dB = 20;		% 

% amplitude modulate a tone onto the complex carrier
Fc_dyn = ones(1,sz)*Fc;
Fc_dyn(floor(end/2):end) = -Fc_dyn(floor(end/2):end);
t = (0:sz-1)/Fs;
Sig = (1.0 + Mod_ratio*sin(2*pi*Fm*t)).*exp(1i*2*pi*Fc_dyn.*t);

% add some noise
RX_Sig = awgn(Sig, SNR_dB, 'measured');

% plot RX
figure(1);
freq_plot(RX_Sig, Fs, 'RX Sig');
figure(2)
plot(t, real(RX_Sig),'r');
hold on;
plot(t, imag(RX_Sig),'g');
hold off;
title('RX Sig');
xlabel('time');
ylabel('value');
xlim([0 0.1]);

% set up the PLL
bb = zeros(1,sz);
dc_est = zeros(1,sz);
out = zeros(1,sz);
err = zeros(1,sz);
lf_int = zeros(1,sz);
frq = zeros(1,sz);
phs = zeros(1,sz);
lo = zeros(1,sz);
dc_coeff = 1e-3;
BWSel = 1;
P_coeff = [1e-3 5e-4 5e-6 ];
I_coeff = [1e-6 1e-7 1e-8 ];
timer_st = 0;
timer_ct = 0;

% iterate over the full signal
for i=1:sz-1
    % check for DC estimate ramp up after PLL lock
    if (timer_st == 0) && (dc_est(i) > 0.5)
        % Start timer
        timer_st = 1;
        timer_ct = 5000;
    end
    
    % Reset BW if lost lock
    if (timer_st == 2) && (dc_est(i) < 0.5)
        BWSel = 1;
        timer_st = 0;
    end
    
    % timeout?
    if (timer_st == 1)
        if timer_ct == 0
            timer_st = 2;
            BWSel = 3;
        else
            timer_ct = timer_ct - 1;
        end
    end
    
    % conjugate LO and mixer
	lo(i) = exp(-1i*2*pi*phs(i));
	bb(i) = lo(i) * RX_Sig(i);
    
    % Raw AM is real component of baseband - DC block it
	out(i) = real(bb(i)) - dc_est(i);
	dc_est(i+1) = dc_est(i) + out(i)*dc_coeff;
    
    % Error is angle of baseband
	err(i) = angle(bb(i));
    
    % Loop Filter set up so BW changes don't disturb it
	lf_int(i+1) = lf_int(i) + I_coeff(BWSel) * err(i);
	frq(i) = lf_int(i) + P_coeff(BWSel) * err(i);
    
    % Update phase of LO
	phs(i+1) = mod(phs(i) + frq(i), 1.0);
end

% Plot PLL
figure(3)
plot(t, err,'r');
hold on;
plot(t, 100*frq, 'g');
hold off;
title('PLL');
xlabel('time');
ylabel('value');
legend('err', 'frq');
%xlim([0 0.1]);

% plot Phase
figure(4)
plot(t, phs,'r');
title('Phs');
xlabel('time');
ylabel('value');
xlim([0.38 0.4]);

% plot LO
figure(5)
plot(t, real(RX_Sig),'r');
hold on;
plot(t, real(lo), 'g');
hold off;
title('LO');
xlabel('time');
ylabel('value');
legend('RX Sig', 'LO');
%xlim([0.38 0.4]);

% plot baseband
figure(6)
plot(t, real(bb),'r');
hold on;
plot(t, imag(bb), 'g');
plot(t, dc_est, 'k');
plot(t, out, 'b');
hold off;
title('Baseband');
xlabel('time');
ylabel('value');
legend('I', 'Q', 'DC', 'Out');
%xlim([0.39 0.4]);

