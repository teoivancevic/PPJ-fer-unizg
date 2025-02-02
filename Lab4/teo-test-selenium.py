import os
import subprocess
import json
from time import sleep

# Selenium imports
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.keys import Keys

# ------------------------ Configuration ------------------------
TESTS_FOLDER = 'test'       # Folder containing subfolders with test.in and test.out
SRC_FOLDER   = 'src'        # Folder containing C++ code
EXECUTABLE   = 'semAnalizator'  # Name of the compiled binary
SIMULATOR_URL= 'https://balrog.zemris.fer.hr/friscjs'
MAX_RUN_TIME_SEC = 10       # If simulator runs > 10s, we'll treat it as stuck
VERBOSE_MODE = True
FIRST_FOLDER_OFFSET = 0     # offset if you want to skip some tests

# --------------------------------------------------------------
def print_verbose(*args, **kwargs):
    """Prints only if VERBOSE_MODE=True"""
    if VERBOSE_MODE:
        print(*args, **kwargs)

def compile_cpp_code():
    """
    Compile the C++ code in SRC_FOLDER into a single executable (EXECUTABLE).
    Adjust the g++ command/flags as needed.
    """
    print("Compiling C++ code...")
    cpp_files = [f for f in os.listdir(SRC_FOLDER) if f.endswith('.cpp')]
    # print("DEBUG: cpp files: ", cpp_files)
    cpp_files_paths = [os.path.join(SRC_FOLDER, f) for f in cpp_files]
    # cpp_files_paths = ["*.cpp"]
    print("DEBUG: cpp files: ", cpp_files_paths)
    # Example compile command:
    cmd = ["g++"] + cpp_files_paths + ["-o", os.path.join(SRC_FOLDER, EXECUTABLE)]
    print("DEBUG cmd: ", cmd)
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print("Compilation failed! Stderr:\n", result.stderr)
        raise SystemExit("Cannot proceed without a successful build.")
    print("Compilation successful.\n")

def generate_frisc_assembly(test_input_path):
    """
    Run the compiled semAnalizator with the .in file as stdin.
    The analyzer itself writes the FRISC code to 'a.frisc' in src/.
    Then we read and return the code from 'a.frisc'.
    """
    analyzer_path = os.path.join(SRC_FOLDER, EXECUTABLE)
    frisc_file_path = os.path.join(SRC_FOLDER, "a.frisc")
    print("DEBUG analy path:", analyzer_path)
    print("DEBUG test_input_path:", test_input_path)
    # print("DEBUG stdin:", fin)
    # print("DEBUG analy path:", analyzer_path)
    file2 = open("izlaz.txt", "w")
    # 1) Run the analyzer with the test input
    with open(test_input_path, 'r') as fin:
        run_process = subprocess.run(
            ["./src/semAnalizator"],
            stdin=fin,
            stdout=file2,  # No need to capture stdout
            stderr=subprocess.PIPE,     # Capture errors, if any
        )
    
    if run_process.returncode != 0:
        print("Error running semAnalizator:\n", run_process.stderr)
        return None
    
    # 2) Check if 'a.frisc' exists
    if not os.path.exists(frisc_file_path):
        print("Error: 'a.frisc' not found in src/ after semAnalizator run.")
        return None

    # 3) Read and return the FRISC code
    with open(frisc_file_path, 'r') as f:
        frisc_code = f.read()
    
    return frisc_code


def build_simulator_json(frisc_assembly_code):
    """
    Build the JSON data structure that the online simulator expects.
    """
    sim_data = {
        "program": frisc_assembly_code,
        "cpuFreq": "1000",
        "memSize": "256",
        "ioUnits": []
    }
    return json.dumps(sim_data, indent=4)

