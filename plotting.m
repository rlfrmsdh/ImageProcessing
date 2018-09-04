 %A= textread('array.txt',"%f");
 %A= textread('2nd deriv.txt',"%f");
 A_cross = textread('Fmeas_cross.txt',"%f");
A_ver = textread('Fmeas_ver.txt',"%f");
 %A = textread('Fmeas.txt',"%f");

 %image_height= 1944;
 %image_width = 2592;
 %B= reshape(A,image_width,image_height);
  B_cross= reshape(A_cross,size(A_cross)/2,2) ;
  B_ver = reshape(A_ver,size(A_ver)/2, 2);
% B = reshape(A, size(A)/2,2) ;
 % x=1: 1: 2592;   
% plot(x, B(1000,:));
 %hist(x,-20:1: 20,"facecolor","b","edgecolor","k");  
 
 figure();
 plot(B_cross(:,2),B_cross(:,1)) ;
 hold on;
 plot(B_ver(:,2),B_ver(:,1));
 
 legend('using all','using vertical');