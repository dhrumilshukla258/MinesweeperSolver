#Reference
#https://stackoverflow.com/questions/7853628/how-do-i-find-an-image-contained-within-an-image
import numpy as np
import time
import copy

from ReadMineBoard import MineBoard

class Solver:
    def __init__(self, totalMine):
        self.mField = MineBoard(totalMine)
        self.mTempField = 0

    def __Rule12(self, x, y):
        ul, km = self.mTempField.UnknownLocation_KnownMines(x,y)
        val = self.mTempField.GetMineCount(x,y)
        size = len(ul)
        if val - km == size:
            for i,j in ul:
                self.mTempField.MarkAsMine(i,j)
            
        if val == km:
            for i,j in ul:
                self.mTempField.MarkAsSafe(i,j)
			
    def __Rule3(self, x1, x2):
        ul_1, km_1 = self.mTempField.UnknownLocation_KnownMines(x1[0],x1[1])
        val_1 = self.mTempField.GetMineCount(x1[0],x1[1])

        ul_2, km_2 = self.mTempField.UnknownLocation_KnownMines(x2[0],x2[1])
        val_2 = self.mTempField.GetMineCount(x2[0],x2[1])

        mine = [item for item in ul_1 if item not in ul_2]
        safe = [item for item in ul_2 if item not in ul_1]
       
        if (val_1 - km_1) - (val_2 - km_2) == len(mine):
            for i,j in mine:
                self.mTempField.MarkAsMine(i,j)
                
            for i,j in safe:
                self.mTempField.MarkAsSafe(i,j)

        if (val_2 - km_2) - (val_1 - km_1) == len(safe):
            #The Opposite of what we did above to avoid multiple calculations
            for i,j in safe:
                self.mTempField.MarkAsMine(i,j)
                
            for i,j in mine:
                self.mTempField.MarkAsSafe(i,j)

    # Proof by Contradiction  
    def __Rule45(self, x, y):
        ul, km = self.mTempField.UnknownLocation_KnownMines(x,y)
        val = self.mTempField.GetMineCount(x,y)
        return val < km, val > (km+len(ul))
    
    # Applies all the 5 Rules
    def __ApplyRules(self):
        values = self.mTempField.GetAllKnownLocation()

        for i in range(len(values)):            
            for j in range(i+1,len(values)):
                if ( abs(values[i][0] - values[j][0]) <= 2 and  abs(values[i][1] - values[j][1]) <= 2):
                    self.__Rule3( values[i], values[j] )
        
        for i in range(len(values)):                
            self.__Rule12( values[i][0], values[i][1] )

        for i in range(len(values)):            
            ans = self.__Rule45( values[i][0], values[i][1] )
            if ans[0] or ans[1]:
                return True

        return False

    # Resolution Proof by Contradiction
    def IsMine(self,x,y):
        self.mTempField = copy.deepcopy(self.mField)
        self.mTempField.MarkAsSafe(x,y)
        return self.__ApplyRules()
    
    def IsSafe(self,x,y):
        self.mTempField = copy.deepcopy(self.mField)
        self.mTempField.MarkAsMine(x,y)
        return  self.__ApplyRules()
    
    # The rules are same as before but here we will
    # Open the indices instead of marking it safe and
    # The Flag will be set on screen where there will be mine
    def __Rule12_Aux(self, x, y):
        ul, km = self.mField.UnknownLocation_KnownMines(x,y)
        val = self.mField.GetMineCount(x,y)
        size = len(ul)
        if val - km == size:
            for i,j in ul:
                self.mField.MarkAsMineOnScreen(i,j)
            
        if val == km:
            for i,j in ul:
                self.__CheckValue(i,j)

    def __Rule3_Aux(self, x1, x2):
        ul_1, km_1 = self.mField.UnknownLocation_KnownMines(x1[0],x1[1])
        val_1 = self.mField.GetMineCount(x1[0],x1[1])

        ul_2, km_2 = self.mField.UnknownLocation_KnownMines(x2[0],x2[1])
        val_2 = self.mField.GetMineCount(x2[0],x2[1])

        mine = [item for item in ul_1 if item not in ul_2]
        safe = [item for item in ul_2 if item not in ul_1]
        
        if (val_1 - km_1) - (val_2 - km_2) == len(mine):
            for i,j in mine:
                self.mField.MarkAsMineOnScreen(i,j)
                
            for i,j in safe:
                self.__CheckValue(i,j)

        elif (val_2 - km_2) - (val_1 - km_1) == len(safe):
            #The Opposite of what we did above to avoid multiple calculations
            for i,j in safe:
                self.mField.MarkAsMineOnScreen(i,j)
                
            for i,j in mine:
                self.__CheckValue(i,j)

    # Applies Rule 1, 2 and 3
    def __ApplyRules_Aux( self ):
        values = self.mField.GetAllKnownLocation()
        
        for i in range(len(values)):
            self.__Rule12_Aux( values[i][0], values[i][1] )            
            for j in range(i+1,len(values)):
                if ( abs(values[i][0] - values[j][0]) <= 2 and  abs(values[i][1] - values[j][1]) <= 2):
                    self.__Rule3_Aux( values[i], values[j] )
        

        return self.mField.GetTotalUnknownLocations()

    def __CheckValue(self,x,y):  
        self.mField.Open(x,y)
        return self.mField.GetTotalUnknownLocations()

    def __Find_XY(self):
        ul = self.mField.GetAllUnknownLocation()
        return ul[0]

    def __LoopCheck(self):
        return self.mField.GetStatus() and self.mCount>0 

    def Solve(self):
        self.R, self.C = self.mField.GetBoardSize()
        self.__CheckValue( int(self.R/2),int(self.C/2) )        
        
        self.mCount = 1
        self.mPrev_Count = -1
        while self.__LoopCheck() and  self.mCount!=self.mPrev_Count:
            print(self.mCount,self.mPrev_Count)
            self.mPrev_Count = self.mCount
            
            self.mCount = self.__ApplyRules_Aux()
            if self.__LoopCheck() and self.mCount==self.mPrev_Count:
                self.mCount = self.__SolveAux()
                if self.__LoopCheck() and self.mCount==self.mPrev_Count:
                    p = self.__Find_XY()
                    print("Random selection of Point: ",p)
                    self.mCount = self.__CheckValue(p[0],p[1])
            
        print(self.mField.mBoard)
    
    def __SolveAux(self):

        s = time.time()
        indices = self.mField.GetAllUnknownLocation()
        
        #print("SolveAux",len(indices))
        print("SolAux: ",len(indices),time.time()-s)
        apple = 0

        for x,y in indices:
            ul, km = self.mField.UnknownLocation_KnownMines(x,y)
            xCheck = ( x==0 or x==self.R-1 )
            yCheck = ( y==0 or y==self.C-1 )

            check = True
            if xCheck:
                if yCheck and len(ul) == 3:
                    check = False
                elif len(ul) == 5:
                    check = False
            elif yCheck:
                if xCheck and len(ul) == 3:
                    check = False
                elif len(ul) == 5:
                    check = False
            else:
                if len(ul) == 8:
                    check = False

            if check == True:
                if self.IsMine(x,y):
                    print("SolveAux_Mine")
                    self.mField.MarkAsMineOnScreen(x,y)
                if self.IsSafe(x,y):
                    print("SolveAux_Safe")
                    self.__CheckValue(x,y)
                apple+=1
        
        print("Total TimeTaken by SolAux ",time.time()-s)

        return self.mField.GetTotalUnknownLocations()
    

print(" ----------------------------------------- ")
print(" Make sure Minesweeeper is fully visible before starting and")
print(" do not move the Window onces when the application is started")
print(" ----------------------------------------- ")

totalMine = int ( input("Enter Total Mines: ") )
print(totalMine)
sol = Solver(totalMine)
sol.Solve()
print("---------")