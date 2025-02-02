from selenium import webdriver
from selenium.webdriver.common.by import By
import subprocess
import os
import json
from time import sleep

# ----------------------------------------- OPTIONS -----------------------------------------
# ONLY CHANGE THEESE

# Path to python3 command
PYTHON_COMMAND = 'python3'

# Path to your program, that converts the AST to assemly. Make sure that your program has the corret output format.
ASSEMLY_BUILDER = 'src/SemantickiAnalizator.py'
TESTS_FOLDER = 'test4_disc'
MAX_PROG_RUNNING_TIME_SEC = 10  # if the program runs more than this time, it will be considered as stuck

# Used for printing extra debug infor when running
VERBOSE_MODE = True

# offset of the first folder to run, use this if you want to skip some folders
FIRST_FOLDER_OFFSET = 0  # 1


def print_verbose(*args, **kwargs):
    if VERBOSE_MODE:
        print(*args, **kwargs)


def process_file(target_folder_name: str) -> str:
    input_file_path = os.path.join(target_folder_name, 'test.in')
    output_file_path = os.path.join(target_folder_name, 'test.a')
    sim_json_path = os.path.join(target_folder_name, 'sim.json')

    print_verbose("convert to assembly...")
    analizator_command = [PYTHON_COMMAND, ASSEMLY_BUILDER]
    with open(input_file_path, 'r') as input_file, open(output_file_path, 'w') as output_file:
        assemly_pipe = subprocess.run(analizator_command, stdin=input_file, stdout=subprocess.PIPE, text=True)
        output_file.write(assemly_pipe.stdout)

    sim_data = {
        "program": assemly_pipe.stdout,
        "cpuFreq": "1000",
        "memSize": "256",
        "ioUnits": []
    }

    print_verbose("convert to simulator input...")
    with open(sim_json_path, 'w') as output_file:
        json.dump(sim_data, output_file, indent=4)
        return json.dumps(sim_data, indent=4)


driver = webdriver.Firefox()
driver.get("https://balrog.zemris.fer.hr/friscjs")

sleep(1)

tests_passed = 0
tests_faild: list[int] = []

# find all fodlers in test folder
folders = sorted(os.listdir(TESTS_FOLDER))

if FIRST_FOLDER_OFFSET > len(folders):
    print("FIRST_FOLDER_OFFSET is too big, there are only", len(folders), "folders")
    exit(1)

for i in range(FIRST_FOLDER_OFFSET, len(folders)):
    folder_name = os.path.join(TESTS_FOLDER, folders[i])
    print("\nprocessing", folder_name)
    sim_in = process_file(folder_name)
    if sim_in is None:
        print("simulatro input is None, error happened chack the output in folder", folder_name)
        tests_faild.append(i)
        continue

    # load program
    print_verbose("loading program...")
    driver.find_element(By.LINK_TEXT, "Load/Save").click()
    text_area = driver.find_element(By.ID, "frisc-cfg-import-text")
    text_area.send_keys(sim_in)
    driver.find_element(By.ID, "frisc-cfg-import").click()
    sleep(0.2)
    load_button = driver.find_element(By.ID, "frisc-load")
    load_button.click()

    # run program
    print_verbose("running program...")
    driver.find_element(By.ID, "frisc-execute").click()

    counter = 0
    prog_stuck = False
    # wait for program to finish
    while not load_button.is_enabled():
        sleep(0.1)
        counter += 1
        if counter > MAX_PROG_RUNNING_TIME_SEC * 10:
            prog_stuck = True
            print("program is stuck, check the output in folder", folder_name)
            break

    if prog_stuck:
        tests_faild.append(i)
        print("skipping this test, program is stuck, running more than", MAX_PROG_RUNNING_TIME_SEC, "seconds")
        driver.find_element(By.ID, "frisc-stop").click()
        continue
    print("program finished in", counter / 10, "seconds")

    # read results
    print_verbose("reading results...")
    r6 = driver.find_element(By.ID, "cpu_r6")

    r6_text = r6.find_element(By.CLASS_NAME, "cpu-reg-value").text
    r6_number = int(r6_text, 16)

    if r6_number & (1 << (len(r6_text) * 4 - 1)):
        r6_number = r6_number - (1 << (len(r6_text) * 4))

    with open(os.path.join(folder_name, 'test.out'), 'r') as f:
        expected_number = int(f.read())
    print(f"r6 = {r6_number}, expected {expected_number}")
    passed_test = r6_number == expected_number
    if passed_test:
        tests_passed += 1
        print(" -- OK -- ")
    else:
        tests_faild.append(i)
        print(" -- FAIL -- ")

print("\n\n\n")
print(f"Passed {tests_passed}/{len(folders) - FIRST_FOLDER_OFFSET}")
print("\n")
print("faild tests:", tests_faild)

driver.quit()
