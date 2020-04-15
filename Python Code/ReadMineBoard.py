#Reference
#https://stackoverflow.com/questions/7853628/how-do-i-find-an-image-contained-within-an-image
import numpy as np
import pyautogui 
import time
import cv2

class ReadScreen:
    def __init__(self):
        # 0 in cv2.imread() function means to read in grayscale
        self.__mClosedBox = cv2.imread('Images//closedbox.png', 0)

        # Index 0 to 8 in mNumbers corresponds to 0-8 image numbers in minesweeper board
        # where 0 is represented by emptybox
        self._mNumbers = []
        self._mNumbers.append( cv2.imread('Images//emptybox.png',0) )
        for i in range(1,9):
            self._mNumbers.append( cv2.imread('Images//'+ str(i) +'.png', 0) )
        
        # Index 9 and 10 in mNumber: Unidentified MineImage
        self._mNumbers.append( cv2.imread('Images//mine.png',0) )
        self._mNumbers.append( cv2.imread('Images//mine_opened.png',0) )

        self.__mFlag = cv2.imread('Images//flag.png',0)
        self.__mSmiley = cv2.imread('Images//smiley.png', 0)
        
        #Screenshot of Current Screen
        w,h= pyautogui.size()
        reg = (0,0,w,h)
        self.__mCurrentScreen = self._ScreenShot(reg)
        self.__FindCoordinates()

    def _FindMatchingImage(self, main, img,threshold = 0.9):
        w, h = img.shape
        res = cv2.matchTemplate(main, img, cv2.TM_CCOEFF_NORMED)
        
        loc = np.where(res >= threshold)
        return w,h,loc

    def __FindCoordinates(self):
        #Finding Array size and coordinates of Array
        w,h,loc = self._FindMatchingImage(self.__mCurrentScreen, self.__mClosedBox)

        self._mX = ( min(loc[1]), max(loc[1]), w )
        self._mY = ( min(loc[0]), max(loc[0]), h )

        # Pixel Region where minesweeper field exist on screen
        self._mReg = (  self._mX[0], 
                        self._mY[0], 
                        self._mX[1]-self._mX[0]+self._mX[2], 
                        self._mY[1]-self._mY[0]+self._mY[2]  )

        self._mCoordinatesToIndex = {}
        self._mIndexToCoordinates = {}
        # loc[1] so h
        self._mRow = int ( ( self._mY[1] - self._mY[0] )/self._mY[2] ) + 1
        # loc[0] so w
        self._mCol = int ( ( self._mX[1] - self._mX[0] )/self._mX[2] ) + 1

        i = 0
        j = 0
        for pt in zip(*loc[::-1]):
            self._mCoordinatesToIndex[ pt[0]-self._mX[0], pt[1]-self._mY[0] ] = (i,j)
            self._mIndexToCoordinates[ (i,j) ] = ( pt[0], pt[1], w, h )
            if j == self._mCol-1:
                i+=1
                j=-1
            j+=1
            
            #Line below draws red square on the matching images( i.e. blankboxes in minesweeper )
            #cv2.rectangle(self.mCurrentScreen, pt, (pt[0] + w, pt[1] + h), (0, 0, 255), 2)

        # Finding Restart Button:
        # Will always be at index 0 cause only 1 occurence of smiley in whole minesweeper
        w,h,loc = self._FindMatchingImage( self.__mCurrentScreen, self.__mSmiley, 0.8)       
        self.__mResult = ( loc[1][0], loc[0][0], w, h )

    def __GetCenterCoordinates(self,point):
        x,y,w,h = point
        return int(x+(w/2)), int(y+(h/2))

    def _Click(self, point):
        cenX, cenY = self.__GetCenterCoordinates(point)
        pyautogui.click(cenX, cenY)
        pyautogui.moveTo(self.__mResult)

    def __RightClick(self,point):
        cenX, cenY = self.__GetCenterCoordinates(point)
        pyautogui.rightClick(cenX, cenY)
        pyautogui.moveTo(self.__mResult)

    def _ScreenShot(self, reg):    
        img = np.array(pyautogui.screenshot(region=reg))
        return cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    def _Restart(self):
        self._Click(self.__mResult)

    def _SetFlag(self,i,j):
        self.__RightClick(self._mIndexToCoordinates[(i,j)])
        
      
