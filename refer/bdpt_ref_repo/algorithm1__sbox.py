
class Sbox:
	def __init__(self, sbox):
		self.sbox = sbox
		self.SBOXSIZE = self.SboxSize()

	def SboxSize(self):
		s = format(len(self.sbox), "b")
		num_of_1_in_the_binary_experission_of_the_len_of_sbox = s.count("1")
		assert num_of_1_in_the_binary_experission_of_the_len_of_sbox == 1
		return (len(s) - 1)

	def BitProduct(self, u, x):
		if (u & x) == u:
			return 1
		else:
			return 0

	def GetTruthTable(self, u):
		temp = [u for i in range(len(self.sbox))]
		table = list(map(self.BitProduct, temp, self.sbox))
		return table

	def ProcessTable(self, table):
		for i in range(0, self.SBOXSIZE):
			for j in range(0, 2**i):
				for k in range(0, 2**(self.SBOXSIZE - 1 - i)):
					table[k + 2**(self.SBOXSIZE - 1 - i) + j*(2**(self.SBOXSIZE - i))] =\
	                table[k + 2**(self.SBOXSIZE - 1 - i) + j*(2**(self.SBOXSIZE - i))] ^\
	                table[k + j*(2**(self.SBOXSIZE - i))]

	def CreatANF(self):
		ANF = [[]for i in range(0, len(self.sbox))]
		for i in range(1, len(self.sbox)): 
			table = self.GetTruthTable(i)
			self.ProcessTable(table)
			sqr = []
			for j in range(0, len(self.sbox)):
				if table[j] != 0:
					sqr.append(j)
			ANF[i] = sqr
		return ANF

	def NEW_computer(self, x):
		sbox_size = self.SBOXSIZE ** 2
		n = []
		m = []
		all = [k for k in range(sbox_size)]
		for i in range(0,sbox_size):
			if (x | i) == x:
				n.append(i)
		m = [p for p in all if p not in n]
		return m

	def CreateDivisionTrails_K(self):
		"""
		Return all the division trails K of a given sbox
		"""
		ANF = self.CreatANF()
		INDP = []
	    # add zero vector into the division trails
		sqr = [0 for i in range(2 * self.SBOXSIZE)]
		INDP.append(sqr)
		# start from the non-zero vector
		for i in range(1, len(self.sbox)):
			sqn = []
			# start from the non-zero vector
			for j in range(1, len(self.sbox)):
				flag = False
				for entry in ANF[j]:
					if (i | entry) == entry:
						flag = True
						break
				if flag:
					sqn1 = []
					flag_add = True
					for t1 in sqn:
						if (t1 | j) == j:
							flag_add = False
							break
						elif (t1 | j) == t1:
							sqn1.append(t1)
					if flag_add:
						for t2 in sqn1:
							sqn.remove(t2)
						sqn.append(j)
			for num in sqn:
				a = format(i, "0256b")
				b = format(num, "0256b")
				a = list(reversed(list(map(int, list(a)))))
				b = list(reversed(list(map(int, list(b)))))
				a = a[0:self.SBOXSIZE]
				b = b[0:self.SBOXSIZE]
				a.reverse()
				b.reverse()
				INDP.append((a+b))
		return INDP

	def PrintfDivisionTrails_K(self, filename):
		"""
		Write all division trails K of an sbox into a file
		"""
		INDP = self.CreateDivisionTrails_K()
		fileobj = open(filename, "w")
		fileobj.write("Division Trails K of sbox:\n")
		for l in INDP:
			fileobj.write(str(l) + "\n")
		fileobj.write("\n")
		fileobj.close()
	def CreateDivisionTrails_L(self):
		"""
		Return all the division trails L of a given sbox
		"""
		ANF = self.CreatANF()
		S_N = []
		INDP = []
		# add zero vector into the division trails
		sqr = [0 for i in range(2 * self.SBOXSIZE)]
		INDP.append(sqr)
		# start from the non-zero vector
		for i in range(1, len(self.sbox)): 
			sqn = [] 
			# start from the non-zero vector
			for j in range(1, len(self.sbox)):
				p = []
				for entry in ANF[j]: 
					if entry == i:
						p.append(entry)
				if len(p) == 1:
					r = []
					for entry in ANF[j]:
						r.append(self.NEW_computer(entry))
					flag = set.intersection(*map(set,r))
					if len(flag) >= 1:
						sqn.append(j)
				else:
					continue
			for num in sqn:
				a = format(i, "0256b")
				b = format(num, "0256b")
				a = list(reversed(list(map(int, list(a)))))
				b = list(reversed(list(map(int, list(b)))))
				a = a[0:self.SBOXSIZE]
				b = b[0:self.SBOXSIZE]
				a.reverse()
				b.reverse()
				INDP.append((a+b))
		sqf = [1 for i in range(2 * self.SBOXSIZE)]
		INDP.append(sqf)
		return INDP

	def PrintfDivisionTrails_L(self, filename):
		"""
		Write all division trails L of an sbox into a file
		"""
		INDP = self.CreateDivisionTrails_L()
		fileobj = open(filename, "w")
		fileobj.write("Division Trails L of sbox:\n")
		for l in INDP:
			fileobj.write(str(l) + "\n")
		fileobj.write("\n")
		fileobj.close()

if __name__ == "__main__":
    
	cipher = "SIMON"
	sbox = [0x0, 0x1, 0x3, 0x2, 0x4, 0x5, 0x7, 0x6, 0x8, 0x9, 0xb, 0xa, 0xd, 0xc, 0xe, 0xf]

	simon = Sbox(sbox)

	filename_K = cipher + "_DivisionTrails_K.txt"
	filename_L = cipher + "_DivisionTrails_L.txt"
	simon.PrintfDivisionTrails_K(filename_K)
	simon.PrintfDivisionTrails_L(filename_L)

