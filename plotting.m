 %A= textread('array.txt',"%f");
 %A= textread('2nd deriv.txt',"%f");
 x = textread('Line Fitting Result.txt',"%f");
 %A=ans
 %image_height= 1944;
 %image_width = 2592;
 %B= reshape(A,image_width,image_height);
 %B=B.';
 
  % x=1: 1: 2592;   
 %plot(x, B(1,:));
 hist(x,-20:1: 20,"facecolor","b","edgecolor","k");  