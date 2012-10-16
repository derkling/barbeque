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
threads = [ 1, 4, 8 ]
instances = [ 1, 3, 6, 9, 12 ]
configs = [
    ["NOBBQ",   "Unmanaged",    '0.2'],
    ["BBQ",     "BBQ Managed",  '0.6'],
]

################################################################################
#   Metrics of interest
################################################################################
metrics = {
#     Label         Name                    Description                         Column      Improvements
#                                                                                               1 = the lower the better
#                                                                                               0 = the higher the better
 "ctime":        [ "Time [s]",           "Workload completion time [s]",              1,     1,   ],
 "power":        [ "Power [W]",          "System power consumption [W]",              2,     1,   ],
 "task-clock":   [ "Ticks",              "Task clock ticks",                          3,     1,   ],
 "ctx":          [ "Context-Switches",   "Total number of context switches",          5,     1,   ],
 "mig":          [ "Migrations",         "Total number of CPU migrations",            6,     1,   ],
 "pf":           [ "Page-Faults",        "Total number of page fauls",                7,     1,   ],
 "cycles":       [ "Cycles",             "Total number of CPU cycles",                8,     1,   ],
 "fes":          [ "Fronte-End Stalls",  "Total number of front-end stalled-cycles", 10,     1,   ],
 "fei":          [ "Front-End Idles",    "Total number of front-end idle-cycles",    11,     1,   ],
 "bes":          [ "Back-End Stalls",    "Total number of back-end stalled-cycles",  12,     1,   ],
 "bei":          [ "Back-End Idles",     "Total number of back-end idle-cycles",     13,     1,   ],
 "ins":          [ "Instructions",       "Total number of executed instructions",    14,     1,   ],
 "scpi":         [ "SPC",                "Effective Stalled-Cycles-per-Instruction", 16,     1,   ],
 "b":            [ "Branches",           "Total number of branches",                 17,     1,   ],
 "b-rate":       [ "Branches-Rate",      "Effective rate of branch instructions",    18,     1,   ],
 "b-miss":       [ "Branch-miss",        "Total number of missed branches",          19,     1,   ],
 "b-miss-rate":  [ "Branch-miss Quota",  "Effective percentage of missed branches",  20,     1,   ],
 "ghz":          [ "GHz",                "Effective processor speed",                 9,     0,   ],
 "cpu-used":     [ "CPUs utilized",      "CPUs utilization",                          4,     0,   ],
 "ipc":          [ "IPC",                "Effective Instructions-per-Cycles",        15,     0,   ],
}



################################################################################
#   End of Configuration Section
#   Do not touch after this marker
################################################################################

# Confiugrations columns
configs_name  = [c[0] for c in configs]
configs_desc  = [c[1] for c in configs]
configs_color = [c[2] for c in configs]

# Configure metrics columns
metrics_label = metrics.keys()
metrics_names  = [c[0] for c in metrics.values()]
def metric_name(m):
    return metrics[m][0]
metrics_descs  = [c[1] for c in metrics.values()]
def metric_desc(m):
    return metrics[m][1]
metrics_cols  = [c[2] for c in metrics.values()]
def metric_col(m):
    return metrics[m][2]

print "Parsing profile data, columns: ", metrics_cols, "=", metrics_label

# Internal configuration flags
show_plot = 0
verbose = 0

# Autovivification support for nested dictionaries
class AutoVivification(dict):
    """Implementation of perl's autovivification feature."""
    def __getitem__(self, item):
        try:
            return dict.__getitem__(self, item)
        except KeyError:
            value = self[item] = type(self)()
            return value

# Pretty print a nested dictionary using proper indentetion
def pretty(d, indent=0):
    for key, value in d.iteritems():
        print '  ' * indent + str(key)
        if isinstance(value, dict):
            pretty(value, indent+1)
        else:
            print '  ' * (indent+1) + str(value)

# Collection of statistic on performance metrics
metrics_stats = AutoVivification()


################################################################################
#   Plotting functions
################################################################################

