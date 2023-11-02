PRINT_ENABLE = True

def print_server(str,warning_level = 0):
    level = get_warning_level(warning_level)
    print(f"[SERVER]{level} " + str)

def print_client(str,warning_level =0):
    level = get_warning_level(warning_level)
    print(f"[CLIENT2]{level} " + str)

def normal_print(str):
    if(PRINT_ENABLE):
        print(str)  

def get_warning_level(warning_level): 
    if warning_level > 3:
        warning_level = 3
    if warning_level == 0:
        return ""
    if warning_level == 1:
        return "[INFO]"
    if warning_level == 2:
        return "[WARNING]"
    if warning_level == 3:
        return "[ERROR]"
    