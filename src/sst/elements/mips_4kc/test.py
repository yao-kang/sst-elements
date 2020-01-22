import sst
import os
from optparse import OptionParser

# options
op = OptionParser()
op.add_option("-c", "--cacheSz", action="store", type="int", dest="cacheSz", default=4)
op.add_option("-f", "--faultLoc", action="store", type="int", dest="faultLoc", default=0)
op.add_option("-e", "--execFile", action="store", type="string", dest="execFile", default="test/matmat.out")
(options, args) = op.parse_args()

faultPeriod = 100000

if options.faultLoc == 0x4:
    faultPeriod = 1500   #ensure we get at least one MDU insert
elif (options.faultLoc == 0x8 or options.faultLoc == 0x10):
    faultPeriod = 20000

# Define the simulation components
comp_mips = sst.Component("MIPS4KC", "mips_4kc.MIPS4KC")
comp_mips.addParams({
    "verbose" : 0,
    "execFile" : options.execFile,
    "clock" : "1GHz",
    "fault_locations" : options.faultLoc,
    "fault_period" : faultPeriod,
    "timeout" : 1000000
})

comp_l1cache = sst.Component("l1cache", "memHierarchy.Cache")
comp_l1cache.addParams({
    "access_latency_cycles" : "1",
    "cache_frequency" : "4 Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MSI",
    "associativity" : "4",
    "cache_line_size" : "64",
    #"debug" : "1",
    #"debug_level" : "10",
    "verbose" : 0,
    "L1" : "1",
    "cache_size" : "%dKiB"%options.cacheSz
})

comp_memory = sst.Component("memory", "memHierarchy.MemController")
comp_memory.addParams({
      "debug" : 1,
      "coherence_protocol" : "MSI",
      "debug_level" : 10,
      "backend.access_time" : "10 ps",
      "backing" : "malloc", 
      "clock" : "4GHz",
      "backend.mem_size" : "512MiB"
})

# Enable statistics
sst.setStatisticLoadLevel(7)
sst.setStatisticOutput("sst.statOutputConsole")
#sst.enableAllStatisticsForComponentType("memHierarchy.Cache")


# Define the simulation links
link_mips_cache = sst.Link("link_mips_mem")
link_mips_cache.connect( (comp_mips, "mem_link", "10ps"), (comp_l1cache, "high_network_0", "10ps") )
link_mem_bus_link = sst.Link("link_mem_bus_link")
link_mem_bus_link.connect( (comp_l1cache, "low_network_0", "10ps"), (comp_memory, "direct_link", "10ps") )

