import sys

def esLogLine(L):
	a = ("MITRACE" in L)
	b = (L == "-"*42)
	c = (len(L)==1)
	d = ("=>" in L)
	e = "L"=="\n"
	return a or b or c or d or e

def isLineFunction(L):	
	return ";" in L.split()[-1]

def getNameOfLineFunction(L):
	return L.split()[-1]

def getTimeOfLineFunction(L):
	return float(L.split()[1])




def isOpenFunction(L):
	return "{" == L.split()[-1]
def getNameOpenFunction(L):
	return L.split()[2]

def isCloseFunction(L):
	return "}" == L.split()[-1]

def getTotalTime(diccionario):
	total = 0
	for k in diccionario:
		total = total + diccionario[k]
	return total

def getResumen(filename):
	archivo = open(filename,"r")
	diccionario = {}

	funcionPendiente = ""
	for linea in archivo:
		linea = linea.replace("+"," ")
		linea = linea.replace("!"," ")
		if not esLogLine(linea):
			if isLineFunction(linea):
				if not getNameOfLineFunction(linea) in diccionario:
					diccionario[getNameOfLineFunction(linea)] = 0
				diccionario[getNameOfLineFunction(linea)] = diccionario[getNameOfLineFunction(linea)] + getTimeOfLineFunction(linea)

			if isOpenFunction(linea):
				funcionPendiente = getNameOpenFunction(linea)
				#print "get: ",funcionPendiente

			if isCloseFunction(linea):
				#print "set: ",funcionPendiente
				if funcionPendiente not in diccionario:
					diccionario[funcionPendiente] = 0
				diccionario[funcionPendiente] = diccionario[funcionPendiente] + getTimeOfLineFunction(linea)
				funcionPendiente = ""
	archivo.close()
	return diccionario


import glob, os


if(len(sys.argv)<2):
	print "Error. Uso: python script.py rutaArchivos"
	exit()

#files = glob.glob(os.getcwd()+"/*.txt")
files = glob.glob(sys.argv[1]+"/*.txt")
print files

totaldict = {}

for file in sorted(files):
	filename = file.split("/")[-1]
	threads = int(filename.split("_")[1].split("threads")[0])
	datos = getResumen(file)

	#import pprint
	#pprint.pprint(datos, width=1)
	
	# Para imprimir el resumen de total de tiempos en llamdas de bloqueo
	#print threads, getTotalTime(datos)

	# Para construir el detallado por tiempo por funcion
	for funcion in datos:
		if funcion not in totaldict:
			totaldict[funcion] = {1:0,2:0,4:0,8:0,16:0,24:0,36:0,48:0,64:0,128:0}
		totaldict[funcion][threads] = datos[funcion]

#para imprimir el detallado
for fun in totaldict:
	print fun,",",
	for threads in sorted(totaldict[fun]):
		print totaldict[fun][threads],",",
	print ""

#import pprint
#pprint.pprint(totaldict, width=1)