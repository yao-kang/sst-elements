import os
import unittest
import filecmp

def testlog(stringtolog):
    print("\n{0}\n".format(stringtolog))


class exampletest(unittest.TestCase):


    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_RNG_Mersenne(self):
        self.run_RNG_Testcase("mersenne")


    def test_RNG_Marsaglia(self):
        self.run_RNG_Testcase("marsaglia")
        pass


    def test_RNG_xorshift(self):
        self.run_RNG_Testcase("xorshift")
        pass

###

    def run_RNG_Testcase(self, rngcase):
#        testlog("")

        # Set all the various file paths
        sdlfile = "./src/sst/elements/simpleElementExample/tests/test_simpleRNGComponent_{0}.py".format(rngcase)
        outfile = "./test_outputs/test_simpleRNGComponent_{0}.out".format(rngcase)
        tmpfile = "./test_outputs/test_simpleRNGComponent_{0}.tmp".format(rngcase)
        cmpfile = "./test_outputs/test_simpleRNGComponent_{0}.cmp".format(rngcase)
        reffile = "./src/sst/elements/simpleElementExample/tests/refFiles/test_simpleRNGComponent_{0}.out".format(rngcase)

        # Build the launch command
        # TODO: Implement a run timeout
        oscmd = "sst {0} > {1}".format(sdlfile, outfile)
        os.system(oscmd)

        # Post processing of the output data to scrub it into a format to compare
        os.system("grep Random {0} > {1}".format(outfile, tmpfile))
        os.system("tail -5 {0} > {1}".format(tmpfile, cmpfile))

        # Perform the test
        self.assertTrue(filecmp.cmp(cmpfile, reffile), "Output/Compare file {0} does not match Reference File {1}".format(cmpfile, reffile))