from cProfile import label
import matplotlib.pyplot as plt


x = []
y = []
with open('n=36.txt') as archivo: #Cambiar el nombre del archivo correspondiente
    for linea in archivo:
        punto = [float(i) for i in linea.split(',')]
        x.append(punto[0])
        y.append(punto[1])
plt.plot(x, y, "o--", label="N=36")
plt.legend(loc="best" )

x = []
y = []
with open('n=576.txt') as archivo:  # Cambiar el nombre del archivo correspondiente
    for linea in archivo:
        punto = [float(i) for i in linea.split(',')]
        x.append(punto[0])
        y.append(punto[1])
plt.plot(x, y, "o--", label="N=576")
plt.legend(loc="best" )


x = []
y = []
with open('n=1440.txt') as archivo:  # Cambiar el nombre del archivo correspondiente
    for linea in archivo:
        punto = [float(i) for i in linea.split(',')]
        x.append(punto[0])
        y.append(punto[1])
plt.plot(x, y, "o--", label="N=14400")
plt.legend(loc="best" )


# plt.xscale("log") 
# plt.yscale("log")
# # plt.ylim(0, 123000)
# plt.ylabel("Tiempo en ms")
# plt.xlabel("Cantidad de vertices")
# plt.title("Kruskal: HashGraph")
# plt.savefig("HashGraph.jpg")
# plt.show()


# plt.xscale("log")
# plt.yscale("log")
# # plt.ylim(0, 123000)
# plt.ylabel("Tiempo en ms")
# plt.xlabel("Cantidad de vertices")
# plt.title("Kruskal: ListGraph")
# plt.savefig("ListGraph.jpg")
# plt.show()



plt.xscale("log")
plt.yscale("log")
# plt.ylim(0, 123000)
plt.ylabel("Tejec (seg)")
plt.xlabel("#Process")
plt.title("Quicksort en paralelo - Eficiencia")
plt.savefig("/Users/paolovasquezgrahammer/desktop/quicksort_eficiencia")
plt.show()

#Cambiar los respectivos nombre de acuerdo a la conveniencia