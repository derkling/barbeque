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
from math import sqrt

################################################################################
#   Input datasets configuration
################################################################################
workloads = [ "bodytrack" ]
num_threads = [ 1, 4, 8 ]
num_apps = [ 1, 3, 6, 9, 12 ]
configs = [
    ["NOBBQ",   "Unmanaged",    '0.2'],
    ["BBQ",     "BBQ Managed",  '0.6'],
]

################################################################################
#   Metrics of interest
################################################################################
metrics = [
#     Name                  Description                                 Label           Column  Improvements
#                                                                                               1 = the lower the better
#                                                                                               0 = the higher the better
    [ "Time [s]",           "Workload completion time [s]",             "ctime",         1,     1,   ],
    [ "Power [W]",          "System power consumption [W]",             "power",         2,     1,   ],
    [ "Ticks",              "Task clock ticks",                         "task-clock",    3,     1,   ],
    [ "Context-Switches",   "Total number of context switches",         "ctx",           5,     1,   ],
    [ "Migrations",         "Total number of CPU migrations",           "mig",           6,     1,   ],
    [ "Page-Faults",        "Total number of page fauls",               "pf",            7,     1,   ],
    [ "Cycles",             "Total number of CPU cycles",               "cycles",        8,     1,   ],
    [ "Fronte-End Stalls",  "Total number of front-end stalled-cycles", "fes",          10,     1,   ],
    [ "Front-End Idles",    "Total number of front-end idle-cycles",    "fei",          11,     1,   ],
    [ "Back-End Stalls",    "Total number of back-end stalled-cycles",  "bes",          12,     1,   ],
    [ "Back-End Idles",     "Total number of back-end idle-cycles",     "bei",          13,     1,   ],
    [ "Instructions",       "Total number of executed instructions",    "ins",          14,     1,   ],
    [ "SPC",                "Effective Stalled-Cycles-per-Instruction", "scpi",         16,     1,   ],
    [ "Branches",           "Total number of branches",                 "b",            17,     1,   ],
    [ "Branches-Rate",      "Effective rate of branch instructions",    "b-rate",       18,     1,   ],
    [ "Branch-miss",        "Total number of missed branches",          "b-miss",       19,     1,   ],
    [ "Branch-miss Quota",  "Effective percentage of missed branches",  "b-miss-rate",  20,     1,   ],
    [ "GHz",                "Effective processor speed",                "ghz",           9,     0,   ],
    [ "CPUs utilized",      "CPUs utilization",                         "cpu-used",      4,     0,   ],
    [ "IPC",                "Effective Instructions-per-Cycles",        "ipc",          15,     0,   ],
]

# Configure metrics columns
cols = [c[3] for c in metrics]
mtrs = [m[2] for m in metrics]
print "Parsing profile data, columns: ", cols, "=", mtrs

# Internal configuration flags
show_plot = 0
verbose = 0



class AutoVivification(dict):
    """Implementation of perl's autovivification feature."""
    def __getitem__(self, item):
        try:
            return dict.__getitem__(self, item)
        except KeyError:
            value = self[item] = type(self)()
            return value




# Data collection
metrics_per_app = AutoVivification()
metrics_stats = AutoVivification()


def plotAMetric(a, ppm, m, t):

    if verbose:
        print "Plotting [%s] from dataset: " % metrics[m][2], ppm

    # Setup graph geometry, axis and legend
    ytitle = metrics[m][0]
    graph_title = metrics[m][1]
    graph_name = "BBQProfiling-%s-%s-%02dT.pdf" % (a, metrics[m][2], t)

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
    plot1.legend(bars,
            [l[1] for l in configs],
            loc = 4 # lower-right
        )

    # Add some labels
    plot1.set_ylabel(metrics[m][0])
    plot1.set_ybound(lower = 0)
    plot1.set_xlabel("Number of '%s' concurrent instances (%d threads)" % (a, t))
    plot1.set_title(metrics[m][1])
    plot1.set_xticks(x_pos + y_wid)
    plot1.set_xticklabels(num_apps)

    print "Plotting %s" % (graph_name)
    if show_plot:
        plt.show()
    else:
        plt.savefig(graph_name,
            papertype = 'a3',
            format = 'pdf',
        )


def pretty(d, indent=0):
    for key, value in d.iteritems():
        print '  ' * indent + str(key)
        if isinstance(value, dict):
            pretty(value, indent+1)
        else:
            print '  ' * (indent+1) + str(value)

