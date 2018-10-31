function ADSR(A,D,S,R,Ts)
%AUTHOR: Tim Lindquist
% A = Attack time (ms)
% D = Delay time (ms)
% S = Sustain level (0-1 multiplication factor)
% R = Release time (ms)
% Ts = Duration of Sustain(ms)
% ex: ADSR(100,150,0.7,500,1000)

close all;
f=440;

%base waveform=square wave @ 440Hz normalized to max amplitude=1
dt=0.0001;
t = 0:dt:(A+D+S+R+Ts)/1000;  %seconds
x = square(2*pi*f*t);

figure;
subplot(2,1,1);
plot(t,x);
xlabel('time (s)');
ylabel('Amplitude');
title('Base Waveform');


%amplitude vector
amp=zeros(1,length(t));

%attack linear rise to T_A
for i=1:round(A/(1000*dt));
    amp(i)=1/round(A/(1000*dt))*i;
end

%decay linear fall to T_D
j=1;
for i=i:round((A+D)/(1000*dt));
    amp(i)=(S-1)/round((D)/(1000*dt))*j+1;
    j=j+1;
end

%sustain to S till Tr
for i=i:round(((A+D+Ts)/(1000*dt)));
    amp(i)=S;
end

%release linear fall to T_R
j=1;
for i=i:round((A+D+Ts+R)/(1000*dt));
    amp(i)=(0-S)/round((R)/(1000*dt))*j+S;
    j=j+1;
end

x=x.*amp;
subplot(2,1,2);
plot(t,x);
xlabel('time (s)');
ylabel('Amplitude');
title('ADSR Envelope * Base Waveform');

sound(x,1/dt);

