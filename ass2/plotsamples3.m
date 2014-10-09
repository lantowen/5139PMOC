data = csvread('parsedresults3.txt',1);
tidx = data(:,1) == 0;
pidx = data(:,1) == 1;
tdata = data(tidx,:);
pdata = data(pidx,:);
for r = 1:10
    rate = r * 10;
    tsamples{r} = tdata(tdata(:,2) == rate,:);
    psamples{r} = pdata(pdata(:,2) == rate,:);
end

for r = 1:10
    rate = r * 10;
    tmedians(r) = median(tsamples{r}(:,4));
    pmedians(r) = median(psamples{r}(:,4));
end

X =  10:10:100;

% Mean Square Error 
tActual = tsamples{10}(:,7);
pActual = psamples{10}(:,7);

for r = 1:10
    rate = r * 10;
    factor = 1/((rate/100)^2);
    tMSE(r) = sum((tsamples{r}(:,7)*factor-tActual).^2)/10;
    pMSE(r) = sum((psamples{r}(:,7)*factor-pActual).^2)/10;
end
figure;
set(gca,'FontSize',14);

semilogy(X, tMSE, 'blue');
hold on;
semilogy(X, pMSE, 'red');

legend('Tuple-based','Page-based', 'Location', 'northeast');
xlabel('Sample rate (%)');
ylabel('Mean Square Error of Est. Count (count/(rate^2))');
title('Accuracy of |R join S| = ~25000 Using Sampling');

hold off;


% Performance vs Sample Rate
figure;
hold on;
set(gca,'FontSize',14);

scatter(tdata(:,2), tdata(:,4), 'blue');
scatter(pdata(:,2), pdata(:,4), 'red', 'x');

plot(X, tmedians, 'blue');
plot(X, pmedians, 'red');

legend('Tuple-based','Page-based', 'Location', 'northwest');
xlabel('Sample rate (%)');
ylabel('Time taken (ms)');
title('Performance of |R join S| = ~25000 Using Sampling');
hold off

