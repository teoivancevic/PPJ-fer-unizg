import os
import subprocess
import glob


os.system('color')

red     = "\033[0;31m"
green   = "\033[0;32m"
nocolor = "\033[0m"

TEST_DISCORD = False
TEST_INTRANET = True

cwd = os.getcwd()

# TODO update test discord
if TEST_DISCORD:

    
    tests = cwd + "/discord"
    print(tests)
    os.chdir(tests)

    items = os.listdir()
    types = set()
    for item in items:
        print(item)
        a = item.split(".")
        types.add(a[0])
    print(types)
    os.chdir(cwd)
    tn = 0
    for it in types:
        lang = it + ".lan"
        iN = it + ".in"
        out = it + ".out"
        # print(lang, iN, out)
        subprocess.run(["cp", "integration/" + lang, "code/"])
        subprocess.run(["cp", "integration/" + iN, "code/analizator"])
        subprocess.run(["cp", "integration/" + out, "code/analizator/correct.txt"])
        nd = cwd + "/code"
        # print("TEO TEST DBG", nd)
        print("--------------------")
        os.chdir(nd)
        
        # subprocess.run(["g++", "generator.cpp", "-o", "generator"])
        # subprocess.run(["g++", "Generator.cpp", "automata.cpp", "Regex.cpp", "", "-o", "generator"])
        cpp_files = glob.glob("*.cpp")
        subprocess.run(["g++"] + cpp_files +["-std=c++17","-O2","-o","generator"])

        file = open(lang, "r")
        subprocess.run(["./generator"], stdin=file, text=True)
        file.close()
        nd2 = nd + "/analizator"
        os.chdir(nd2)
        file = open(iN, "r")
        file2 = open("izlaz.txt", "w")

        # subprocess.run(["g++", "analizator.cpp", "automat.cpp", "-o", "analizator"])
        cpp_files = glob.glob("*.cpp")
        subprocess.run(["g++"] + cpp_files +["-std=c++17","-O2","-o","analizator"])

        subprocess.run(
            ["./analizator"], stdin=file, stdout=file2, stderr=subprocess.DEVNULL
        )
        file.close()
        file2.close()
        out = subprocess.run(
            ["diff", "izlaz.txt", "correct.txt"], text=True, capture_output=True
        )
        if out.stdout == "":
            print(f"{green}PASSED TEST{nocolor} {it}")
        else:
            print(f"{red}FAIL TEST{nocolor} {it}")
            print(out.stdout)

            #break
        subprocess.run(["rm", "izlaz.txt"])
        subprocess.run(["rm", "correct.txt"])
        subprocess.run(["rm", iN])
        os.chdir(nd)
        subprocess.run(["rm", lang])
        os.chdir(cwd)


# now test integration2 from intranet
if TEST_INTRANET:
    
    tests = cwd + "/test"
    print(tests)
    os.chdir(tests)
    folders = os.listdir()
    print(folders)

    folders.remove(".DS_Store")
    
    os.chdir(cwd)


    print("--------------------")
    print("INTRANET TESTS")
    print("--------------------")

    defaultTestFileName = "test"
<<<<<<< HEAD
    # it = defaultTestFileName
=======
    srcFolder = "lab2-SPRUT"

>>>>>>> 05e144514fd80b39a30c02f9346977e6334d1bda

    for folder in folders:
        lang = defaultTestFileName + ".san"
        iN = defaultTestFileName + ".in"
        out = defaultTestFileName + ".out"
        # print(lang, iN, out)
        subprocess.run(["cp", "test/" + folder + "/" + lang, "src/"])
        subprocess.run(["cp", "test/" + folder + "/"  + iN, "src/analizator"])
        subprocess.run(["cp", "test/" + folder + "/"  + out, "src/analizator/correct.txt"])
        nd = cwd + "/src"
        # print("TEO TEST DBG", nd)
        print("--------------------")
        os.chdir(nd)
        
        # subprocess.run(["g++", "generator.cpp", "-o", "generator"])
        # subprocess.run(["g++", "Generator.cpp", "automata.cpp", "Regex.cpp", "", "-o", "generator"])
        cpp_files = glob.glob("*.cpp")
        subprocess.run(["g++"] + cpp_files +["-std=c++17","-O2","-o","generator"])
        # subprocess.run(["g++"] + cpp_files +["-o","generator"])

        file = open(lang, "r")
        subprocess.run(["./generator"], stdin=file, text=True)
        file.close()
        nd2 = nd + "/analizator"
        os.chdir(nd2)
        file = open(iN, "r")
        file2 = open("izlaz.txt", "w")

        # subprocess.run(["g++", "analizator.cpp", "automat.cpp", "-o", "analizator"])
        cpp_files = glob.glob("*.cpp")
        subprocess.run(["g++"] + cpp_files +["-std=c++17","-O2","-o","analizator"])
        # subprocess.run(["g++"] + cpp_files +["-o","analizator"])

        subprocess.run(
            ["./analizator"], stdin=file, stdout=file2, stderr=subprocess.DEVNULL
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
            # print(out.stdout)

            #break
        subprocess.run(["rm", "izlaz.txt"])
        subprocess.run(["rm", "correct.txt"])
        subprocess.run(["rm", iN])
        os.chdir(nd)
        subprocess.run(["rm", lang])
        os.chdir(cwd)

