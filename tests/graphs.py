import matplotlib.pyplot as plt
import time
import os
from subprocess import call
import subprocess
import sys

x = [i for i in range(31)]

Y = []

Paths = []

# Chemins absolus
library = os.getcwd() + "/build"

for i in sys.argv[1:]:
    Paths.append(i)

# Ajout de la variable d'environnement "LD_LIBRARY_PATH"
my_env = os.environ.copy()
my_env["LD_LIBRARY_PATH"] = library + ":" + my_env["PATH"]

for p in Paths:
    Y = []
    for i in x:
        Y.append(0)
        #En moyenne sur 3 exécutions, modifier la boucle for suivant pour avoir
        #une seule exécution
        for j in range(3):
            start = time.time()
            subprocess.run([os.getcwd() + "/"  + p, str(i)], env=my_env)
            end = time.time()
            Y[-1] += (end - start)/3

    #Trace les courbes
    plt.plot(x, Y, label=p)



#Ajoute une échelle logarithmique pour l'axe des ordonnées
plt.yscale("log")

#Ajoute une grille sur le graphe
plt.grid(True, which="both", linestyle='--')

#Ajoute une légende
plt.legend("time measurement of fibo")

#Affiche les courbes
plt.show()
