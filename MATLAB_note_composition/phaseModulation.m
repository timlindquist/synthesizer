function phaseModulation(f, change);
close all;
dt=0.0001;
Fs=1/dt;
t = 0:dt:2;

%y=sin(2*pi*f*t)+0.5*sin(4*pi*f*t+rand()*2*pi)
%first harmonic
% y_phase=zeros(1,length(t));
% phase=0;
% for i=1:length(t)
%     if(~mod(i,change))
%         phase=rand()*2*pi;
%     end
%     y_phase(i)=sin(2*pi*f*t(i)+phase);
% end
% 
% plot(t(1:50),y_phase(1:50));

%sound(y_phase,Fs);


%2 harmonics
%hear pops because of very high frequency when the discontinuties occure
%amplitude adjustment
%pressing 2 keys on piano then tapping one
y_phase=zeros(1,length(t));
phase=0;
for i=1:length(t)
    if(~mod(i,change))
        phase=rand()*2*pi;
    end
    y_phase(i)=sin(2*pi*f*t(i))+0.5*sin(4*pi*f*t(i)+phase);
end
y=sin(2*pi*f*t)+0.5*sin(4*pi*f*t);
plot(t(1:2500),y_phase(1:2500));
sound(y,Fs);
pause(5);
sound(sin(2*pi*f*t)+0.5*sin(4*pi*f*t+phase),Fs);
pause(5);
sound(y_phase,Fs);
figure;
l=length(y);
Y=fft(y);
P2 = abs(Y/l);
P1 = P2(1:l/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(l/2))/l;
subplot(2,1,1);
stem(f,P1);
xlim([0 1000]);
title('FFT')
xlabel('f (Hz)')
ylabel('Magnitude')

l=length(y_phase);
Y=fft(y_phase);
P2 = abs(Y/l);
P1 = P2(1:l/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(l/2))/l;
subplot(2,1,2);
stem(f,P1);
xlim([0 1000]);
title('FFT')
xlabel('f (Hz)')
ylabel('Magnitude')
