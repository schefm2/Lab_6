current_data = finaldata001;
time = .1*(1:size(current_data,1)); %coefficient reflects
                                    %each data point is 100 ms

close all
figure(1)
set(gcf,'Visible','on') %Automatically pop out the figure
clf
subplot(2,1,1)
hold on
plot(time,current_data(:,3),'-k','LineWidth',1.5)
plot(time,current_data(:,1),'--k')
plot([0 32],[0 0], '-.k')
legend('Heading Error', 'Desired Heading', 'Location', 'southeast')

subplot(2,1,2)
hold on
plot(time,current_data(:,6),'-k','LineWidth',1.5)
plot(time,current_data(:,7),'x-k','MarkerIndices',(1:25:size(current_data,1)))
legend('Motor PW','Servo PW','Location','southeast')

%----------------------------------------------------------------
figure(2)
set(gcf,'Visible','on') %Automatically pop out the figure
clf

subplot(2,1,1)
plot(time,current_data(:,5),'-k')
legend('Battery Voltage','Location','southeast')

subplot(2,1,2)
plot(time,current_data(:,4),'-k')
legend('Range','Location','southeast')
ylim([0 300])

%----------------------------------------------------------------
%For the data with the fans turned off and derivative gain set to 0
figure(3)
set(gcf,'Visible','on')
clf

subplot(2,1,1)
hold on
plot(time,current_data(:,2),'-k',time,current_data(:,1),'--k')
plot(time,current_data(:,3),'x-.k','MarkerIndices',(1:25:size(current_data,1)))
legend('Actual Heading','Desired Heading','Heading Error',...
    'Location','southeast')

subplot(2,1,2)
plot(time,current_data(:,6),'-k')
legend('Motor PW')
xlabel('Time [s]')

