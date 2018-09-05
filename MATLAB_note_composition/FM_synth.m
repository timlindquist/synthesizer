function FM_synth(A, C, D, M)
close all;

dt=0.0001;
Fs=1/dt;
t = 0:dt:.05;  %seconds
figure;

y=A*sin(2*pi*C*t+D*sin(2*pi*M*t));
c=A*sin(2*pi*C*t);
m=D*sin(2*pi*M*t);

subplot(3,2,1);
plot(t,c);
title('Carrier');
xlabel('Time');
ylabel('Magnitude');

l=length(c);
Y=fft(c);
P2 = abs(Y/l);
P1 = P2(1:l/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(l/2))/l;
subplot(3,2,2);
stem(f,P1);
xlim([0 1000]);
title('FFT')
xlabel('f (Hz)')
ylabel('Magnitude')

subplot(3,2,3);
plot(t,m);
title('Modulator');
xlabel('Time');
ylabel('Magnitude');

l=length(m);
Y=fft(m);
P2 = abs(Y/l);
P1 = P2(1:l/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(l/2))/l;
subplot(3,2,4);
stem(f,P1);
xlim([0 1000]);
title('FFT')
xlabel('f (Hz)')
ylabel('Magnitude')

subplot(3,2,5);
plot(t,y);
title('FM Output');
xlabel('Time');
ylabel('Magnitude');

l=length(y);
Y=fft(y);
P2 = abs(Y/l);
P1 = P2(1:l/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(l/2))/l;
subplot(3,2,6);
stem(f,P1);
xlim([0 1000]);
title('FFT')
xlabel('f (Hz)')
ylabel('Magnitude');

sound(y,1/dt);