from gurobipy import *
import time

class Simon:
	def __init__(self, Round, activebits, word_len, insert, i):
		self.Round = Round
		self.activebits = activebits
		self.blocksize = 2 * word_len
		self.WORD_LENGTH = word_len
		self.insert = insert
		self.i = i
		self.filename_model = "Simon_" + str(word_len) + "_" + str(self.Round) + "_" + str(self.activebits) + "_" + str(self.insert) + "constraint.lp"
		self.filename_result = "result_" + str(word_len) + "_" + str(self.Round) + "_" + str(self.activebits) + "_" + str(self.insert) + "constraint.txt"
		fileobj = open(self.filename_model, "w")
		fileobj.close()
		fileboj = open(self.filename_result, "w")
		fileobj.close()

	R1 = 1
	R2 = 8
	R3 = 2

	S_T=[[0, -1, -1, -1, 0, 1, 1, 1, 0],\
	[1, 0, 0, 0, -1, 0, 0, 0, 0],\
	[0, 1, 0, 0, 0, -1, 0, 0, 0],\
	[-1, 0, -1, -1, 1, 0, 1, 1, 0],\
	[0, 0, 1, 0, 0, 0, -1, 0, 0],\
	[1, -1, 0, 0, 0, 1, 0, 0, 0],\
	[1, 0, 1, 1, 0, 0, 0, -1, 0],\
	[-1, 1, 0, 0, 1, 0, 0, 0, 0],\
	[0, 1, 1, 1, 0, 0, 0, -1, 0],\
	[0, 0, 0, 1, -1, -1, -1, -1, 3]]
	S_T_K=[[-1, 0, -1, -1, 1, 0, 1, 1, 0],\
	[0, -1, -1, -1, 0, 1, 1, 1, 0],\
	[0, 0, 1, 1, -1, 0, -1, -1, 1],\
	[0, 0, 1, 1, 0, -1, -1, -1, 1],\
	[1, 1, 1, 1, -1, -1, -1, -1, 0],\
	[0, 0, 1, 0, 0, 0, -1, 0, 0]]

	NUMBER = 9

	def CreateVariable(self, n, x):  
		variable = []
		for i in range(0, self.WORD_LENGTH):
			variable.append(x + "_" + str(n) + "_" + str(i))
		return variable

	def CreateVariable_for_uv(self, n, m, x):  
		variable = []
		for i in range(0, self.WORD_LENGTH):
			variable.append(x + "_" + str(n) + "_" + str(m) + "_" + str(i))
		return variable

	def CreateObjectiveFunction(self):
		fileobj = open(self.filename_model, "a")
		fileobj.write("Minimize\n")
		eqn = []
		for i in range(0,self.WORD_LENGTH):
			eqn.append("x" + "_" + str(self.Round) + "_" + str(i))
		for j in range(0,self.WORD_LENGTH):
			eqn.append("y" + "_" + str(self.Round) + "_" + str(j))
		temp = " + ".join(eqn)
		fileobj.write(temp)
		fileobj.write("\n")
		fileobj.close()

	def VariableRotation(self, x, n):
		eqn = []
		for i in range(0, self.WORD_LENGTH):
			eqn.append(x[(i + n) % self.WORD_LENGTH])
		return eqn
	def VariableRotation_constant(self, n):
		eqn = []
		for i in range(0, self.WORD_LENGTH):
			eqn.append((i + n) % self.WORD_LENGTH)
		return eqn
			
	def ConstraintsBySbox_L(self, variable_x, variable_y, variable_x_next, variable_y_next, n):
		fileobj = open(self.filename_model,"a")
		variable_1 = self.VariableRotation(variable_x , Simon.R1)
		variable_8 = self.VariableRotation(variable_x , Simon.R2)
		variable_2 = self.VariableRotation(variable_x , Simon.R3)
		list_res = [[] for i in range(self.WORD_LENGTH)]
		list_1 = self.VariableRotation_constant(Simon.R1)
		list_2 = self.VariableRotation_constant(Simon.R2)
		list_3 = self.VariableRotation_constant(Simon.R3)
		list_constant = [1 for j in range(self.WORD_LENGTH)]
		list_constant_constraint = self.CreateVariable(n, "c") 
		variable_u_constraint = self.CreateVariable_for_uv(n, self.WORD_LENGTH-1, "u") 
		variable_v_constraint = self.CreateVariable_for_uv(n, self.WORD_LENGTH-1, "v") 
		for i in range(self.WORD_LENGTH):
			list_res[i].append(list_1[self.WORD_LENGTH-1-i])
			list_res[i].append(list_2[self.WORD_LENGTH-1-i])
			list_res[i].append(list_3[self.WORD_LENGTH-1-i])
		for k in range(0,self.WORD_LENGTH): 
			variable_u = self.CreateVariable_for_uv(n, k, "u")
			variable_v = self.CreateVariable_for_uv(n, k, "v")
			for coff in Simon.S_T:
				temp = []
				if k == 0:
					for u in range(0,4):
						if u == 0:
							temp.append(str(coff[u]) + " " + variable_1[self.WORD_LENGTH-1-k])
						elif u == 1:
							temp.append(str(coff[u]) + " " + variable_8[self.WORD_LENGTH-1-k])
						elif u == 2:
							temp.append(str(coff[u]) + " " + variable_2[self.WORD_LENGTH-1-k])
						else:
							temp.append(str(coff[u]) + " " + variable_y[self.WORD_LENGTH-1-k])
				else:
					for u in range(0,4):
						if u == 0:
							temp.append(str(coff[u]) + " " + variable_u_in[list_res[k][u]])
						elif u == 1:
							temp.append(str(coff[u]) + " " + variable_u_in[list_res[k][u]])
						elif u == 2:
							temp.append(str(coff[u]) + " " + variable_u_in[list_res[k][u]])
						else:
							temp.append(str(coff[u]) + " " + variable_v_in[self.WORD_LENGTH-1-k])
				for v in range(0,4):
					if v == 0:
						temp.append(str(coff[v + 4]) + " " + variable_u[(self.WORD_LENGTH-1-k +Simon.R1) % self.WORD_LENGTH])
					elif v == 1:
						temp.append(str(coff[v + 4]) + " " + variable_u[(self.WORD_LENGTH-1-k +Simon.R2) % self.WORD_LENGTH])
					elif v == 2:
						temp.append(str(coff[v + 4]) + " " + variable_u[(self.WORD_LENGTH-1-k +Simon.R3) % self.WORD_LENGTH])
					else:
						temp.append(str(coff[v + 4]) + " " + variable_v[self.WORD_LENGTH-1-k])
				temp1 = " + ".join(temp)
				temp1 = temp1.replace("+ -", "- ")
				s = str(-coff[Simon.NUMBER - 1])
				s = s.replace("--", "")
				temp1 += " >= " + s
				fileobj.write(temp1)
				fileobj.write("\n")
			
			list_all = [j for j in range(self.WORD_LENGTH)] 
			list_index = [p for p in list_all if p not in list_res[k]]
			for i in list_index:
				if k == 0:
					Equation = []
					Equation.append(variable_x[i])
					Equation.append(variable_u[i])
					temp = " - ".join(Equation)
					temp = temp + " = " + str(0)
					fileobj.write(temp)
					fileobj.write("\n")
				else: 
					Equation = []
					Equation.append(variable_u_in[i])
					Equation.append(variable_u[i])
					temp = " - ".join(Equation)
					temp = temp + " = " + str(0)
					fileobj.write(temp)
					fileobj.write("\n")
			list_index_2 = [p for p in list_all if p != (self.WORD_LENGTH-1-k)]	
			for j in list_index_2:
				if k == 0:
					equ = []
					equ.append(variable_y[j])
					equ.append(variable_v[j])
					temp = " - ".join(equ)
					temp = temp + " = " + str(0)
					fileobj.write(temp)
					fileobj.write("\n")	
				else:
					equ = []
					equ.append(variable_v_in[j])
					equ.append(variable_v[j])
					temp = " - ".join(equ)
					temp = temp + " = " + str(0)
					fileobj.write(temp)
					fileobj.write("\n")	
			variable_u_in = variable_u
			variable_v_in = variable_v
		if n == self.insert-1 :
			temp = " + ".join(variable_v_constraint)
			temp = temp + " <= " + str(self.WORD_LENGTH-1)
			fileobj.write(temp)
			fileobj.write("\n")
		for i in range(self.WORD_LENGTH):
			eqn = []
			eqn.append(variable_u[i])
			eqn.append(variable_y_next[i])
			temp = " - ".join(eqn)
			temp = temp + " = " + str(0)
			fileobj.write(temp)
			fileobj.write("\n")
		if n == self.insert-1 :
			for i in range(self.WORD_LENGTH):
				eqn = []
				eqn.append(variable_x_next[i])
				eqn.append(str(list_constant_constraint[i]))
				temp = " - ".join(eqn)
				temp = temp + " = " + str(0)
				fileobj.write(temp)
				fileobj.write("\n")	
		else:
			for i in range(self.WORD_LENGTH):
				eqn = []
				eqn.append(variable_v[i])
				eqn.append(variable_x_next[i])
				temp = " - ".join(eqn)
				temp = temp + " = " + str(0)
				fileobj.write(temp)
				fileobj.write("\n")	
		fileobj.close()
	def ConstraintsBySbox_k(self, variable_x, variable_y, variable_x_next, variable_y_next, n):
		fileobj = open(self.filename_model,"a")
		variable_1 = self.VariableRotation(variable_x , Simon.R1)
		variable_8 = self.VariableRotation(variable_x , Simon.R2)
		variable_2 = self.VariableRotation(variable_x , Simon.R3)
		list_res = [[] for i in range(self.WORD_LENGTH)]
		list_1 = self.VariableRotation_constant(Simon.R1)
		list_2 = self.VariableRotation_constant(Simon.R2)
		list_3 = self.VariableRotation_constant(Simon.R3)
		for i in range(self.WORD_LENGTH):
			list_res[i].append(list_1[self.WORD_LENGTH-1-i])
			list_res[i].append(list_2[self.WORD_LENGTH-1-i])
			list_res[i].append(list_3[self.WORD_LENGTH-1-i])
		for k in range(0,self.WORD_LENGTH):
			variable_u = self.CreateVariable_for_uv(n, k, "u")
			variable_v = self.CreateVariable_for_uv(n, k, "v")
			for coff in Simon.S_T_K:
				temp = []
				if k == 0:
					for u in range(0,4):
						if u == 0:
							temp.append(str(coff[u]) + " " + variable_1[self.WORD_LENGTH-1-k])
						elif u == 1:
							temp.append(str(coff[u]) + " " + variable_8[self.WORD_LENGTH-1-k])
						elif u == 2:
							temp.append(str(coff[u]) + " " + variable_2[self.WORD_LENGTH-1-k])
						else:
							temp.append(str(coff[u]) + " " + variable_y[self.WORD_LENGTH-1-k])
				else:
					for u in range(0,4):
						if u == 0:
							temp.append(str(coff[u]) + " " + variable_u_in[list_res[k][u]])
						elif u == 1:
							temp.append(str(coff[u]) + " " + variable_u_in[list_res[k][u]])
						elif u == 2:
							temp.append(str(coff[u]) + " " + variable_u_in[list_res[k][u]])
						else:
							temp.append(str(coff[u]) + " " + variable_v_in[self.WORD_LENGTH-1-k])
				for v in range(0,4):
					if v == 0:
						temp.append(str(coff[v + 4]) + " " + variable_u[(self.WORD_LENGTH-1-k +Simon.R1) % self.WORD_LENGTH])
					elif v == 1:
						temp.append(str(coff[v + 4]) + " " + variable_u[(self.WORD_LENGTH-1-k +Simon.R2) % self.WORD_LENGTH])
					elif v == 2:
						temp.append(str(coff[v + 4]) + " " + variable_u[(self.WORD_LENGTH-1-k +Simon.R3) % self.WORD_LENGTH])
					else:
						temp.append(str(coff[v + 4]) + " " + variable_v[self.WORD_LENGTH-1-k])
				temp1 = " + ".join(temp)
				temp1 = temp1.replace("+ -", "- ")
				s = str(-coff[Simon.NUMBER - 1])
				s = s.replace("--", "")
				temp1 += " >= " + s
				fileobj.write(temp1)
				fileobj.write("\n")
			
			list_all = [j for j in range(self.WORD_LENGTH)] 
			list_index = [p for p in list_all if p not in list_res[k]]
			for i in list_index:
				if k == 0:
					Equation = []
					Equation.append(variable_x[i])
					Equation.append(variable_u[i])
					temp = " - ".join(Equation)
					temp = temp + " = " + str(0)
					fileobj.write(temp)
					fileobj.write("\n")
				else:
					Equation = []
					Equation.append(variable_u_in[i])
					Equation.append(variable_u[i])
					temp = " - ".join(Equation)
					temp = temp + " = " + str(0)
					fileobj.write(temp)
					fileobj.write("\n")
			list_index_2 = [p for p in list_all if p != (self.WORD_LENGTH-1-k)]	
			for j in list_index_2:
				if k == 0:
					equ = []
					equ.append(variable_y[j])
					equ.append(variable_v[j])
					temp = " - ".join(equ)
					temp = temp + " = " + str(0)
					fileobj.write(temp)
					fileobj.write("\n")	
				else:
					equ = []
					equ.append(variable_v_in[j])
					equ.append(variable_v[j])
					temp = " - ".join(equ)
					temp = temp + " = " + str(0)
					fileobj.write(temp)
					fileobj.write("\n")	
			variable_u_in = variable_u
			variable_v_in = variable_v
		for i in range(self.WORD_LENGTH):
			eqn = []
			eqn.append(variable_u[i])
			eqn.append(variable_y_next[i])
			temp = " - ".join(eqn)
			temp = temp + " = " + str(0)
			fileobj.write(temp)
			fileobj.write("\n")
		for i in range(self.WORD_LENGTH):
			eqn = []
			eqn.append(variable_v[i])
			eqn.append(variable_x_next[i])
			temp = " - ".join(eqn)
			temp = temp + " = " + str(0)
			fileobj.write(temp)
			fileobj.write("\n")	
		fileobj.close()
	def Init(self):
		assert(self.activebits < (2 * self.WORD_LENGTH))
		fileobj = open(self.filename_model, "a")
		x = self.CreateVariable(0,"x")
		y = self.CreateVariable(0,"y")
		c = self.CreateVariable(self.insert-1, "c") 
		if self.activebits <= self.WORD_LENGTH:
			for i in range(0,self.activebits):
				fileobj.write((y[(self.WORD_LENGTH - 1 - i) % self.WORD_LENGTH] + " = " + str(1)))
				fileobj.write("\n")
			for i in range(self.activebits,self.WORD_LENGTH):
				fileobj.write((y[(self.WORD_LENGTH - 1 - i) % self.WORD_LENGTH] + " = " + str(0)))
				fileobj.write("\n")
			for i in range(0,self.WORD_LENGTH):
				fileobj.write((x[(self.WORD_LENGTH - 1 - i) % self.WORD_LENGTH] + " = " + str(0)))
				fileobj.write("\n")
		else:
			if self.i < self.WORD_LENGTH:
				for i in range(0, self.i):
					fileobj.write((x[i] + " = " + str(1)))
					fileobj.write("\n")
				fileobj.write((x[self.i] + " = " + str(0)))
				fileobj.write("\n")
				for i in range(self.i + 1, self.WORD_LENGTH):
					fileobj.write((x[i] + " = " + str(1)))
					fileobj.write("\n")
				for i in range(0, self.WORD_LENGTH):
					fileobj.write((y[(self.WORD_LENGTH - 1 - i) % self.WORD_LENGTH] + " = " + str(1)))
					fileobj.write("\n")
			else:
				for i in range(0, self.WORD_LENGTH):
					fileobj.write((x[(self.WORD_LENGTH - 1 - i) % self.WORD_LENGTH] + " = " + str(1)))
					fileobj.write("\n")
				for i in range(0, self.i % self.WORD_LENGTH):
					fileobj.write((y[i] + " = " + str(1)))
					fileobj.write("\n")
				fileobj.write((y[self.i % self.WORD_LENGTH] + " = " + str(0)))
				fileobj.write("\n")
				for i in range(self.i % self.WORD_LENGTH + 1, self.WORD_LENGTH):
					fileobj.write((y[i] + " = " + str(1)))
					fileobj.write("\n")
		for i in range(0,self.WORD_LENGTH):
			fileobj.write((c[i] + " = " + str(1)))
			fileobj.write("\n")
		fileobj.close()


	def CreateConstraints(self):
		assert(self.Round >= 1)
		fileobj = open(self.filename_model, "a")
		fileobj.write("Subject To\n")
		fileobj.close()
		x_in = self.CreateVariable(0,"x")
		y_in = self.CreateVariable(0,"y")
		x_in_insert = self.CreateVariable(self.insert,"x")
		y_in_insert = self.CreateVariable(self.insert,"y")
		for i in range(0, self.insert):
			x_out = self.CreateVariable((i+1), "x")
			y_out = self.CreateVariable((i+1), "y")
			self.ConstraintsBySbox_L(x_in, y_in, x_out, y_out, i)
			x_in = x_out
			y_in = y_out
		for i in range(self.insert, self.Round):
			x_out_insert = self.CreateVariable((i+1), "x")
			y_out_insert = self.CreateVariable((i+1), "y")
			self.ConstraintsBySbox_k(x_in_insert, y_in_insert, x_out_insert, y_out_insert, i)
			x_in_insert = x_out_insert
			y_in_insert = y_out_insert

	def BinaryVariable(self):
		fileobj = open(self.filename_model, "a")
		fileobj.write("Binary\n")
		for i in range(0, self.Round):
			for j in range(0, self.WORD_LENGTH):
				fileobj.write(("x_" + str(i) + "_" + str(j)))
				fileobj.write("\n")
			for j in range(0, self.WORD_LENGTH):
				fileobj.write(("y_" + str(i) + "_" + str(j)))
				fileobj.write("\n")
			for j in range(0, self.WORD_LENGTH):
				for p in range(0, self.WORD_LENGTH):
					fileobj.write(("u_" + str(i) + "_" + str(j) + "_" + str(p)))
					fileobj.write("\n")
			for j in range(0, self.WORD_LENGTH):
				for p in range(0, self.WORD_LENGTH):
					fileobj.write(("v_" + str(i) + "_" + str(j) + "_" + str(p)))
					fileobj.write("\n")
		for j in range(0, self.WORD_LENGTH):
			fileobj.write(("x_" + str(self.Round) + "_" + str(j)))
			fileobj.write("\n")
		for j in range(0, self.WORD_LENGTH):
			fileobj.write(("y_" + str(self.Round) + "_" + str(j)))
			fileobj.write("\n")
		fileobj.write("END")
		fileobj.close()

	def MakeModel(self):
		self.CreateObjectiveFunction()
		self.CreateConstraints()
		self.Init()
		self.BinaryVariable()

	def WriteObjective(self, obj):
		fileobj = open(self.filename_result, "a")
		fileobj.write("The objective value = %d\n" %obj.getValue())
		eqn1 = []
		eqn2 = []
		for i in range(0, self.blocksize):
			u = obj.getVar(i)
			if u.getAttr("x") != 0:
				eqn1.append(u.getAttr('VarName'))
				eqn2.append(u.getAttr('x'))
		length = len(eqn1)
		for i in range(0,length):
			s = eqn1[i] + "=" + str(eqn2[i])
			fileobj.write(s)
			fileobj.write("\n")
		fileobj.close()

	def SolveModel(self):
		time_start = time.time()
		m = read(self.filename_model)
		counter = 0
		set_zero = []
		global_flag = False
		while counter < self.blocksize:
			m.optimize()
			if m.Status == 2:
				obj = m.getObjective()
				if obj.getValue() == 1:
					fileobj = open(self.filename_result, "a")
					fileobj.write("************************************COUNTER = %d\n" % counter)
					fileobj.close()
					self.WriteObjective(obj)
					for i in range(0, self.blocksize):
						u = obj.getVar(i)
						temp = u.getAttr('x')
						if temp == 1:
							set_zero.append(u.getAttr('VarName'))
							u.ub = 0
							m.update()
							counter += 1
							break
				else:
					global_flag = True
					break
			elif m.Status == 3:
				global_flag = True
				break
			else:
				print("Unknown error!")

		fileobj = open(self.filename_result, "a")		
		if global_flag:
			fileobj.write("\nIntegral Distinguisher Found!\n\n")
			print("Integral Distinguisher Found!\n")
		else:
			fileobj.write("\nIntegral Distinguisher do NOT exist\n\n")
			print("Integral Distinguisher do NOT exist\n")

		fileobj.write("Those are the coordinates set to zero: \n")
		for u in set_zero:
			fileobj.write(u)
			fileobj.write("\n")
		fileobj.write("\n")
		time_end = time.time()
		fileobj.write(("Time used = " + str(time_end - time_start)))
		fileobj.close()



if __name__ == "__main__":

	WORD_LENGTH = int(input("Input the word length of the target cipher (16 for SIMON32): "))
	while WORD_LENGTH not in [16, 24, 32, 48, 64]:
		print("Invalid word length!")
		WORD_LENGTH = int(input("Input the word length again: "))

	ROUND = int(input("Input the target round number: "))
	while not (ROUND > 0):
		print("Input a round number greater than 0.")
		ROUND = int(input("Input the target round number again: "))

	ACTIVEBITS = int(input("Input the number of acitvebits: "))
	while not (ACTIVEBITS < 128 and ACTIVEBITS > 0):
		print("Input a number of activebits with range (0, 128):")
		ACTIVEBITS = int(input("Input the number of acitvebits again: "))
	i = 0
	ROUND_NEW = ROUND + 1
	for insert in range(1, ROUND_NEW):
		simon = Simon(ROUND, ACTIVEBITS, WORD_LENGTH, insert, i)
		simon.MakeModel()
		simon.SolveModel()
