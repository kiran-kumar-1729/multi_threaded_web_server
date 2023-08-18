import matplotlib.pyplot as plt  
x =[]
f=open('points3.txt')
y=[]
c=1
for i in f:
	i=str(i)
	j=i.split(" ")
	x.append((int)(j[0]))
	c=c+1
	a=(float)((float)(j[1]))
	y.append(a)
plt.plot(x, y) 
plt.xlabel('load(# users in 1000s)') 
plt.ylabel('response time in ms') 
plt.title('Load vs Throughput') 
plt.show() 
