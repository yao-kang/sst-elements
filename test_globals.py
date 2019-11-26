# -*- coding: utf-8 -*-

""" This is some globals to pass data between the top level test engine
    and the lower level testscripts """
def initTestGlobals():
    global debugMode
    global verbosity
    global numRanks
    global numThreads
    global listOfSearchableTestPaths
    global __TestAppDebug  # Only used to debug the test application

    debugMode = False
    verbosity = 1
    numRanks = 0
    numThreads = 0
    listOfSearchableTestPaths = ['.']
    __TestAppDebug = False
