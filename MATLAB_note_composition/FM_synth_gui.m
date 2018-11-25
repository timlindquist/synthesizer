function FM_synth_gui
%AUTHOR: Tim Lindquist
%Interactive FM synthesis program
%Top Left Knob: Carrier Frequency
%Top Right Knob: Carrier Amplitude
%Top Dropdown: Carrier Frequency Multiplier
%Bottom Left Knob: Modulator Frequency
%Bottom Right Knob: Modulator Amplitude
%Bottom Dropdown: Modulator Frequency Multiplier
close all;

fig = uifigure;
fig.Position=[100,500,300,400];
ui();


A_prev=kb_carrier_amp.Value;
C_prev=kb_carrier_freq.Value*dd_carrier_mul.Value;
D_prev=kb_modulator_amp.Value;
M_prev=kb_modulator_freq.Value*dd_modulator_mul.Value;

while(1)
    pause(0.1);
 
    
    A=kb_carrier_amp.Value;
    if btn_carrier.Value
        C=kb_carrier_freq.Value*dd_carrier_mul.Value;
    else
        C=0;
    end
    D=kb_modulator_amp.Value;
    if btn_modulator.Value
        M=kb_modulator_freq.Value*dd_modulator_mul.Value;
    else
        M=0;
    end
    
    if(A~=A_prev|C~=C_prev|D~=D_prev|M~=M_prev)
        dt=0.000001;
        Fs=1/dt;
        t = 0:dt:.05;  %seconds


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
        P1 = P2(1:(l/2+1));
        P1(2:end-1) = 2*P1(2:end-1);
        f = Fs*(0:(l/2))/l;
        subplot(3,2,2);
        stem(f,P1);
        xlim([0 max(2*C,100)]);
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
        xlim([0 max(2*M,100)]);
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
        xlim([0 max(max(2*C,2*M),100)]);
        title('FFT')
        xlabel('f (Hz)')
        ylabel('Magnitude');
      
        
    end
    
    if btn_play.Value
        dt=0.00001;
        Fs=1/dt;
        t = 0:dt:3;  %seconds
        y=A*sin(2*pi*C*t+D*sin(2*pi*M*t));
        sound(y,Fs);
       
        btn_play.Value = false;
    end
 
   A_prev=A;
   C_prev=C;
   D_prev=D;
   M_prev=M;
    
   
end

    function ui
        kb_carrier_freq = uiknob(fig);
        kb_carrier_freq.Limits = [0 20];
        kb_carrier_freq.Position=[50,300,60,60];

        kb_carrier_amp = uiknob(fig);
        kb_carrier_amp.Limits = [0 5];
        kb_carrier_amp.Position=[200,300,60,60];
      
        btn_carrier= uibutton(fig,'state','Text', 'carrier','Value', true,'Position',[35 250 100 22]);
        btn_modulator= uibutton(fig,'state','Text', 'modulator','Value', true,'Position',[35 50 100 22]);

        kb_modulator_freq = uiknob(fig);
        kb_modulator_freq.Limits = [0 20];
        kb_modulator_freq.Position=[50,100,60,60];
        
        kb_modulator_amp = uiknob(fig);
        kb_modulator_amp.Limits = [0 5];
        kb_modulator_amp.Position=[200,100,60,60];

        btn_play= uibutton(fig,'state','Text', 'play','Value', false,'Position',[60,210, 50, 22]);
       
        dd_carrier_mul = uidropdown(fig, 'Items',{'*10','*100','*1000'});
        dd_carrier_mul.ItemsData=[10,100,1000];
        dd_carrier_mul.Position=[150,300,20,20];

 

        dd_modulator_mul = uidropdown(fig, 'Items',{'*10','*100','*1000'});
        dd_modulator_mul.ItemsData=[10,100,1000];
        dd_modulator_mul.Position=[150,100,20,20];
        
    end



close all
end