#!/usr/bin/python
#
# Copyright (C) 2012  Politecnico di Milano
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>

import getopt
import os
import string
import sys

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats

################################################################################
#   Input datasets configuration
################################################################################
workloads = [ "bodytrack", "ferret" ]
num_threads = [ 1, 4, 8, 12 ]
num_apps = [ 1, 3, 6, 9, 12 ]
configs = [
    ["BBQ",     "BBQ Managed",  '0.6'],
    ["NOBBQ",   "Unmanaged",    '0.2'],
]

################################################################################
#   Metrics of interest
################################################################################
metrics = [
    [ "Time [s]",           "Workload completion time [s]",             "ctime",         1],
    [ "Power [W]",          "System power consumption [W]",             "power",         2],
    [ "Ticks",              "Task clock ticks",                         "task-clock",    3],
    [ "CPUs utilized",      "CPUs utilization",                         "cpu-used",      4],
    [ "Context-Switches",   "Total number of context switches",         "ctx",           5],
    [ "Migrations",         "Total number of CPU migrations",           "mig",           6],
    [ "Page-Faults",        "Total number of page fauls",               "pf",            7],
    [ "Cycles",             "Total number of CPU cycles",               "cycles",        8],
    [ "GHz",                "Effective processor speed",                "ghz",           9],
    [ "Fronte-End Stalls",  "Total number of front-end stalled-cycles", "fes",          10],
    [ "Front-End Idles",    "Total number of front-end idle-cycles",    "fei",          11],
    [ "Back-End Stalls",    "Total number of back-end stalled-cycles",  "bes",          12],
    [ "Back-End Idles",     "Total number of back-end idle-cycles",     "bei",          13],
    [ "Instructions",       "Total number of executed instructions",    "ins",          14],
    [ "IPC",                "Effective Instructions-per-Cycles",        "ipc",          15],
    [ "SPC",                "Effective Stalled-Cycles-per-Instruction", "scpi",         16],
    [ "Branches",           "Total number of branches",                 "b",            17],
    [ "Branches-Rate",      "Effective rate of branch instructions",    "b-rate",       18],
    [ "Branch-miss",        "Total number of missed branches",          "b-miss",       19],
    [ "Branch-miss Quota",  "Effective percentage of missed branches",  "b-miss-rate",  20],
]

# Internal configuration flags
show_plot = 0
verbose = 0

def plotAMetric(app, ppm, m):

    print "Plotting [%s] from dataset: " % metrics[m][2], ppm

    # Setup graph geometry, axis and legend
    ytitle = metrics[m][0]
    graph_title = metrics[m][1]
    graph_name = "PTest-%s-%s.pdf" % (app, metrics[m][2])

    # Add a bargraph for each configuration compared
    c_bar = len(num_apps)   # Number of bar groups to plot
    x_pos = np.arange(c_bar)       # Location of bars on X axis
    y_wid = 0.4            # Bar didth

    fig = plt.figure()
    plot1 = fig.add_subplot(111)

    bars = []
    for c_i in range(len(configs)):
        values = [v[1] for v in ppm[c_i]]
        errors = [v[2] for v in ppm[c_i]]
        if verbose:
            print "DS[", c_i, ", pos: ", x_pos, "], V: ", values, "E: ", errors
        bars.append(plot1.bar(x_pos + (y_wid * c_i),
                values,
                y_wid,
                color = configs[c_i][2],
                yerr = errors, ecolor = '0.0'))

    #if verbose:
    #    print "Generated bars: ", bars
    plot1.legend(bars, [l[1] for l in configs])

    # Add some labels
    plot1.set_ylabel(metrics[m][0])
    plot1.set_ybound(lower = 0)
    plot1.set_xlabel("Number of concurrently running applications")
    plot1.set_title(metrics[m][1])
    plot1.set_xticks(x_pos + y_wid)
    plot1.set_xticklabels(num_apps)

    if show_plot:
        plt.show()
    else:
        plt.savefig(graph_name,
            papertype = 'a3',
            format = 'pdf',
        )

def plotPerMetric(app, ppm):
    for m in range(len(metrics)):
        plotAMetric(app, ppm[metrics[m][2]], m)

def plotPerThreads(app, ppt):
    for tp in ppt:
        plotPerMetric(app, tp)

def graphStatistics(app):
    plots_per_threads = []

    # Configure metrics columns
    cols = (c[3] for c in metrics)
    
    # Initialize metrics matrix
    stats_per_metrics = [[(0,0,0) for i in range(len(num_apps))] for i in range(len(metrics))]

    for nt in num_threads:
        # Initialize the container of plots for each metrics
        plots_per_metrics = {}
        for m_i in range(len(metrics)):
            plots_per_metrics[metrics[m_i][2]] = []
        #print "\nPlots per %d threads: " % (nt), plots_per_metrics

        # With or without BBQ
        for cfg in [c[0] for c in configs]:

            # For each number of concurrent instances
            for a_i in range(len(num_apps)):

                # Load profling data from file
                datafile = "PARSECTest-%s-N%02d-T%02d-%s.dat" % (app, num_apps[a_i], nt, cfg)
                metrics_values = np.loadtxt(datafile, usecols = cols)

                # Compute Statistics on each metric
                for m_i in range(len(metrics)):
                    samples = [r[m_i] for r in metrics_values]
                    n, (smin, smax), smean, svar, sskew, skurt = stats.describe(samples)
                    #print "%02d - %s: " % (num_apps[a_i], metrics[m_i][0]), smean, svar
                    # Keep track of the statistics for <Threads,Conf,Apps,Metric>
                    # This produces a table, [rows,cols]: [Metrics,Apps]
                    stats_per_metrics[m_i][a_i] = (num_apps[a_i], smean, svar)

                # for metrics

            # for num_apps
            #print "Conf[%s]: " % (cfg), stats_per_metrics

            # Add dataset to be plotted
            dataset_key = "%d threads (%s)" % (nt, cfg)
            for m_i in range(len(metrics)):
                #print "%d Appending: " % (m_i), stats_per_metrics[m_i]
                plots_per_metrics[metrics[m_i][2]].append(
                        list(stats_per_metrics[m_i])
                )
            
        # for cfg
        #print "\nPlots per %d threads: " % (nt), plots_per_metrics
        plots_per_threads.append(plots_per_metrics)

    # for num_threads
    #print "\nAll Plots: ", plots_per_threads
    plotPerThreads(app, plots_per_threads)



class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def main(argv=None):
    if argv is None:
        argv = sys.argv

    # parse command line options
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "hsv", ["help", "show", "verbose"])
        except getopt.error, msg:
            raise Usage(msg)

        # process options
        for o, a in opts:
            if o in ("-h", "--help"):
                print __doc__
                return 0
            if o in ("-s", "--show"):
                show_plot = 1
            if o in ("-v", "--verbose"):
                verbose = 1

        # process arguments
        #for arg in args:
        #    process(arg) # process() is defined elsewhere

    except Usage, err:
        print >>sys.stderr, err.msg
        print >>sys.stderr, "for help use --help"
        return 2

    # Iterate on all the workload applications
    for Wi in workloads:
        graphStatistics(Wi)


if __name__ == "__main__":
    sys.exit(main())

# vim: set ai sw=4 ts=4 sta et fo=croql softtabstop=4 :#
