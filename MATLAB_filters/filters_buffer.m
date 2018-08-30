function filters_buffer
%AUTHOR: Tim Lindquist
%   Program simulates filter frequency response 2-20k Hz audible range
%   using parameters from digital pot datasheet

close all 
R0=1120; %wiper resistance
R1=88200;   %resistance range
C=1.3E-7;   %chosen capacitor for optimal 2-20k range: 1/(RC)

fig = uifigure;
fig.Position=[200,500,200,400];
ui();

s=tf('s');
R_LP_prev=(100-kb_LP.Value - 0) * (R1 - R0) / (100 - 0) + R0+1;
R_HP_prev=(100-kb_HP.Value - 0) * (R1 - R0) / (100 - 0) + R0;
order_LP_prev=dd_LP.Value;
order_HP_prev=dd_HP.Value;
series_prev=btn_series.Value;
parallel_prev=btn_parallel.Value;
LP_enable_prev=btn_LP.Value;
HP_enable_prev=btn_HP.Value;


while(1)
    pause(0.1);
    R_LP=(100-kb_LP.Value - 0) * (R1 - R0) / (100 - 0) + R0;
    R_HP=(100-kb_HP.Value - 0) * (R1 - R0) / (100 - 0) + R0;
    order_LP=dd_LP.Value;
    order_HP=dd_HP.Value;
    series=btn_series.Value;
    parallel=btn_parallel.Value;
    LP_enable=btn_LP.Value;
    HP_enable=btn_HP.Value;
    
    if(series~=series_prev)
        btn_parallel.Value=~series;
        parallel=btn_parallel.Value;
    end
    
    if(parallel~=parallel_prev)
        btn_series.Value=~parallel;
        series=btn_series.Value;
    end
    
    if(R_LP~=R_LP_prev | R_HP~=R_HP_prev | order_LP~=order_LP_prev | order_HP~=order_HP_prev | series~=series_prev | parallel~=parallel_prev | LP_enable~=LP_enable_prev | HP_enable~=HP_enable_prev)
        R_LP_prev=R_LP;
        R_HP_prev=R_HP;
        order_LP_prev=order_LP;
        order_HP_prev=order_HP;
        series_prev=series;
        parallel_prev=parallel;
        LP_enable_prev=LP_enable;
        HP_enable_prev=HP_enable;
        
        clf
        %lowpass
        LP=1;
        LP_1st=1/(1+s*C*R_LP);
        if LP_enable
            for i=1:order_LP
                LP=LP*LP_1st;
            end
        end
        
        %highpass
        HP=1;
        HP_1st=(s*C*R_HP)/(1+s*C*R_HP);
        if HP_enable
            for i=1:order_HP
                HP=HP*HP_1st;
            end
        end
        
        %presetup
        if LP_enable 
            TF=LP;
        elseif HP_enable
            TF=HP;
        else
            TF=0;
        end
        
        %series or parallel
        if series %output of LP fed into input of HP
            if LP_enable & HP_enable    
                TF=LP*HP;
            end
        else %parallel LP/HP connection through averageing amplifier
            if LP_enable & HP_enable    
                TF=(LP+HP)/2.0;
            end
        end
        hold on
        if LP_enable | HP_enable
            bodemag(TF,{50,130000});
            set(findall(gcf,'type','line'),'linewidth',5);
            if LP_enable
                bodemag(LP,{50,130000});
            end
            if HP_enable
                bodemag(HP,{50,130000});
            end
            
            setoptions(gcr,'FreqUnits','Hz')
            if LP_enable & HP_enable
                legend('Output','Low Pass','High Pass');
            elseif LP_enable
                legend('Output','Low Pass');
            elseif HP_enable
                legend('Output','High Pass');
            end

            ylim([-50,10]);

        end
        hold off  
    end
   
end





    function ui
        kb_LP = uiknob(fig);
        kb_LP.Position=[50,300,60,60];

        dd_LP = uidropdown(fig, 'Items',{'1st Order','2nd Order','3rd Order','4th Order'});
        dd_LP.ItemsData=[1,2,3,4];
        dd_LP.Position=[150,300,20,20];

        kb_HP = uiknob(fig);
        kb_HP.Position=[50,100,60,60];

        dd_HP = uidropdown(fig, 'Items',{'1st Order','2nd Order','3rd Order','4th Order'});
        dd_HP.ItemsData=[1,2,3,4];
        dd_HP.Position=[150,100,20,20];
        
        btn_series= uibutton(fig,'state','Text', 'series','Value', true,'Position',[25,210, 50, 22]);
        btn_parallel= uibutton(fig,'state','Text', 'parallel','Value', false,'Position',[100,210, 50, 22]);

        btn_LP= uibutton(fig,'state','Text', 'low pass filter','Value', true,'Position',[35 250 100 22]);
        btn_HP= uibutton(fig,'state','Text', 'high pass filter','Value', false,'Position',[35 50 100 22]);
        
    end



close all
end






