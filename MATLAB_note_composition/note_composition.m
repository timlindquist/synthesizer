function note_composition(filename)
close all;
[y,Fs]=audioread(filename);
%sound(y,Fs);
l=length(y);
f=-Fs/2:Fs/l:Fs/2-Fs/l;
figure
%Y=fftshift(fft(y))/length(fft(y));
Y=fft(y);
f = Fs*(0:(l/2))/l;
P2 = abs(Y/l);
P1 = P2(1:l/2+1);
P1(2:end-1) = 2*P1(2:end-1);
plot(f,P1)
axis([0 3000 0 0.03])
%plot(f/2,abs(Y));
title('FFT of Input Audio');
xlabel('Frequency(Hz)');
ylabel('Amplitude');

amplitudes=findpeaks(abs(y));

[pks,locs] = findpeaks(abs(Y));
B = sort(pks,'descend');
C=zeros(1,length(B));
%for i=1:length(B)
for i=1:10
    for j=1:length(pks)
        if(B(i)==pks(j))
            C(1,i)=f(locs(j));
            break
        end
    end
end

output=0;
t=0:1/Fs:2;
for i=1:10
    output=output+B(i)*sin(2*pi*C(i)*t);
end


t = (0:l-1)*1/Fs;
%output=.0033*sin(2*pi*260.9*t)+.02378*sin(2*pi*523.6*t)+0.00407*sin(2*pi*785.9*t)+.0012*sin(2*pi*1049*t)+.001951*sin(2*pi*1848*t);
%make it loud then soft multiply the amplitude of the piano note
length(output)
A=zeros(1,length(output));
k=0;
for i=1:length(amplitudes)
    for j=1:2
        A(i+k)=amplitudes(i);
        k=k+1;
    end  
end

%output=output.*A;
sound(output,Fs);