class MineBoard(ReadScreen):
    def __init__(self,totalMines):
        ReadScreen.__init__(self)
        self.mBoard = np.zeros((self._mRow, self._mCol), dtype=int)
        self.mBoard -= 10
        self.mStatus = True
        self.mTotalMines = totalMines
        self.mOpenedValue = []
        self.mMarkedMines = 0

    def BoardCheckValue(self,x,y):
        # Assigning value in list for fast iteration
        if self.mBoard[x,y] >= 0 and self.mBoard[x,y] <= 8:
            self.mOpenedValue.append((x,y))
        elif self.mBoard[x,y] == 9 or self.mBoard[x,y] == 10:
            self.mStatus = False

    def Open(self,x,y):
        if self.IsUnknown(x,y) == False:
            return self.mBoard[x,y]
        
        imgBefore = self._ScreenShot(self._mReg)
        self._Click(self._mIndexToCoordinates[(x,y)])
        self.BoardCheckValue( x,y )
        imgAfter = self._ScreenShot(self._mReg)
        res = imgAfter ^ imgBefore
        
        for n in range(len(self._mNumbers)):
            w,h,loc = self._FindMatchingImage( res,self._mNumbers[n] )
            for pt in zip(*loc[::-1]):
                
                i,j = self._mCoordinatesToIndex[(pt[0],pt[1])]
                print(i,j)
                self.mBoard[i,j] = n
                self.BoardCheckValue( i,j )

        return self.mBoard[x,y]

    def GetBoardSize(self):
        return self._mRow, self._mCol

    def GetTotalUnknownLocations(self):
        return (self._mRow*self._mCol) - len(self.mOpenedValue) - self.mMarkedMines

    def GetAllUnknownLocation(self):
        ans = np.where(self.mBoard == -10)
        return list ( zip( ans[0], ans[1] ) )

    def GetAllKnownLocation(self):
        return self.mOpenedValue

    #Used by Solver Class
    def MarkAsSafe(self,x,y):
        #print("MarkAsSafe: ",self.mBoard[x,y])
        self.mBoard[x,y] = -2
 
    def MarkAsMine(self,x,y):
        #print("MarkAsMine: ",self.mBoard[x,y])
        self.mBoard[x,y] = -1

    def MarkAsMineOnScreen(self,x,y):
        self.mMarkedMines+=1
        self.MarkAsMine(x,y)
        #self._SetFlag(x,y)

    def GetStatus(self):
        return self.mStatus and ( self.mTotalMines !=  (self._mRow*self._mCol) - len(self.mOpenedValue) ) and ( self.mTotalMines != self.mMarkedMines )

    def IsUnknown(self,x,y):
        return self.mBoard[x,y] == -10

    def IsMarkedMine(self,x,y):
        return self.mBoard[x,y] == -1

    # This will always be known value when called by Solver
    def GetMineCount(self,x,y):
        if self.IsUnknown(x,y):
            print(x,y,"Locha ho gaya")
        return self.mBoard[x,y]

    def UnknownLocation_KnownMines(self,x,y):
        ul = []
        km = 0
        for i in range(-1,2,1):
            for j in range(-1,2,1):
                if i == 0 and j == 0:
                    continue

                elif ( (x+i<self._mRow) and (x+i>-1) and  (y+j<self._mCol) and (y+j>-1) ):
                    if self.IsUnknown(x+i,y+j):
                        ul.append((x+i,y+j))
                    if self.IsMarkedMine(x+i,y+j):
                        km+=1
        return ul,km




'''
s = time.time()
#input("Enter any number to start: ")
board = MineBoard()
temp = board.GetBoxNumber(5,6)
print(board.GetBoardSize())


print( time.time() - s )
print(temp)
#input("Press Key to Exit: ")

def IsUnknown(self,i,j):
        if self.mBoard[i,j] == -10:
            return True
        return False
'''