def plotAll_ANT(data, a, n, t):
    print "Plotting all metrics for (%s, %d instances, %d threads)" % (a, n, t)

    # Setup graph geometry, axis and legend
    xtitle = "Performances metrics"
    ytitle = "Normalized value"
    graph_title = "Metrics for (%s, %d instances, %d threads)" % (a, n, t)
    graph_name = "BBQProfiling_AllMetrics_%s-%02dI-%02dT.pdf" % (a, n, t)

    # Add a bargraph for each configuration compared
    c_bar = len(metrics) # Number of bar groups to plot
    x_pos = np.arange(c_bar) # Location of bars on X axis
    y_wid = 0.4 # Bar didth

    # Add a single plot (for all metrics)
    fig = plt.figure()
    plot1 = fig.add_subplot(111)

    bars = []
    for c_i in range(len(configs)):
        # Reference values
        rvalues = []
        rerrors = []
        # Plotting values
        values = []
        errors = []
        for m_i in range(len(metrics)):
            m = metrics[m_i][2]
            c = configs[c_i][0]
            if (c_i == 0):
                factor = data[a][n][t][c][m][0]
                rvalues.append(0.0)
                rerrors.append((data[a][n][t][c][m][5])/factor)
            else:
                factor = data[a][n][t][configs[0][0]][m][0]
                values.append(1 - (data[a][n][t][c][m][0]/factor))
                errors.append((data[a][n][t][c][m][5]/factor))

        # Add bar plots for the curent configuration
        if (c_i != 0):
            bars.append(
                    plot1.bar(
                        x_pos + (y_wid * c_i),
                        values,
                        y_wid,
                        color = configs[c_i][2],
                        yerr = errors, ecolor = '0.0'
                    )
            )

    if verbose:
        print values
        print errors

#    # Plot a legend
#    plot1.legend(bars,
#            [l[1] for l in configs],
#            loc = 4 # lower-right
#        )

    # Add some labels
    plot1.set_xlabel(xtitle)
    plot1.set_ylabel(ytitle)
    plot1.set_ybound(
            lower = -1.5,
            upper =  1.5)
    plot1.set_title(graph_title)

    # Draw X ticks labels
    plot1.set_xticks(x_pos + y_wid)
    plot1.set_xticklabels(
            mtrs,
            rotation='vertical',
            size='small')

    print "Plotting %s" % (graph_name)
    if show_plot:
        plt.show()
    else:
        plt.savefig(graph_name,
            papertype = 'a3',
            format = 'pdf',
        )




def addAppMetrics(t, a, ppm):
    global metrics_per_app
    metrics_per_app[t][a] = ppm
    #print "MetricsPerApps:\n", metrics_per_app

def dumpAll():
    print "MetricsPerApps:\n", metrics_per_app

def plotPerMetric(app, ppm, t):
    for m in range(len(metrics)):
        plotAMetric(app, ppm[metrics[m][2]], m, t)

def plotPerThreads(app, ppt):
    for t_i in range(len(num_threads)):
        plotPerMetric(app, ppt[t_i], num_threads[t_i])

def graphStatistics(app):
    global metrics_stats
    plots_per_threads = []

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
            for n_i in range(len(num_apps)):

                # Load profling data from file
                datafile = "PARSECTest-%s-N%02d-T%02d-%s.dat" % (app, num_apps[n_i], nt, cfg)
                if verbose:
                    print "Parsing %s..." % datafile
                metrics_values = np.loadtxt(datafile, usecols = cols)
                #print metrics_values

                # Compute Statistics on each metric
                for m_i in range(len(metrics)):
                    samples = [r[m_i] for r in metrics_values]
                    if verbose:
                        print "Stats for [%s]: " % (metrics[m_i][2]), samples
                    n, (smin, smax), smean, svar, sskew, skurt = stats.describe(samples)
                    # Keep track of the statistics for <Threads,Conf,Apps,Metric>
                    # This produces a table, [rows,cols]: [Metrics,Apps]

                    sd = sqrt(svar)
                    se = sd / sqrt(n)
                    ci95 = 1.96 * se
                    ci99 = 2.58 * se

                    stats_per_metrics[m_i][n_i] = (num_apps[n_i], smean, ci99)
                    metrics_stats[app][num_apps[n_i]][nt][cfg][metrics[m_i][2]] = (smean, sd, n, se, ci95, ci99, smin, smax)

                # for metrics

            # for num_apps
            #print "Conf[%s]: " % (cfg), stats_per_metrics

            # Add dataset to be plotted
            dataset_key = "%d threads (%s)" % (nt, cfg)
            for m_i in range(len(metrics)):
                if verbose:
                    print "%s: " % (metrics[m_i][2]), stats_per_metrics[m_i]
                plots_per_metrics[metrics[m_i][2]].append(
                        list(stats_per_metrics[m_i])
                )
            
        # for cfg
        #print "\nPlots per %d threads: " % (nt), plots_per_metrics
        plots_per_threads.append(plots_per_metrics)

        addAppMetrics(nt, app, plots_per_threads)

    # for num_threads
    #print "\nAll Plots: ", plots_per_threads
    plotPerThreads(app, plots_per_threads)



class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def main(argv=None):
    global show_plot
    global verbose

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
                continue
            if o in ("-v", "--verbose"):
                verbose = 1
                continue

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

    #dumpAll()
    pretty(metrics_stats, 1)

    for a in workloads:
        for n in num_apps:
            for t in num_threads:
                plotAll_ANT(metrics_stats, a, n, t)

if __name__ == "__main__":
    sys.exit(main())

# vim: set ai sw=4 ts=4 sta et fo=croql softtabstop=4 :#
