function note_decompose(filename, n)
%filename = '*.mp3'
%n= number of sinusoids
close all;
[y,Fs]=audioread(filename);
sound(y,Fs);
l=length(y);

%compute fft
Y=fft(y);
P2 = abs(Y/l);
P1 = P2(1:l/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(l/2))/l;

P2_neg = (Y/l);
P1_neg = P2(1:l/2+1)
P1_neg(2:end-1) = 2*P1(2:end-1);



%get fundamental frequency
[M,I]= max(P1);
f0=f(I);

%plot period in time domain
y_time=y(1:10*Fs/f0);
t = 0:1/Fs:(length(y_time)/Fs)-1/Fs;
subplot(2, 2, 1);
plot(t,y_time);
title('Time Domain')
xlabel('Seconds'); 
ylabel('Amplitude');
subplot(2, 2, 2);
plot(f,P1) 
title('FFT')
xlabel('f (Hz)')
ylabel('Magnitude')
xlim([20 20000]) %audible range


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Decompose n Dominate sinusoids
[pks,locs] = findpeaks(P1);
pks_sort=pks;
locs_sort=locs;

for i=1:length(pks_sort)-1
    for j=1:length(pks_sort)-1
        if pks_sort(j)<pks_sort(j+1)
            pksTemp=pks_sort(j);
            pks_sort(j)=pks_sort(j+1);
            pks_sort(j+1)=pksTemp;
            locsTemp=locs_sort(j);
            locs_sort(j)=locs_sort(j+1);
            locs_sort(j+1)=locsTemp;
        end
    end
end
%account for negatives
for i=1:length(pks_sort)
    if P1_neg(locs_sort(i))<0
        pks_sort(i)=-pks_sort(i);
    end
end


%addititive synthesis
tGen = 0:1/Fs:(length(y)/Fs)-1/Fs;
yGen=zeros(1,length(tGen));
if n>length(pks_sort) 
    n=length(pks_sort);
end

for i=1:n
   yGen=yGen+pks_sort(i)*sin(2*pi*f(locs_sort(i))*tGen);
end
sound(yGen,Fs);

YGen=fft(yGen);
P2 = abs(YGen/l);
P1 = P2(1:l/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(l/2))/l;


%plot stuff
y_time=yGen(1:10*Fs/f0);
t = 0:1/Fs:(length(y_time)/Fs)-1/Fs;

subplot(2, 2, 3);
plot(t,y_time);
title('Time Domain')
xlabel('Seconds'); 
ylabel('Amplitude');
subplot(2, 2, 4);
plot(f,P1) 
title('FFT')
xlabel('f (Hz)')
ylabel('Magnitude')
xlim([20 20000]) %audible range