%% sample routine to compare results of the testing, checking of data transmission



%%%%%% Here we load both HOST and ITFC data

% test data location from host
testdata = load('.\Examples\SCADA_reader\host_scada_.mat');

% load itfc data
itfc_loads = load('.\Examples\SCADA_reader\itfc_loads.mat');
itfc_loads = itfc_loads.test_ITFC;

itfc_mm = load('.\Examples\SCADA_reader\itfc_mm.mat');
itfc_mm = itfc_mm.test_ITFC;

itfc_scada = load('.\Examples\SCADA_reader\itfc_scada.mat');
itfc_scada = itfc_scada.test_ITFC;


%% plot comparison on desired quantity

y1 = testdata.outmat_fast.("WT1_GeneratorSpeed[RPM]");
y2 = itfc_scada.WT1_scada.GeneratorSpeed;


delay = finddelay(y1 - mean(y1), y2 - mean(y2));

sample_time_itfc = 0.1; % seconds
delay_s = seconds(sample_time_itfc*delay);

reference_time = testdata.outmat_fast.TimeStamp_UTC;

nElem = size(y2,1);
itfc_timestamp = seconds([0 : nElem-1]*sample_time_itfc) + reference_time(1) - delay_s;

figure(); hold on
plot(testdata.outmat_fast.TimeStamp_UTC, y1)
plot(itfc_timestamp - seconds(0.3), y2)
legend('host', 'itfc')

y1 = testdata.outmat_fast.("WT1_Power[kW]");
y2 = itfc_scada.WT1_scada.Power;
figure(); hold on
plot(testdata.outmat_fast.TimeStamp_UTC, y1)
plot(itfc_timestamp - seconds(0.3), y2)
legend('host', 'itfc')


