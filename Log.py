PRINT_ENABLE = True

def PrintServer(str,WorningLevel =0):
    level = GetWorningLevel(WorningLevel)
    print(f"[SERVER]{level} "+str)


def PrintClient(str,WorningLevel =0):
    level = GetWorningLevel(WorningLevel)
    print(f"[CLIENT2]{level} "+str)


def NormalPrint(str):
    if(PRINT_ENABLE):
        print(str)  


def GetWorningLevel(WorningLevel): 
    if WorningLevel > 3:
        WorningLevel = 3
    if WorningLevel == 0:
        return ""
    if WorningLevel == 1:
        return "[INFO]"
    if WorningLevel == 2:
        return "[WARNING]"
    if WorningLevel == 3:
        return "[ERROR]"
    
    