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
plot(time,current_data(:,5),'-k')
xlabel('Time [s]')
ylabel('Battery Voltage [mV]')

subplot(2,1,2)
plot(time,current_data(:,4),'-k')
xlabel('Time [s]')
ylabel('Range [cm]')
ylim([0 300])

%----------------------------------------------------------------
%For the data with the fans turned off and derivative gain set to 0
figure(3)
set(gcf,'Visible','on')
clf

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

