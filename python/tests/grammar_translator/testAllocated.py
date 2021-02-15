#!/usr/bin/env python3
import addtoplevelpath
import test
import grammar as grammar
import translator.translator as translator

testdata ="""
allocated(A_d)
allocated(message%incoming%buffer(:))
""".strip(" ").strip("\n").split("\n")

test.run(
   expression     = grammar.allocated,
   testdata       = testdata,
   tag            = "allocated",
   raiseException = True
)

import translator.translator

test.run(
   expression     = translator.allocated,
   testdata       = testdata,
   tag            = "allocated",
   raiseException = True
)

v = testdata[0]
print(translator.allocated.parseString(v)[0].fStr().replace(" ","")  ) 
v = testdata[1]
print(translator.allocated.parseString(v)[0].fStr().replace(" ","")  ) 
#assert translator.allocated.parseString(v)[0].fStr(True,True,8).replace(" ","")   == "hipMemcpy(a,b,1_8*(c)*(8),hipMemcpyDeviceToDevice)"
#assert translator.allocated.parseString(v)[0].fStr(False,True,8).replace(" ","")  == "hipMemcpy(c_loc(a),b,1_8*(c)*(8),hipMemcpyDeviceToHost)"
#assert translator.allocated.parseString(v)[0].fStr(True,False,8).replace(" ","")  == "hipMemcpy(a,c_loc(b),1_8*(c)*(8),hipMemcpyHostToDevice)"
#assert translator.allocated.parseString(v)[0].fStr(False,False,8).replace(" ","") == "hipMemcpy(c_loc(a),c_loc(b),1_8*(c)*(8),hipMemcpyHostToHost)"
#print(translator.allocated.parseString(v)[0].fStr(True,True,8).replace(" ","")  ) 
#print(translator.allocated.parseString(v)[0].fStr(False,True,8).replace(" ","") )
#print(translator.allocated.parseString(v)[0].fStr(True,False,8).replace(" ","") ) 
#print(translator.allocated.parseString(v)[0].fStr(False,False,8).replace(" ",""))

print("SUCCESS")