def plotMetric_WTM(data, w, t, m):

    # Setup graph geometry, axis and legend
    xtitle = "Number of '%s' concurrent instances (%d threads)" % (w, t)
    ytitle = m
    graph_title = metrics[m][1]
    graph_name = "PlotWTM_%s-%02dT-%s.pdf" % (w, t, m)

    print "Plotting [%s]..." % graph_name

    # Add a bargraph for each configuration compared
    c_bar = len(instances)   # Number of bar groups to plot
    x_pos = np.arange(c_bar) # Location of bars on X axis
    y_wid = 0.4              # Bar didth

    fig = plt.figure()
    plot1 = fig.add_subplot(111)

    bars = []
    for c_i in range(len(configs)):
        values = []
        errors = []
        c = configs_name[c_i]
        for i in instances:
            values.append(data[w][i][t][c][m][0])
            errors.append(data[w][i][t][c][m][5])

        bars.append(
                plot1.bar(
                    x_pos + (y_wid * c_i),
                    values,
                    y_wid,
                    color = configs_color[c_i],
                    yerr = errors, ecolor = '0.0'
                )
            )

    # Setup graph legend
    plot1.legend(
            bars,
            [l[1] for l in configs],
            loc = 4, # lower-right
        )

    # Setup X-Axis
    plot1.set_xlabel(xtitle)
    plot1.set_xticks(x_pos + y_wid)
    plot1.set_xticklabels(instances)

    # Setup Y-Axis
    plot1.set_ylabel(ytitle)
    plot1.set_ybound(
            lower = 0)

    # Setup Graph title
    plot1.set_title(graph_title)
   
    if show_plot:
        plt.show()
    else:
        plt.savefig(graph_name,
            papertype = 'a3',
            format = 'pdf',
        )

def plotMetrics_WIT(data, w, i, t):

    # Setup graph geometry, axis and legend
    xtitle = "Performances metrics"
    ytitle = "Normalized value"
    graph_title = "Metrics for (%s, %d instances, %d threads)" % (w, i, t)
    graph_name = "PlotWIT_%s-%02dI-%02dT.pdf" % (w, i, t)

    print "Plotting [%s]..." % graph_name

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
            m = metrics_label[m_i]
            c = configs[c_i][0]
            if (c_i == 0):
                factor = data[w][i][t][c][m][0]
                rvalues.append(0.0)
                rerrors.append((data[w][i][t][c][m][5])/factor)
            else:
                factor = data[w][i][t][configs[0][0]][m][0]
                values.append(1 - (data[w][i][t][c][m][0]/factor))
                errors.append((data[w][i][t][c][m][5]/factor))

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

    # Setup X-Axis
    plot1.set_xlabel(xtitle)
    plot1.set_xticks(x_pos + y_wid)
    plot1.set_xticklabels(
            metrics_label,
            rotation='vertical',
            size='small')

    # Setup Y-Axis
    plot1.set_ylabel(ytitle)
    plot1.set_ybound(
            lower = -1.5,
            upper =  1.5)

    # Setup Graph title
    plot1.set_title(graph_title)

    # Plot the graph...
    if show_plot:
        plt.show()
    else:
        plt.savefig(
            graph_name,
            papertype = 'a3',
            format = 'pdf',
        )

def plotGraphs():

    for w in workloads:
        for i in instances:
            for t in threads:
                plotMetrics_WIT(metrics_stats, w, i, t)
    
    for w in workloads:
        for t in threads:
            for m in metrics:
                plotMetric_WTM(metrics_stats, w, t, m)


def computeStats(w,i,t,c):
    global metrics_stats

    # Load profling data from file
    datafile = "PARSECTest-%s-N%02d-T%02d-%s.dat" % (w, i, t, c)
    if verbose:
        print "Parsing %s..." % datafile
    metrics_values = np.loadtxt(datafile, usecols = metrics_cols)

    # Compute Statistics on each metric
    for m_i in range(len(metrics)):

        # Get specific metrics samples
        samples = [row[m_i] for row in metrics_values]
        if verbose:
            print "Stats for [%s]: " % (metrics_label[m_i]), samples

        # Compute statistics
        n, (smin, smax), smean, svar, sskew, skurt = stats.describe(samples)
        sd = sqrt(svar)
        se = sd / sqrt(n)
        ci95 = 1.96 * se
        ci99 = 2.58 * se

        # Keep track of statistics
        metrics_stats[w][i][t][c][metrics_label[m_i]] = (smean, sd, n, se, ci95, ci99, smin, smax)

def parseDatFiles():
    for w in workloads:
        for i in instances:
            for t in threads:
                for c in configs_name:
                    computeStats(w,i,t,c)

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

    # Parse *.DAT files produced by the profiler
    parseDatFiles()
    if verbose:
        pretty(metrics_stats, 1)

    # Produce all the plots
    plotGraphs()


if __name__ == "__main__":
    sys.exit(main())

# vim: set ai sw=4 ts=4 sta et fo=croql softtabstop=4 :#
