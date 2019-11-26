# -*- coding: utf-8 -*-

""" This is some globals to pass data between the top level test engine
    and the lower level testscripts """
def initTestGlobals():
    global __TestAppDebug  # Only used to debug the test application
    global debugMode
    global verbosity
    global numRanks
    global numThreads
    global listOfSearchableTestPaths
    global testSuiteFilePath
    global testOutputDirPath
    global tempOutputDirPath

    __TestAppDebug = False
    debugMode = False
    verbosity = 1
    numRanks = 0
    numThreads = 0
    listOfSearchableTestPaths = ['.']
    testSuiteFilePath = ""
    testOutputDirPath = "./test_outputs/run_data"
    tempOutputDirPath = "./test_outputs/temp_data"
