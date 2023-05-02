# Copyright (c) 2023, Sérgio Vieira <internalregister@gmail.com>
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import sys
import inspect

z80hla_executable = "../bin/z80hla"

def compare_binaries(fileName1, fileName2):
    fileContent1 = None
    fileContent2 = None
    with open(fileName1, mode='rb') as file:
        fileContent1 = file.read()
    with open(fileName2, mode='rb') as file:
        fileContent2 = file.read()
    if (len(fileContent1) != len(fileContent2)):
        return False

    for i in range(len(fileContent1)):
        if (fileContent1[i] != fileContent2[i]):
            return False

    return True

def removeFile(filePath):
    if (os.path.exists(filePath)):
        os.remove(filePath)

def standardTest(name):
    result = False
    filePath1 = name + ".z80hla"
    filePath2 = name + ".asm"
    outputFile1 = name + "_output1.bin"
    outputFile2 = name + "_output2.bin"
    try:
        removeFile(outputFile1)
        removeFile(outputFile2)
        os.system(f"{z80hla_executable} -o {outputFile1} {filePath1} > /dev/null 2>&1")
        os.system(f"z80asm -o {outputFile2} {filePath2} > /dev/null 2>&1")
        if ((not os.path.exists(outputFile1)) or (not os.path.exists(outputFile2))):            
            return False    
        result = compare_binaries(outputFile1, outputFile2)
        return result
    finally:
        if result:
            removeFile(outputFile1)
            removeFile(outputFile2)


def testAllOps():
    """All ops     """
    return standardTest("all_ops")

def testInline():
    """Inline      """
    return standardTest("inline")

def testLibrary():
    """Library     """
    return standardTest("library")

def testFunction():
    """Function    """
    return standardTest("function")

def testExpression():
    """Expression  """
    return standardTest("expression")

def testIf():
    """If          """
    return standardTest("if")

def testBreakif():
    """Breakif     """
    return standardTest("breakif")

def testData():
    """Data        """
    return standardTest("data")

def testStruct():
    """Struct      """
    return standardTest("struct")

def testContinueif():
    """Continueif  """
    return standardTest("continueif")

if __name__ == "__main__":
    print("Z80HLA Tests\n")
    testFunctions = [obj for name,obj in inspect.getmembers(sys.modules[__name__]) if (inspect.isfunction(obj) and name.startswith('test'))]
    for f in testFunctions:
        print(f.__doc__ + ":\t", end = "")
        if f():
            print("\033[92mPASSED\033[0m")
        else:
            print("\033[91mFAILED\033[0m")



