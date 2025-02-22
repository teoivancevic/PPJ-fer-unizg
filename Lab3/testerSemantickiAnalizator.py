import os
import subprocess
import glob


os.system('color')

red     = "\033[0;31m"
green   = "\033[0;32m"
nocolor = "\033[0m"


cwd = os.getcwd()


# now test integration2 from intranet
tests = cwd + "/test"
print(tests)
os.chdir(tests)
folders = os.listdir()
folders.sort()
print(folders)

folders.remove(".DS_Store")

os.chdir(cwd)


print("----------------------------------------")
print("INTRANET TESTS w/ Discord Tests merged")
print("----------------------------------------")

defaultTestFileName = "test"
srcFolder = "src"


for folder in folders:
    # lang = defaultTestFileName + ".san"
    iN = defaultTestFileName + ".in"
    out = defaultTestFileName + ".out"

    # subprocess.run(["cp", "test/" + folder + "/" + lang, srcFolder + "/"])
    subprocess.run(["cp", "test/" + folder + "/"  + iN, srcFolder + "/"])
    subprocess.run(["cp", "test/" + folder + "/"  + out, srcFolder + "/correct.txt"])
    # nd = cwd + "/src"
    nd = cwd + "/" + srcFolder

    print("--------------------")
    os.chdir(nd)
    
    # subprocess.run(["g++", "generator.cpp", "-o", "generator"])
    # subprocess.run(["g++", "Generator.cpp", "automata.cpp", "Regex.cpp", "", "-o", "generator"])
    # cpp_files = glob.glob("*.cpp")
    # subprocess.run(["g++"] + cpp_files +["-std=c++17","-O2","-o","generator"])
    # subprocess.run(["g++"] + cpp_files +["-o","generator"])

    # file = open(lang, "r")
    # subprocess.run(["./generator"], stdin=file, text=True)
    # file.close()
    # nd2 = nd + "/analizator"
    # os.chdir(nd2)
    file = open(iN, "r")
    file2 = open("izlaz.txt", "w")

    # subprocess.run(["g++", "analizator.cpp", "automat.cpp", "-o", "analizator"])
    cpp_files = glob.glob("*.cpp")
    subprocess.run(["g++"] + cpp_files +["-std=c++17","-O2","-o","semAnalizator"])
    # subprocess.run(["g++"] + cpp_files +["-o","analizator"])

    subprocess.run(
        ["./semAnalizator"], stdin=file, stdout=file2, stderr=subprocess.DEVNULL
    )
    file.close()
    file2.close()
    out = subprocess.run(
        ["diff", "izlaz.txt", "correct.txt"], text=True, capture_output=True
    )
    if out.stdout == "":
        print(f"{green}PASSED TEST{nocolor} {folder}")
    else:
        print(f"{red}FAIL TEST{nocolor} {folder}")
        print(out.stdout)
        #break

    subprocess.run(["rm", "izlaz.txt"])
    subprocess.run(["rm", "correct.txt"])
    subprocess.run(["rm", iN])
    os.chdir(nd)
    # subprocess.run(["rm", lang])
    os.chdir(cwd)