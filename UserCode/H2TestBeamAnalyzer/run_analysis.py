#!/usr/bin/python
#run_analysis.py

import subprocess
import sys
import optparse
import os

#Options
parser = optparse.OptionParser("usage: %prog [options]")

parser.add_option ('--r', type='string',
                   dest="runs", default = "*",
                   help="Pick a specific run number or range of numbers with Unix globbing - may need to use single quotes or -f before running. NOTE - MUST USE 6 DIGIT RUN NUMBER, add leading zeros as needed.")
parser.add_option ('-d',
                   dest="delete", action="store_true",
                   default=False, help="Delete files after moving to destination")
parser.add_option ('--dest',
                   type='string', dest="destination",
                   default="/hcalTB/Analysis/", help="Destination directory for run results/html. Remote locations ok")
defRunDest = "/home/daq/Analysis/HcalTestBeam/data_spool_mirror"
parser.add_option ('--runDest',
                   type='string', dest="runDest", default=defRunDest, help="Where the run files HTB*.root are to be stored during processing. '.' for working directory, but will be removed if '.' is used")
parser.add_option ('-v', dest="verbose", action="store_true",
                   default=False, help="Runs the analysis in verbose mode. Not recommended on large runs or batches of runs, as verbose output can be quite massive.")
parser.add_option ('-q', dest="mute", action="store_true", default=False, help="Further decreases verbosity.")

options, args = parser.parse_args()

delete = options.delete
runDest = options.runDest
runs = options.runs
destination = options.destination
verbose = options.verbose
mute = options.mute
dataLoc = '/data/spool/'

########

#Get files from cmshcaltb02
lsCom = 'ls ' + dataLoc + 'HTB_' + runs + '.root'

ls = subprocess.Popen(['ssh','daq@cmshcaltb02', lsCom,], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err =  ls.communicate()

fileList = out.split()
fileList.sort(reverse=True)

for fileName in fileList:
    name = fileName[12:]
    runNum = fileName[16:-5]
    if len(runNum) == 6:
        print "Getting run number %s" % runNum
        rsyncPath = "daq@cmshcaltb02:%s" % fileName
        if verbose:
            subprocess.call(["rsync", "-av", rsyncPath, runDest])
        else:
            subprocess.call(["rsync", "-aq", rsyncPath, runDest])
        symLinkPath = runDest + '/' + name
        if runDest != '.':
            subprocess.call(["ln", "-s", symLinkPath, "."], stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb'))
###########################
#Run analysis
for fileName in fileList:
    name = fileName[12:]
    runNum = fileName[16:-5]
    #Check if file is an HTB*.root file
    if len(name) == 15 and name[:3] == "HTB" and name[-5:] == ".root":
        if verbose:
            subprocess.call(["cmsRun", "h2testbeamanalyzer_cfg_verbose.py", runNum])
        elif mute:
            subprocess.call(["cmsRun", "h2testbeamanalyzer_cfg_verbose.py", runNum], stdout=open(os.devnull, 'wb'))
        else:
            subprocess.call(["cmsRun", "h2testbeamanalyzer_cfg.py", runNum])
        ana = "ana_h2_tb_run%s.root" % runNum
        ana2 = "ana_tb_out_run%s.root" % str(int(runNum))
        plotsDir = "tb_plots_run%s" % str(int(runNum))
        if mute:
            print "Adding dummy value to \"edges\" dictionary in tb_utils.py and processing run " + runNum
            subprocess.call(["./tb_ana.py", "--i", ana, "--o", ana2, "--r", str(int(runNum))], stdout=open(os.devnull, 'wb'))
            subprocess.call(["rm", "-rf", plotsDir], stdout=open(os.devnull, 'wb'))
            print "Generating plots for run " + runNum
            subprocess.call(["./tb_plots.py", "--i", ana2, "--o", plotsDir, "--r", str(int(runNum))], stdout=open(os.devnull, 'wb'))
            print "Generating html for run " + runNum
            subprocess.call(["./makeHtml.py", plotsDir], stdout=open(os.devnull, 'wb'))
            print "Moving results of run " + runNum
            subprocess.call(["rsync", "-aq", "--delete", plotsDir, destination], stdout=open(os.devnull, 'wb'))
        else:
            print "Adding dummy value to \"edges\" dictionary in tb_utils.py and processing run " + runNum
            subprocess.call(["./tb_ana.py", "--i", ana, "--o", ana2, "--r", str(int(runNum))])
            subprocess.call(["rm", "-rf", plotsDir])
            print "Generating plots for run " + runNum
            subprocess.call(["./tb_plots.py", "--i", ana2, "--o", plotsDir, "--r", str(int(runNum))])
            print "Generating html for run " + runNum
            subprocess.call(["./makeHtml.py", plotsDir])
            print "Moving results of run " + runNum
            subprocess.call(["rsync", "-av", "--delete", plotsDir, destination])
        subprocess.call(["rm", name])
        if delete:
            subprocess.call(["rm", ana])
            subprocess.call(["rm", ana2])
            subprocess.call(["rm", "-rf", plotsDir])

