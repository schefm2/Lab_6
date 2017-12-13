current_data = allthedata17;
time = .1*(1:size(current_data,1)); %coefficient reflects
                                    %each data point is 100 ms

current_data(:,6) = (current_data(:,6) - 2765)/(3502 - 2027)*200;
current_data(:,7) = (current_data(:,7) - 2835)/(3245 - 2425)*200;

close all
figure(1)
set(gcf,'Visible','on') %Automatically pop out the figure
clf
subplot(2,1,1)
hold on
plot(time,current_data(:,3),':k')
plot(time,current_data(:,1),'--k')
plot(time,current_data(:,2),'-k','LineWidth',1.5)
plot([.1 .1*size(current_data,1)],[0 0], '-k', 'LineWidth',.5)
xlabel('Time [s]')
ylabel('Tenths of a Degree')
% xlim([35 50]) %useful for looking in detail at ranger run
legend('Heading Error', 'Desired Heading','Current Heading', 'Location', 'southeast')

subplot(2,1,2)
hold on
plot(time,current_data(:,6),'-k','LineWidth',1.5)
plot(time,current_data(:,7),'x-k','MarkerIndices',(1:25:size(current_data,1)))
xlabel('Time [s]')
ylabel('Normalized Percentage, -100 to 100')
legend('Motor PW','Servo PW','Location','southeast')

%----------------------------------------------------------------
figure(2)
set(gcf,'Visible','on') %Automatically pop out the figure
clf

subplot(2,1,1)
plot(time,current_data(:,1),'-k')
legend('Desired Heading')
xlabel('Time [s]')
ylabel('Tenths of a Degree')

subplot(2,1,2)
plot(time,current_data(:,4),'-k')
xlabel('Time [s]')
ylabel('Range [cm]')
ylim([0 300])

%----------------------------------------------------------------
%Motor pulse width and its effect on battery voltage
figure(3)
set(gcf,'Visible','on')
clf

current_data(:,5) = current_data(:,5)/7500*2.4/.236;

subplot(2,1,1)
hold on
plot(time,current_data(:,6),'-k')
legend('Motor PW','Location','southeast')
xlabel('Time [s]')
ylabel('Normalized Percentage, -100 to 100')

subplot(2,1,2)
plot(time,current_data(:,5),'-k')
legend('Battery Voltage')
xlabel('Time [s]')
ylabel('Voltage [mV]')
% %----------------------------------------------------------------
% figure(4)
% set(gcf,'Visible','on')
% clf
% 
% hold on
% plot(time,current_data(:,2),'-k')
% plot(time,current_data(:,3),'--k')
% plot([0 max(time)],[1800 1800], ':k')
% plot([0 max(time)],[-1800 -1800], ':k')
% plot([0 max(time)],[3600 3600], ':k')
% legend('Actual Heading','Heading Error')
% xlabel('Time [s]')
% ylabel('Tenths of a Degree')

