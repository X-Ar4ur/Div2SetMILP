
from sbox import Sbox
import copy

class MILP_Reduce_Model(Sbox):
	def __init__(self, filename, ine, sbox):
		Sbox.__init__(self, sbox)
		self.filename = filename
		self.ine = ine
		self.divtrial = self.CreateDivisionTrails_K()	

	@staticmethod
	def init_R(x,y,real_n):
		N1=len(x)
		N2=len(y)
		R = []
		for i in range(N2):
			R.append([])

		for j in range(N2):
			for i in range(N1):
				flag1 = 0
				for k in range(real_n):
					flag1 +=  y[j][k]*x[i][k]
				flag1 +=  x[i][real_n]
				if flag1 < 0 :
					R[j].append(i)
		return R

	@staticmethod
	def Integer2Bitlist(n, l):
		s = list(map(int, list(format(n, "0256b"))))
		s = s[len(s) - l :]
		return s

	def InequalitySizeReduce(self):
		points = self.divtrial
		inequalities = self.ine
		assert len(points) > 0
		assert len(inequalities) > 0
		assert len(points[0]) + 1 == len(inequalities[0])
		length = len(points[0])
		apoints = [MILP_Reduce_Model.Integer2Bitlist(i, length) for i in range(2**length)]
		cpoints = [p for p in apoints if p not in points]
		ineq = copy.deepcopy(inequalities)
		R = MILP_Reduce_Model.init_R(ineq,cpoints,length)
		lp_filename = self.filename + "_select.lp"
		f = open(lp_filename,"w")
		f.write("Minimize"+'\n')
		len_inequality = len(ineq)
		f.write(' + '. join ( ['z' + str(i) for i in range (len_inequality)] )+'\n')
		f.write('\n')
		f.write('Subject To'+'\n')
		for i in range(len(R)):
			f.write(' + '. join ( ['z' + str(j) for j in R[i]] )+" >= 1"+'\n')
		f.write('\n')
		f.write('Binary'+'\n')
		for v in range (len_inequality):
			f.write('z' +str(v)+'\n')
		f.write('End'+'\n')
		f.close()

if __name__ == "__main__":
	
	sbox = [0x0, 0x1, 0x3, 0x2, 0x4, 0x5, 0x7, 0x6, 0x8, 0x9, 0xb, 0xa, 0xd, 0xc, 0xe, 0xf]
	cipher = "SIMON"
	sbox_division = Sbox(sbox)
	Hull=Polyhedron(sbox_division.CreateDivisionTrails_K())
	ine=[]
	for v in Hull.inequalities_list():
		v=v[1:]+ v[:1]
		ine.append(v)
	filename = cipher+"_Inequalities.txt"
	fileobj = open(filename, "w")
	for l in ine:
		fileobj.write(str(l) + "\n")
	fileobj.close()
	Reduce_Model = MILP_Reduce_Model(cipher, ine, sbox)
	Reduce_Model.InequalitySizeReduce()