def main():
    # 1. Compile the code first
    compile_cpp_code()
    
    # 2. Create the Selenium driver (example with Firefox)
    chrome_options = Options()
    chrome_options.add_argument("--allow-running-insecure-content")
    chrome_options.add_argument("--ignore-certificate-errors")
    chrome_options.add_argument("--headless")

    # chrome_options.add_argument("--headless")  # Uncomment for headless mode if desired

    driver = webdriver.Chrome(
        service=Service(ChromeDriverManager().install()),
        options=chrome_options
    )
    driver.get(SIMULATOR_URL)
    sleep(1)
    
    # 3. Gather test subfolders
    all_items = os.listdir(TESTS_FOLDER)
    all_items.sort()
    test_folders = [item for item in all_items 
                    if os.path.isdir(os.path.join(TESTS_FOLDER, item)) 
                    and item != '.DS_Store']  # filter out .DS_Store or anything not a folder
    
    tests_passed = 0
    tests_failed = []

    # 4. Run each test
    for i in range(FIRST_FOLDER_OFFSET, len(test_folders)):
        folder_name = test_folders[i]
        folder_path = os.path.join(TESTS_FOLDER, folder_name)
        
        print(f"\n-----------------------------------------")
        print(f"Running test #{i} in folder '{folder_name}'")
        
        test_in_path  = os.path.join(folder_path, 'test.in')
        test_out_path = os.path.join(folder_path, 'test.out')
        
        if not (os.path.exists(test_in_path) and os.path.exists(test_out_path)):
            print(f"Missing test.in or test.out in {folder_name}, skipping...")
            tests_failed.append(folder_name)
            continue
        
        # 4a. Generate FRISC assembly
        print_verbose(f"Generating FRISC assembly from {test_in_path}")
        frisc_code = generate_frisc_assembly(test_in_path)
        if frisc_code is None:
            print("Error generating FRISC code, skipping test.")
            tests_failed.append(folder_name)
            continue
        
        # 4b. Build JSON for simulator
        sim_in = build_simulator_json(frisc_code)
        print("DEBUG: sim frisc code", sim_in)
        
        # 4c. Load assembly into simulator
        print_verbose(f"Loading FRISC assembly into simulator for {folder_name} ...")
        try:
            # Click "Load/Save"
            driver.find_element(By.LINK_TEXT, "Load/Save").click()
            sleep(0.2)
            
            # Paste JSON into the 'frisc-cfg-import-text' textarea
            text_area = driver.find_element(By.ID, "frisc-cfg-import-text")
            # Clear it first
            text_area.clear()
            text_area.send_keys(sim_in)
            
            # Now click "Import" -> "Load"
            driver.find_element(By.ID, "frisc-cfg-import").click()
            sleep(0.2)
            
            load_button = driver.find_element(By.ID, "frisc-load")
            load_button.click()
            
            # 4d. Run the program
            print_verbose("Running program in the simulator...")
            run_button = driver.find_element(By.ID, "frisc-execute")
            run_button.click()
            
            # Wait up to MAX_RUN_TIME_SEC for it to finish
            counter = 0
            stuck = False
            
            # As soon as "Load" is re-enabled, the program is done
            while not load_button.is_enabled():
                sleep(0.1)
                counter += 1
                if counter > MAX_RUN_TIME_SEC * 10:
                    stuck = True
                    print("Program is stuck / took too long. Skipping test.")
                    break
            
            if stuck:
                # Force stop
                driver.find_element(By.ID, "frisc-stop").click()
                tests_failed.append(folder_name)
                continue
            
            print_verbose(f"Program finished in {counter/10} seconds.")
            
            # 4e. Check results (we read R6)
            r6 = driver.find_element(By.ID, "cpu_r6")
            r6_text = r6.find_element(By.CLASS_NAME, "cpu-reg-value").text  # hex string, e.g. "00000005"
            
            # Convert from hex string to signed integer
            # We'll parse the number of hex digits to see if sign bit is set
            r6_number = int(r6_text, 16)
            bits_count = len(r6_text)*4
            sign_mask = 1 << (bits_count - 1)
            if r6_number & sign_mask:
                # negative
                r6_number = r6_number - (1 << bits_count)
            
            with open(test_out_path, 'r') as fout:
                expected = int(fout.read().strip())
            
            print(f"R6 = {r6_number}, expected = {expected}")
            
            if r6_number == expected:
                tests_passed += 1
                print("  -- PASSED --")
            else:
                tests_failed.append(folder_name)
                print("  -- FAILED --")
        
        except Exception as e:
            print(f"Exception encountered while running test {folder_name}:")
            print(e)
            tests_failed.append(folder_name)
            continue
    
    # Summary
    total_tests = len(test_folders) - FIRST_FOLDER_OFFSET
    print("\n\n-----------------------------------------")
    print(f"Execution finished: {tests_passed}/{total_tests} tests passed.")
    if tests_failed:
        print("Failed tests:", tests_failed)
    else:
        print("No tests failed!")
    
    # Close browser
    driver.quit()

if __name__ == "__main__":
    main()
